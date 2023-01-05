#ifndef FLUTTER_SURFACE_HPP
#define FLUTTER_SURFACE_HPP

#include <flutter_embedder.h>
#include <stereokit.h>
#include <iostream>

static_assert(FLUTTER_ENGINE_VERSION == 1,
              "This Flutter Embedder was authored against the stable Flutter "
              "API at version 1. There has been a serious breakage in the "
              "API. Please read the ChangeLog and take appropriate action "
              "before updating this assertion");

typedef struct {
    sk::vec2 pos;
    float radius;
} touch_point_t;

// A surface that embeds its own Flutter instance
// and provides a handle to a chunk of native memory
// representing an RGBA32 buffer, which is written into
// by the Flutter engine.
class FlutterSurface
{
    private:
        size_t pixel_width;
        size_t pixel_height;
        double pixel_ratio;
        FlutterEngine engine;
        sk::color32* render_surface;
        void CopyBuffer(void* user_data, const void* allocation, size_t row_bytes, size_t height);

    public:
        FlutterSurface(size_t pixel_width, size_t pixel_height,
                       double pixel_ratio,
                       const std::string& project_path,
                       const std::string& icudtl_path);
        ~FlutterSurface();
        sk::color32* GetRenderSurface() { return render_surface; }
        void SendPointerEvent(touch_point_t touchPoint, FlutterPointerPhase phase);
        size_t GetPixelWidth() { return pixel_width; }
        size_t GetPixelHeight() { return pixel_height; }
};

#endif // FLUTTER_SURFACE_HPP