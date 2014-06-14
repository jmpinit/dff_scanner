(ns depthmap.core
  (use [mikera.image.core :only [show]]))
  ;(use '(incanter core optimize charts datasets))

(defn gaussian
  [theta x]
  (let [[a b c d] theta
        inner (* -1 (/ (Math/pow (- x b) 2) (* 2 (* c c))))]
    (+ d (* a (Math/exp inner)))))

(defn load-an-image
  [filename]
  (javax.imageio.ImageIO/read (java.io.File. filename)))

(defn main
  [&args]
  (show (.getSubimage (load-an-image "./resources/images/test-series.png") 0 0 1692 1135)))
