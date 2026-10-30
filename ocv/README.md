# skink-cv/ocv — OpenCV backend

Optional backend that delegates IO and heavy filters to system OpenCV 4.x via `shim.cpp` (a tiny C ABI bridge — the Skink side only needs `extern fn` declarations).

## Prerequisites

Debian/Ubuntu:

```
sudo apt install libopencv-dev
```

Fedora:

```
sudo dnf install opencv-devel
```

macOS (Homebrew — link paths differ, see below):

```
brew install opencv
```

## Install + build

```
scales add markoxley/skink-cv/ocv
```

The subpackage's `skink.toml` carries a `[link]` section that compiles `shim.cpp` into your binary and links `opencv_core`, `opencv_imgproc`, `opencv_imgcodecs`, `opencv_objdetect`, `opencv_videoio`.

The manifest's `cflags` assume the Debian layout (`-I/usr/include/opencv4`). If your distro puts headers elsewhere, either:

- edit `mod/markoxley/skink-ocv.../ocv/skink.toml` in the module cache,
- or export `SKINK_CLANG_FLAGS="-I/path/to/opencv4 -L/path/to/lib"` when building (env flags are appended by the driver).

## API

```skink
import "cvocv"
import "cv_ops"

im, err := cvocv.Load("photo.jpg")          // cv::imread — JPEG/PNG/TIFF/…
cvocv.Save("out.png", &im)                  // cv::imwrite

blur  := cvocv.GaussianBlur(&im, 3.0)       // cv::GaussianBlur
edges := cvocv.Canny(&im, 100.0, 200.0)     // cv::Canny on grayscale

faces, ferr := cvocv.DetectFaces(&im,       // Haar cascade
    "/usr/share/opencv4/haarcascades/haarcascade_frontalface_default.xml", 16)
```

All functions interoperate with the pure-Skink core: `cvocv.Load` returns a regular `cv_img.Image`, so `cv_ops`/`cv_filter`/`cv_feat` work unchanged on OpenCV-decoded images.
