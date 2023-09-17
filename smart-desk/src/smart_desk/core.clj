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

;; Registered handlers for events
(defonce registered-events
  (atom {}))


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

(defn run
  "Execute on the command line and return the response"
  [& fields]
  (-> (Runtime/getRuntime)
      (.exec (into-array String fields))
      (.getInputStream)
      slurp))

(defn run-trailing-q-pusher
  "Execute on the command line and return the response"
  [cmd-array q parse-fn]
  (thread
    (let [rdr (-> (Runtime/getRuntime)
                  (.exec (into-array String cmd-array))
                  (.getInputStream)
                  io/reader)]
      (doseq [line (line-seq rdr)]
        (if-let [event-msg (parse-fn line)]
          (.put q event-msg))))))


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
;; MISC
;; ; ;; ; ;; ; ;; ; ;; ; ;; ; ;; ; ;; ; ;; ; ;; ; ;; ; ;; ; ;; ; ;; ; ;; ; ;; ;

(def mocp-path "/usr/bin/mocp")
(def swaymsg-path "/usr/bin/swaymsg")

(defn get-mocp-state
  ""
  []
  (some->> (run mocp-path "--format" "%state\t%title")
           (clojure.string/trim-newline)
           (split-tsv)
           (zipmap [:mode :title])
           ))


;; ; ;; ; ;; ; ;; ; ;; ; ;; ; ;; ; ;; ; ;; ; ;; ; ;; ; ;; ; ;; ; ;; ; ;; ; ;; ;
;; APP SPECIFIC FN'S
;; ; ;; ; ;; ; ;; ; ;; ; ;; ; ;; ; ;; ; ;; ; ;; ; ;; ; ;; ; ;; ; ;; ; ;; ; ;; ;

(defprotocol CljToProto
  (to-proto [t] "Convert EDN types into proper format for Panel protocol"))

(extend-protocol CljToProto
  java.lang.String
  (to-proto [t] t)

  java.lang.Long
  (to-proto [t] (str t))

  clojure.lang.Keyword
  (to-proto [t] (some-> t
                        name
                        .toUpperCase
                        (.replace \- \_)))

  nil
  (to-proto [_] nil)
  )

(defn caps-to-kw
  "Convert SERIAL text format into Clojure keywords.
   Beware: nil -> nil, \"ONN\" -> :onn, \"-5\" -> -5"
  [^String token]
  (if (not (nil? token))
    (if-let [num (re-get #"(^\-?\d+$)" token)]
      (to-long token)
      (std-keyword token))))

(defn register-event
  "Register a handler (fn [status]) for a particular event id"
  [id handler-fn]
  (swap! registered-events assoc id handler-fn))

(defn parse-event
  "Returns a parsed event, from a (split-tsv line); returns nil if line isn't an event"
  [msg]
  (let [[_ msg-type] msg
        msg (zipmap [:source :msg-type :comp-name :comp-type :status :extra]
                    msg)]
    (if (= msg-type :event)
      (assoc msg

             ;; Add a unique identifier
             :id (keyword (str (name (:source msg)) "|" (name (:comp-name msg)) ))

             ;; Add a timestamp for when it was received
             :ts (current-ts)
             ))))

(defn cmd-string
  "Takes a Panel command, and returns the response.
   Does this via the cmd-tx-q so that only a single
   command is run at a time."
  [^String msg]
  (let [resp (promise)]
    (.put cmd-tx-q [msg resp])
    @resp))

(defn cmd
  "Accept a command"
  [& fields]
  (let [resp (some->> fields
                      (map to-proto)
                      (interpose " ")
                      (apply str)
                      cmd-string)]

    ;; If first field of first record is ERR, throw exception
    (if (some->> resp
                 first
                 first
                 (= :err))
      (throw (Exception. (some->> resp
                                  first
                                  (interpose " ")
                                  (apply str)))))
    resp))

(defn get-panel-state
  "Return all input component states for a specified panel"
  [panel]
  (if-let [resp (cmd panel :desc)]
    (some->> resp
             (keep (fn [fields]
                     (if (> (count fields) 2)
                       (let [comp-name (first fields)
                             value (last fields)]
                         [(keyword (str (name panel) "|" (name comp-name))) value]
                         ))
                     ))
             vec
             (into {}))))

(defn display-mocp
  ""
  []
  (let [{:keys [mode title]} (get-mocp-state)]
    (cmd :music-panel :set :lcd :clr)
    (cmd :music-panel :set :lcd 1 0 mode)
    (cmd :music-panel :set :lcd 2 0 (apply str (take 20 title)))))

(defn get-available-panels
  "Return list of available panels"
  []
  (some->> (cmd :serv "AVAIL")
           (mapv first)))

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
                                        (mapv caps-to-kw))]
                    (if-let [event-msg (parse-event msg)]

                      ;; If we get an event, push to event-q immediately
                      (.put event-q event-msg)

                      ;; Case this isn't an event, see if we agg or push to cmd-rx-q
                      (if (= :ack (first msg))

                        ;; Push aggregate command response
                        (let [resp @agg]
                          (do
                            (.put cmd-rx-q resp)
                            (reset! agg [])))

                        ;; If we see an error, abort everything and send it up
                        (if (= :err (first msg))
                          (do
                            (.put cmd-rx-q [msg])
                            (reset! agg []))

                          ;; Otherwise, just aggregate the message waiting for ACK
                          (swap! agg conj msg)))))))

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

(defn spawn-event-thread
  "Return thread that handles processing events."
  []
  (thread
    (doseq [{:keys [id status] :as event} (repeatedly #(.take event-q))]
      (swap! comp-state assoc id status)
      (if-let [event-handler (get @registered-events id)]
        (event-handler event)))))

(defn spawn-cmd-thread
  "Return thread that handles executing commands"
  []
  (thread
    (doseq [[^String msg prom] (repeatedly #(.take cmd-tx-q))]
      (if-let [wtr @global-writer]
        (do

          ;; Send the message to the server
          (.write wtr (str msg "\n"))
          (.flush wtr)

          ;; Deliver the passed promise, block no cmd-rx-q
          (deliver prom (.take cmd-rx-q)))
        (deliver prom nil)))))

(defn register-default-events
  "Register default events"
  []
  (do
    ;;
    ;; Register default behaviour here
    ;;

    ;; Sway - Fullscreen shortcut
    (register-event :button-panel|key-l1
                    (fn [{:keys [status]}]
                      (if (= status :onn)
                        (run swaymsg-path "fullscreen"))))

    ;; Sway - Close window shortcut
    (register-event :button-panel|key-l5
                    (fn [{:keys [status]}]
                      (if (= status :onn)
                        (run swaymsg-path "kill"))))

    ;; Forward/Back in Playlist
    (register-event :music-panel|re2
                    (fn [{:keys [status]}]
                      (if (= status :right)
                        (do (run mocp-path "--next")
                            (Thread/sleep 3000)
                            (display-mocp))
                        (do (run mocp-path "--previous")
                            (Thread/sleep 3000)
                            (display-mocp)))))

    ;; Volume Up/Down
    (register-event :music-panel|re1
                    (fn [{:keys [status]}]
                      (if (= status :right)
                        (run mocp-path "-v" "+1")
                        (run mocp-path "-v" "-1"))))

    ;; Register Play/Stop
    (register-event :button-panel|toggle,
                    (fn [{:keys [status]}]
                      (if (= status :onn)
                        (do
                          (cmd :music-panel :set :lcd :light :onn)
                          (run mocp-path "--play")
                          (Thread/sleep 3000)
                          (display-mocp))
                        (do
                          (cmd :music-panel :set :lcd :light :off)
                          (run mocp-path "--stop")))))

    ))

(defn init-comp-state
  ""
  []
  (some->> (get-available-panels)
           (map get-panel-state)
           (reduce merge)
           (reset! comp-state)))

(defn -main
  "Spawn threads, sit back, and wait"
  [& args]
  (let [socket-thread (spawn-socket-thread "/tmp/smart_desk.log"
                                           "192.168.1.3"
                                           5000)
        event-thread (spawn-event-thread)
        cmd-thread (spawn-cmd-thread)]

    ;; Initialize component state
    (init-comp-state)

    ;; Register events before we sleep
    (register-default-events)

    ;; Kickoff PINGER
    ;; For tracking network latency and packet loss
    (run-trailing-q-pusher ["/home/busby/env/bin/pinger"]
                           event-q
                           (fn [line]
                             ;; Do something here!
                             nil
                             ))

    ;; Kickoff MyEvTest
    (run-trailing-q-pusher ["/home/busby/env/bin/evtest /dev/hotas"]
                           event-q
                           (fn [line]
                             ;; Do something here!
                             nil
                             ))


    ;; When socket-thread ends, we can shutdown
    (.join socket-thread)))



(comment
;; ; ;; ; ;; ; ;; ; ;; ; ;; ; ;; ; ;; ; ;; ; ;; ; ;; ; ;; ; ;; ; ;; ;



  ;; Handles incoming data from network socket, and divides
  ;; the output to event-q and cmd-rx-q based on type
  (def serial-thread (spawn-socket-thread "/tmp/smart_desk.log"
                                    "192.168.1.3"
                                    5000))

  ;; Handles incoming data from event-q, updates global state,
  ;; and executes any registered even handlers for a given id (source+component)
  (def event-thread (spawn-event-thread))

  ;; Ensures that any commands to the device across the code base are
  ;; queued and run one at a time.
  (def cmd-thread (spawn-cmd-thread))

  ;; Initialize component state
  (init-comp-states)

  ;; Register all the default behavior
  (register-default-events)



  ;; Debug commands
  (.interrupt serial-thread)
  (.interrupt event-thread)
  (.interrupt cmd-thread)

  (some->> [serial-thread event-thread cmd-thread]
           (mapv #(.isAlive %)))

  (cmd :serv :close :status-panel)




  (get-panel-state :music-panel)



  ;; spawn-socket-thread
  (spawn-socket-thread "/tmp/smart_desk.log"
                       "192.168.1.3"
                       5000)

  ;; Start processing events
  (thread
    (doseq [{:keys [id status] :as event} (repeatedly #(.take event-q))]
      (swap! comp-state assoc id status)
      (if-let [event-handler (get @registered-events id)]
        (event-handler event))))





  ;; Turn off LCD backlight
  (cmd :music-panel :set :lcd :light :off)
  (cmd :music-panel :set :rgbled :red :onn)



  ;;
  ;; For trailing output of program,
  ;; like pinger or evtest
  ;;



  
  (-> (Runtime/getRuntime)
      (.exec (into-array String ["/usr/bin/mocp" "--format" "%state\t%title"]))
      (.getInputStream)
      io/reader
      (line-seq)
      )






  (.peek line-q)

  (defn rawcmd
    "Writes a commmand (appends \n to msg) to provided writer"
    [wtr msg]
    (do (.write wtr (str msg "\n"))
        (.flush wtr)))

  (defn dump-q
    "Dump all of the queue"
    [q]
    (take (.size q) (repeatedly #(.take q))))

  (rawcmd "SERV PING")
  (rawcmd "SERV OPENALL")







  (defn read-lines [^String filename]
    (with-open [x (io/reader filename)]
      (vec (line-seq x))))

  ;; Try this!
  (exec "/usr/bin/xeyes")





;; ; ;; ; ;; ; ;; ; ;; ; ;; ; ;; ; ;; ; ;; ; ;; ; ;; ; ;; ; ;; ; ;; ;
  )
