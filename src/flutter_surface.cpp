#include <flutter_surface.hpp>
#include <flutter_embedder.h>
#include <chrono>

void OnVsync(FlutterEngine engine, intptr_t baton) {
    std::chrono::nanoseconds current_time =
        std::chrono::nanoseconds(FlutterEngineGetCurrentTime());
    std::chrono::nanoseconds frame_interval = std::chrono::nanoseconds((long)((1/60.0) * 1000000000.0));
    FlutterEngineOnVsync(engine, baton,
        current_time.count() + frame_interval.count(),
        current_time.count() + (frame_interval * 2).count());
}

FlutterSurface::FlutterSurface(size_t pixel_width, size_t pixel_height,
                               double pixel_ratio,
                               const std::string& project_path,
                               const std::string& icudtl_path) : pixel_width(pixel_width),
                                                         pixel_height(pixel_height),
                                                         pixel_ratio(pixel_ratio)
{
    std::cout << "Flutter bootup at project " << project_path << " and icudtl " << icudtl_path << std::endl;

    // Allocate a buffer for us to blit the rendered Flutter surface into.
    render_surface = (sk::color32*)malloc(sizeof(sk::color32) * pixel_width * pixel_height);

    // Configure Flutter renderer.
    // For now, we'll use the software renderer. Ideally OpenGL
    FlutterRendererConfig config = {};
    config.type = kSoftware;
    config.software.struct_size = sizeof(config.software);
    config.software.surface_present_callback = [](void* user_data,
                                                const void* allocation,
                                                size_t row_bytes,
                                                size_t height) {
        // Can't capture `this` in a SoftwareSurfacePresentCallback lambda.
        // We'll pass `this` around as our user_data baton.
        FlutterSurface* surface = (FlutterSurface*)user_data;
        memcpy(surface->render_surface, allocation, row_bytes * height);
        return true;
    };

    std::string assets_path = project_path + "/build/flutter_assets";

    FlutterProjectArgs args = {
        .struct_size = sizeof(FlutterProjectArgs),
        .assets_path = assets_path.c_str(),
        .icu_data_path = icudtl_path.c_str(),  // Find this in your bin/cache directory.
        // .compositor = &compositor
        .vsync_callback = [](void* user_data, intptr_t baton) -> void {
            std::cout << "Vsync!" << std::endl;
            FlutterSurface* surface = (FlutterSurface*)user_data;
            std::cout << "Surface ptr: " << surface << std::endl;
            OnVsync(surface->engine, baton);
        }
    };

    FlutterEngineResult result = FlutterEngineRun(FLUTTER_ENGINE_VERSION, &config, &args, this, &engine);
    if (result != kSuccess || engine == nullptr) {
        std::cout << "Failed to run Flutter engine: " << result << std::endl;
        return;
    }

    FlutterEngineDisplay display = {};
    display.struct_size = sizeof(FlutterEngineDisplay);
    display.display_id = 0;
    display.single_display = true;
    display.refresh_rate = 60.0;
    std::vector<FlutterEngineDisplay> displays = {display};
    result = FlutterEngineNotifyDisplayUpdate(engine,
                                        kFlutterEngineDisplaysUpdateTypeStartup,
                                        displays.data(), displays.size());

    if (result != kSuccess) {
        std::cout << "Failed to send FlutterEngineNotifyDisplayUpdate: " << result << std::endl;
        return;
    }

    // We need to pass a WindowMetricsEvent to the Flutter engine
    // at startup to tell it the size of the window, or else
    // nothing will be rendered/no vsyncs will occur/etc.
    FlutterWindowMetricsEvent event = {};
    event.struct_size = sizeof(event);
    event.width = pixel_width;
    event.height = pixel_height;
    event.pixel_ratio = 2.0;

    result = FlutterEngineSendWindowMetricsEvent(engine, &event);
    if (result != kSuccess) {
        std::cout << "Failed to send window metrics event: " << result << std::endl;
        return;
    }
}

FlutterSurface::~FlutterSurface()
{
    FlutterEngineShutdown(engine);
    free(render_surface);
}

void FlutterSurface::SendPointerEvent(touch_point_t touchPoint, FlutterPointerPhase phase)
{
    FlutterPointerEvent event = {};
    event.struct_size = sizeof(event);
    event.phase = phase;
    event.x = touchPoint.pos.x * pixel_width;
    event.y = touchPoint.pos.y * pixel_height;
    event.timestamp = FlutterEngineGetCurrentTime();
    FlutterEngineSendPointerEvent(engine, &event, 1);
}