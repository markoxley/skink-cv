// File:    shim.cpp
// Purpose: C ABI bridge between skink-cv's ocv package and OpenCV's C++
//          API. Every function takes raw pixel buffers + scalars so the
//          Skink side needs only extern fns — no C++ interop required.
//          Buffers are row-major, interleaved channels (RGB order,
//          converted to/from OpenCV's BGR internally).
//
//          Build: handled automatically by skink.toml's [link] section
//          (sources + libs + cflags). See README.md.
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/objdetect.hpp>
#include <cstdlib>
#include <cstring>
#include <vector>

extern "C" {

// skinkcv_read loads an image file. Returns a malloc'd pixel buffer
// (caller frees via skinkcv_free) or NULL on failure. *w/*h/*ch are
// outputs; ch is always 3 (BGR→RGB converted).
unsigned char* skinkcv_read(const char* path, int* w, int* h, int* ch) {
    cv::Mat m = cv::imread(path, cv::IMREAD_COLOR);
    if (m.empty()) return NULL;
    cv::Mat rgb;
    cv::cvtColor(m, rgb, cv::COLOR_BGR2RGB);
    *w = rgb.cols;
    *h = rgb.rows;
    *ch = 3;
    size_t n = (size_t)rgb.total() * 3;
    unsigned char* out = (unsigned char*)malloc(n);
    memcpy(out, rgb.data, n);
    return out;
}

// skinkcv_free releases buffers returned by the shim.
void skinkcv_free(void* p) { free(p); }

// skinkcv_write writes an RGB/gray pixel buffer to an image file
// (format chosen by path extension). Returns 0 on success.
int skinkcv_write(const char* path, const unsigned char* px,
                  int w, int h, int ch) {
    int cvType = (ch == 1) ? CV_8UC1 : CV_8UC3;
    cv::Mat m(h, w, cvType, (void*)px);
    cv::Mat bgr;
    const cv::Mat* out = &m;
    if (ch == 3) {
        cv::cvtColor(m, bgr, cv::COLOR_RGB2BGR);
        out = &bgr;
    }
    return cv::imwrite(path, *out) ? 0 : -1;
}

// skinkcv_blur applies Gaussian blur in place. px is w*h*ch bytes.
// Returns 0 on success.
int skinkcv_blur(unsigned char* px, int w, int h, int ch, double sigma) {
    int cvType = (ch == 1) ? CV_8UC1 : CV_8UC3;
    cv::Mat m(h, w, cvType, px);
    cv::GaussianBlur(m, m, cv::Size(0, 0), sigma);
    return 0;
}

// skinkcv_canny runs Canny edge detection. `in` is grayscale
// (w*h bytes), `out` receives the edge map. Returns 0 on success.
int skinkcv_canny(const unsigned char* in, unsigned char* out,
                  int w, int h, double t1, double t2) {
    cv::Mat src(h, w, CV_8UC1, (void*)in);
    cv::Mat dst(h, w, CV_8UC1, out);
    cv::Canny(src, dst, t1, t2);
    return 0;
}

// skinkcv_detect_faces runs a Haar cascade over grayscale pixels.
// Out params: rects receives up to maxRects [x,y,w,h] quads; returns
// the count written.
int skinkcv_detect_faces(const char* cascadePath, const unsigned char* gray,
                         int w, int h, int* rects, int maxRects) {
    cv::CascadeClassifier cc;
    if (!cc.load(cascadePath)) return -1;
    cv::Mat img(h, w, CV_8UC1, (void*)gray);
    std::vector<cv::Rect> faces;
    cc.detectMultiScale(img, faces, 1.1, 3, 0, cv::Size(24, 24));
    int n = (int)faces.size();
    if (n > maxRects) n = maxRects;
    for (int i = 0; i < n; i++) {
        rects[i * 4] = faces[i].x;
        rects[i * 4 + 1] = faces[i].y;
        rects[i * 4 + 2] = faces[i].width;
        rects[i * 4 + 3] = faces[i].height;
    }
    return n;
}

} // extern "C"
