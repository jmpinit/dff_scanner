(ns depthmap.core
  (:import [javax.imageio ImageIO])
  (:import [java.awt Graphics])
  (:import [java.awt.image BufferedImage ConvolveOp Kernel])
  (use [mikera.image.core :only [show]]))
  ;(use '(incanter core optimize charts datasets))

(defn gaussian
  [theta x]
  (let [[a b c d] theta
        inner (* -1 (/ (Math/pow (- x b) 2) (* 2 (* c c))))]
    (+ d (* a (Math/exp inner)))))

(defn clean-image
  "Remove incompatible data to work around image operation bugs in ConvolveOp."
  [image]
  (let [w (.getWidth image)
        h (.getHeight image)
        buffed-image (BufferedImage. w h BufferedImage/TYPE_INT_RGB)
        graphics (.getGraphics buffed-image)]
    (doto graphics
      (.drawImage image 0 0 nil)
      (.dispose))
    buffed-image))

(defn load-an-image
  [filename]
  (clean-image (ImageIO/read (java.io.File. filename))))

(defn convolve
  [image kernel]
  (let [kernel-obj (Kernel. 3 3 (into-array Float/TYPE kernel))
        convolve-obj (ConvolveOp. kernel-obj (ConvolveOp/EDGE_NO_OP) nil)
        image-dest (BufferedImage. (.getWidth image) (.getHeight image) (BufferedImage/TYPE_INT_RGB))]
    (.filter convolve-obj image image-dest)
    image-dest))

(defn main
  [&args]
  (let [sample (load-an-image "./resources/images/marine/marine-1.png")
        laplacian [0 -1  0
                  -1  4 -1
                   0 -1  0]]
    (show (convolve sample laplacian))))
