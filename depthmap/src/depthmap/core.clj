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
  (let [[a b c] theta
        inner (* -1 (/ (Math/pow (- x b) 2) (* 2 (* c c))))]
    (* a (Math/exp inner))))

(defn fit-gaussian
  "Fits a gaussian curve to a set of points. Returns the curve's parameters."
  [xs ys]
  (let [logged (map #(Math/log %) ys)
        a 1 ; TODO
        b 2
        c 3]
    [a b c]))

(defn max-gaussian
  [a b c]
  b)

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
  "Encodes red, green, and blue into an integer."
  [r g b]
  (let [s #(bit-shift-left (bit-and %1 0xFF) %2)]
    (bit-or (s r 16) (s g 8) b)))

(defn rgb-to-intensity
  "Intensity of an int-encoded RGB value."
  [rgb]
  (let [r (bit-shift-right (bit-and rgb 0xFF0000) 16)
        g (bit-shift-right (bit-and rgb 0xFF00) 8)
        b (bit-and rgb 0xFF)]
    (float (div (+ r g b) 3))))

(defn argmax
  "Index of maximum value in sequence."
  [vs]
  (first (apply max-key second (map-indexed vector vs))))

(defn main
  [& filenames]
  (if (= (count filenames) 0)
    (println "Must specify at least one image filename.")
    (let [src-images (map load-an-image filenames)
          z-resolution (int (div 255 (count src-images)))
          width (.getWidth (first src-images))
          height (.getHeight (first src-images))
          num-pixels (* width height)
          xs (range width)
          ys (range height)
          coordinates (cart [xs ys])
          focus-images (map convolve-laplacian src-images)
          ;_ (do (def plot (scatter-plot))
          ;      (view plot))
          depth-map (map (fn [[x y]]
                           (let [intensities (map #(rgb-to-intensity (.getRGB % x y)) focus-images)
                                 ;zs (range (count intensities))
                                 ;_ (let [smallest (apply min intensities)
                                 ;      normed (map #(- % smallest) intensities)]
                                 ;  (add-lines plot zs normed))
                                 ;focus-curve (fit-gaussian zs intensities)
                                 ;depth (apply max-gaussian focus-curve)]
                                 depth (argmax intensities)]
                             [[x y] depth]))
            coordinates)
          blank-depth-image (BufferedImage. width height BufferedImage/TYPE_INT_RGB)
          depth-image (doall (map (fn [[[x y] depth]]
                                    (let [dc (* depth z-resolution)]
                                          ;_ (println (format "%x" (to-rgb dc dc dc)))]
                                      (.setRGB blank-depth-image x y (to-rgb dc dc dc))))
                                  depth-map))]
      ; display depth map
    (show blank-depth-image)
    )))
