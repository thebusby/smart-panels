(ns smart-desk.core
  (:require [clojure.java.io       :as io]
            )
  (:import [java.net Socket])
  
  (:use bagotricks)
  (:gen-class))

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

  (do
    (def socket (new Socket "192.168.1.3" 5000))

    (def line-q (java.util.concurrent.LinkedBlockingQueue.))

    (def rdr (io/reader socket))
    (def wtr (io/writer socket))

    (def bg-reader-thread
      (thread (doseq [line (line-seq rdr)]
                (.put line-q line))))
    )

  (.write wtr "PING\n")
  (.peek line-q)


  (defn cmd
    "Writes a commmand (appends \n to msg) to provided writer"
    [wtr msg]
    (do (.write wtr (str msg "\n"))
        (.flush wtr)))

  (cmd wtr "SERV PING")
  (cmd wtr "SERV OPENALL")

  (defn dump-q
    ""
    [q]
    (take (.size q) (repeatedly #(.take q))))
  
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
  

;; ; ;; ; ;; ; ;; ; ;; ; ;; ; ;; ; ;; ; ;; ; ;; ; ;; ; ;; ; ;; ; ;; ; 
  )
