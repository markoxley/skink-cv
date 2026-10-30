# skink-cv

Computer vision for Skink. A **pure-Skink core** — no native dependencies — covering image codecs (PNG/BMP/PPM), geometry ops, convolution filters, and feature extraction. An optional **OpenCV backend** lives in the `ocv/` subpackage.

## Install

```
scales add markoxley/skink-cv
scales add markoxley/skink-cv/ocv   # optional OpenCV backend
```

## Quick start

```skink
import "cv"
import "cv_filter"

im, err := cv.Load("photo.png")           // PNG/BMP/PNM auto-detected
g   := cv.Grayscale(&im)
edges := cv.Edges(&g)                     // Sobel gradient magnitude
cv.Save("edges.png", &edges)
```

## Module layout

| Module      | Contents                                                                                                  |
| ----------- | --------------------------------------------------------------------------------------------------------- |
| `cv_img`    | `Image{W,H,Channels,Pix []float}` (0–255), Get/Set/Clone/ToBytes                                          |
| `cv_png`    | PNG 8-bit decode (all 5 filters, color types 0/2/3/4/6) + encode                                          |
| `cv_bmp`    | BMP 24/32-bit decode (top-down + bottom-up), 24-bit encode                                                |
| `cv_ppm`    | PNM decode (P2/P3/P5/P6, comments ok) + P6/P5 encode                                                      |
| `cv_io`     | `Load` (magic-sniffed) + `Save` (extension-dispatched)                                                    |
| `cv_ops`    | `Grayscale`, `Crop`, `ResizeNearest`, `ResizeBilinear`, `FlipH/V`, `Rotate90/270`, `Pad`                  |
| `cv_filter` | `Convolve`, `BoxBlur`, `GaussianBlur`, `Sharpen`, `Sobel`, `Threshold`, `Invert`, `Histogram`, `Equalize` |
| `cv_feat`   | `Components` (union-find blob labeling + bounding boxes), `HarrisScore`, `LocalMaxima`                    |
| `ocv/`      | OpenCV backend: `Load`/`Save` via `imread`/`imwrite` (JPEG etc.), `GaussianBlur`, `Canny`, `DetectFaces`  |

## Examples

**Find blobs:**

```skink
lbl, blobs := cv_feat.Components(&gray, 128.0)
for i := 0; i < len(blobs); i = i + 1 {
    println("blob at ", blobs[i].X, ",", blobs[i].Y,
            " area=", blobs[i].Area)
}
```

**Corner detection:**

```skink
score := cv_feat.HarrisScore(&gray, 0.04)
pts := cv_feat.LocalMaxima(&score, 1000.0, 3)
```

**Histogram equalization:**

```skink
eq := cv_filter.Equalize(&gray)
```

## Notes

- Pixels are floats in [0,255]; `ToBytes` clamps + rounds for codecs.
- PNG decode is 8-bit, non-interlaced only; alpha is dropped (palette images expand to RGB). Encode writes filter-0 zlib streams with correct CRCs.
- BMP decode covers uncompressed BI_RGB 24/32-bit only.
- The `ocv` backend needs system OpenCV — see `ocv/README.md`.

## Tests

```
bash run_tests.sh
```

13 tests cover the image model, in-memory PNG/BMP/PPM round-trips, P3 ASCII parsing, all geometry ops, filters, histograms, connected components, Harris corners, and file IO. The encoder output has also been verified byte-for-byte against Python zlib/struct parsing.
