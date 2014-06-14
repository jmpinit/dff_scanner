(defproject depthmap "0.1.0-SNAPSHOT"
  :description "FIXME: write description"
  :url "http://example.com/FIXME"
  :license {:name "Eclipse Public License"
            :url "http://www.eclipse.org/legal/epl-v10.html"}
  :main depthmap.core/main
  :repositories {"local" "file://maven_repository"}
  :dependencies [[org.clojure/clojure "1.5.1"]
                 [incanter "1.5.5"]
                 [net.mikera/imagez "0.3.1"]])

