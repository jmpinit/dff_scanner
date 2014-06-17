(ns depthmap.core
  (:import [javax.imageio ImageIO])
  (:import [java.awt Graphics])
  (:import [java.awt.image BufferedImage ConvolveOp Kernel])
  (use [incanter core optimize charts datasets])
  (use [mikera.image.core :only [show]]))

(defn cart [colls]
  "Cartesian product of sequences."
  (if (empty? colls)
    '(())
    (for [x (first colls)
          more (cart (rest colls))]
      (cons x more))))

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

(defn convolve-laplacian
  [image]
  (let [laplacian [0 -1  0
                   -1  4 -1
                   0 -1  0]]
    (convolve image laplacian)))

(defn to-rgb
  "Converts intensity 0-255 to an int-encoded RGB value."
  [intensity]
  (java.awt.Color/HSBtoRGB 0 0 intensity))

(defn main
  [& filenames]
  (if (= (count filenames) 0)
    (println "Must specify at least one image filename.")
    (let [src-images (map load-an-image filenames)
          width (.getWidth (first src-images))
          height (.getHeight (first src-images))
          num-pixels (* width height)
          xs (range width)
          ys (range height)
          coordinates (cart [xs ys])
          focus-images (map convolve-laplacian src-images)
          depth-map (map (fn [[x y]]
                           (let [intensities (map #(.getRGB % x y) focus-images)
                                 zs (range (count intensities))
                                 start [1.0 0.0 1.0 0.0]
                                 depth (non-linear-model gaussian intensities zs start)]
                             [[x y] depth]))
            coordinates)
          blank-depth-image (BufferedImage. width height BufferedImage/TYPE_INT_RGB)
          depth-image (map (fn [[x y] depth]
                             (.setRGB blank-depth-image x y (to-rgb depth)))
                           depth-map)]
    ; display depth map
    (count depth-map)
    (show blank-depth-image))))
