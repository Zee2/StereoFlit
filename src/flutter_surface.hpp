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
    // The normalized 0..1 position of the touch point.
    sk::vec2 pos;
    // The hierarchy-local depth (i.e., Z coordinate) of the touch point.
    float depth;
    // The normalized 0..1 radius of the touch point.
    float radius;
    // The handedness that generated this touchpoint.
    sk::handed_ handedness;
} touch_point_t;

// A surface that embeds its own Flutter instance
// and provides a handle to a chunk of native memory
// representing an RGBA32 buffer, which is written into
// by the Flutter engine.
class FlutterSurface
{
    private:
        const char* id;
        size_t pixel_width;
        size_t pixel_height;
        double pixel_ratio;
        double init_x;
        double init_y;
        FlutterEngine engine;
        sk::color32* render_surface;
        sk::tex_t texture;
        sk::mesh_t quad_mesh;
        sk::material_t quad_mat;
        touch_point_t GetTouchPoint(sk::bounds_t bounds, sk::handed_ handedness);
        void CopyBuffer(void* user_data, const void* allocation, size_t row_bytes, size_t height);

    public:
        FlutterSurface(const char* id,
                       size_t pixel_width, size_t pixel_height,
                       double pixel_ratio,
                       const std::string& project_path,
                       const std::string& icudtl_path);
        ~FlutterSurface();

        // Obtain a handle to the underlying RGBA32 buffer.
        sk::color32* GetRenderSurface() { return render_surface; }

        // Dispatch a pointer event to the Flutter engine.
        void SendPointerEvent(touch_point_t touchPoint, FlutterPointerPhase phase, int64_t time_offset = 0);

        // Returns the width (in physical pixels) of the underlying RGBA32 buffer.
        size_t GetPixelWidth() { return pixel_width; }

        // Obtain the height (in physical pixels) of the underlying RGBA32 buffer.
        size_t GetPixelHeight() { return pixel_height; }

        // Update/tick and draw the Flutter surface at the specified bounds.
        void Draw(sk::bounds_t bounds);
};

#endif // FLUTTER_SURFACE_HPP