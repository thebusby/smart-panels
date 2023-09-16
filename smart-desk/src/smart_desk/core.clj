(ns smart-desk.core
  (:require [clojure.java.io :as io]
            )
  (:import [java.net Socket])

  (:use bagotricks)
  (:gen-class))


;; ; ;; ; ;; ; ;; ; ;; ; ;; ; ;; ; ;; ; ;; ; ;; ; ;; ; ;; ; ;; ; ;; ; ;; ; ;; ;
;; Global State
;; ; ;; ; ;; ; ;; ; ;; ; ;; ; ;; ; ;; ; ;; ; ;; ; ;; ; ;; ; ;; ; ;; ; ;; ; ;; ;

;; "Provides a queue of events of type emsg"
(defonce event-q
  (java.util.concurrent.LinkedBlockingQueue.))

;; "Provides a queue of commands to be send to the server"
(defonce cmd-tx-q
  (java.util.concurrent.LinkedBlockingQueue.))

;; "Provides a queue of responses from commands, NOT events"
(defonce cmd-rx-q
  (java.util.concurrent.LinkedBlockingQueue.))

;; Stores state for each component
(defonce comp-state
  (atom {}))

;; Stores writer for current socket
(defonce global-writer
  (atom nil))


;; ; ;; ; ;; ; ;; ; ;; ; ;; ; ;; ; ;; ; ;; ; ;; ; ;; ; ;; ; ;; ; ;; ; ;; ; ;; ;
;; Misc. Utilities
;; ; ;; ; ;; ; ;; ; ;; ; ;; ; ;; ; ;; ; ;; ; ;; ; ;; ; ;; ; ;; ; ;; ; ;; ; ;; ;

(defn good-string?
  "Returns true if string provided is both non-nil and not-empty"
  [^String s]
  (and s
       (not (.isEmpty s))))

(defn good-string
  "Returns provided string if (good-string? s) is True."
  [^String s]
  (if (good-string? s)
    s))

(defn keyword-msg
  "Convert SERIAL text format into Clojure keywords.
   Beware: nil -> nil, \"ONN\" -> :onn, \"-5\" -> -5"
  [^String token]
  (if (not (nil? token))
    (if-let [num (re-get #"(^\-?\d+$)" token)]
      (to-long token)
      (std-keyword token))))

;; ; ;; ; ;; ; ;; ; ;; ; ;; ; ;; ; ;; ; ;; ; ;; ; ;; ; ;; ; ;; ; ;; ; ;; ; ;; ;
;; TIMESTAMP
;; ; ;; ; ;; ; ;; ; ;; ; ;; ; ;; ; ;; ; ;; ; ;; ; ;; ; ;; ; ;; ; ;; ; ;; ; ;; ;

;; A 'timestamp' is defined as number of milliseconds since UNIX Epoch UTC time.

(defprotocol ToTimestamp
  (to-ts [t] "A simple function to coerce various date/time values into a 'timestamp'"))

(extend-protocol ToTimestamp
  java.util.Date
  (to-ts [t] (.getTime t))

  java.lang.Long
  (to-ts [t] t)

  java.time.Instant
  (to-ts [t] (.toEpochMilli t))

  java.time.LocalDateTime
  (to-ts [t] (.toEpochMilli (.toInstant t java.time.ZoneOffset/UTC) ))

  java.time.ZonedDateTime
  (to-ts [t] (.toEpochMilli (.toInstant t)))

  nil
  (to-ts [_] nil)

  java.lang.String
  (to-ts [t]
    (some-> t
            good-string
            (java.time.LocalDateTime/parse)
            (.toInstant java.time.ZoneOffset/UTC)
            (.toEpochMilli))))

(defn current-ts
  "Return the current timestamp"
  []
  (to-ts (java.time.ZonedDateTime/now)))

(defn ts-to-str
  "Convert a timestamp into a human readable value"
  [ts]
  (some->> ts
           (java.time.Instant/ofEpochMilli)
           str))

(defn ts-to-local-str
  "Convert a timestamp into a human readable value"
  [ts]
  (some-> ts
           (java.time.Instant/ofEpochMilli)
           (java.time.ZonedDateTime/ofInstant (java.time.ZoneId/systemDefault))
           str))


;; ; ;; ; ;; ; ;; ; ;; ; ;; ; ;; ; ;; ; ;; ; ;; ; ;; ; ;; ; ;; ; ;; ; ;; ; ;; ;
;; APP SPECIFIC FN'S
;; ; ;; ; ;; ; ;; ; ;; ; ;; ; ;; ; ;; ; ;; ; ;; ; ;; ; ;; ; ;; ; ;; ; ;; ; ;; ;

(defn get-panel-prot-name
  ":bottom-panel -> \"BOTTOM_PANEL\" "
  [panel-keyword]
  (some-> panel-keyword
          name
          .toUpperCase
          (.replace \- \_)))

(defn parse-event
  "Returns a parsed event, from a (split-tsv line); returns nil if line isn't an event"
  [msg]
  (let [[_ msg-type] msg
        msg (zipmap [:source :msg-type :comp-name :comp-type :status]
                    msg)]
    (if (= msg-type :event)
      (assoc msg

             ;; Add a unique identifier
             :id (keyword (str (name (:source msg)) "|" (name (:comp-name msg)) ))

             ;; Add a timestamp for when it was received
             :ts (current-ts)
             ))))

(defn cmd
  "Takes a Panel command, and returns the response"
  [panel-keyword ^String msg]

  ;; Abort, and return nil, if the writer isn't available
  (if-let [wtr @global-writer]
    (do

      ;; Send the message
      (.write wtr (str (get-panel-prot-name panel-keyword) " " msg "\n"))
      (.flush wtr)

      ;; Return the response, should block
      (.take cmd-rx-q))))

(defn spawn-socket-thread
  "Spawn thread to handle socket and queues"
  [log-filename address port]
  (thread
    (with-open [logfile (io/writer log-filename :append true)]
      (binding [*out* logfile]
        (let [log (fn [^String s]
                    (println (str (ts-to-local-str (current-ts)) "\t" s)))]

          ;; Handle the case the socket closes and we should re-open it
          (dotimes [socket-count 100]

            ;; For every socket connections
            (log (str "CONN\t" socket-count "\t" address "\t" port))
            (try
              (let [socket (new Socket address port)
                    agg (atom []) ;; Maintain state to aggregate command responses
                    wtr (io/writer socket)]

                ;; Register the new writer for the current socket
                (reset! global-writer wtr)

                (doseq [line (line-seq (io/reader socket))]

                  ;; Let's record some logs
                  (log (str "LINE\t" line))

                  ;; Handle the messages
                  (if-let [msg (some->> line
                                        split-tsv
                                        (mapv keyword-msg))]
                    (if-let [event-msg (parse-event msg)]

                      ;; If we get an event, push to event-q immediately
                      (.put event-q event-msg)

                      ;; Case this isn't an event, see if we agg or push to cmd-rx-q
                      (if (= :ack (first msg))

                        ;; Push aggregate command response
                        (let [resp @agg]
                          (if-not (empty? resp)
                            (do
                              (.put cmd-rx-q resp)
                              (reset! agg []))))

                        ;; Just aggregate message waiting for ACK
                        (swap! agg conj msg))))))

              ;; Catch any exceptions, and restart connections
              (catch InterruptedException e
                (log "INTERUPT")
                (throw (Exception. "Interrupt execution")))
              (catch Exception e
                (log (str "EXCEPTION\t" (.getMessage e)))))

            ;; Wait N seconds if we get disconnected before trying again
            (reset! global-writer nil)
            (log (str "DISC\t" socket-count "\t" address "\t" port))
            (Thread/sleep (* 8 1000))
            ))))))

(defn -main
  "I don't do a whole lot ... yet."
  [& args]
  (println "Hello, World!"))



(comment
;; ; ;; ; ;; ; ;; ; ;; ; ;; ; ;; ; ;; ; ;; ; ;; ; ;; ; ;; ; ;; ; ;; ;


  ;; TODO:
  ;; Get all socket reads, divide into cmd-rx and event queue
  ;; event queue should modify global state
  ;;       retrieve global state easily
  ;; event queue should check for a handler subscribed to that component

  ;; Need command system to do things like DESC and SERV AVAIL


  (def bg-thread (spawn-socket-thread "/tmp/smart_desk.log"
                                      "192.168.1.3"
                                      5000))

  (.interrupt bg-thread)
  (.isAlive bg-thread)







  
  (.interrupt bg-reader-thread)


  (do
    (def socket (new Socket ))
    (def wtr (io/writer socket))

    (defonce line-q (java.util.concurrent.LinkedBlockingQueue.))

    (def bg-reader-thread
      (thread (let [rdr (io/reader socket)]
                (doseq [line (line-seq rdr)]
                  (.put line-q line)))))
    )

  (def bg-reader-thread
    )











  (.write wtr "PING\n")
  (.peek line-q)



(some->> "ACK"
         split-tsv
         (mapv std-keyword))

  (defn rawcmd
    "Writes a commmand (appends \n to msg) to provided writer"
    [msg]
    (do (.write wtr (str msg "\n"))
        (.flush wtr)))

  (defn dump-q
    "Dump all of the queue"
    [q]
    (take (.size q) (repeatedly #(.take q))))

  (rawcmd "SERV PING")
  (raw cmd "SERV OPENALL")


  ;; SOURCE
  ;; MSG_TYPE
  ;; COMPONENT_NAME
  ;; COMPONENT_TYPE
  ;; STATUS






  (let [line "BUTTON_PANEL\tEVENT\tKEY_L1\tBTN\tONN"
        [x y ]]
    (some->> line
             split-tsv))

  (take 2 (line-seq rdr))


  (def test-event "BUTTON_PANEL\tEVENT\tBUTTON\tONN")


  (defn get-queue-lbq
    "Generates a java.util.concurrent.LinkedBlockingQueue
  and returns two functions for 'put' and 'take'"
    ([]
     (let [bq   (java.util.concurrent.LinkedBlockingQueue.)
           put  #(.put bq %)
           take #(.take bq)]
       [put take]))
    ([col]
     (let [bq   (java.util.concurrent.LinkedBlockingQueue. ^Integer (to-int col))
           put  #(.put bq %)
           take #(.take bq)]
       [put take])))

  (defn read-lines [^String filename]
  (with-open [x (io/reader filename)]
    (vec (line-seq x))))

  ;; Try this!
  (exec "/usr/bin/xeyes")




  (some->> line
           split-tsv
           (mapv keyword-msg)
           ;; parse-event
           (zipmap [:source :msg-type :comp-name :comp-type :status]
                    )
           )


  
;; ; ;; ; ;; ; ;; ; ;; ; ;; ; ;; ; ;; ; ;; ; ;; ; ;; ; ;; ; ;; ; ;; ;
  )
