#include <flutter_surface.hpp>
#include <flutter_embedder.h>
#include <stereokit.h>
#include <stereokit_ui.h>
#include <chrono>

void OnVsync(FlutterEngine engine, intptr_t baton) {
    std::chrono::nanoseconds current_time =
        std::chrono::nanoseconds(FlutterEngineGetCurrentTime());
    std::chrono::nanoseconds frame_interval = std::chrono::nanoseconds((long)((1/60.0) * 1000000000.0));
    FlutterEngineOnVsync(engine, baton,
        current_time.count(),
        current_time.count() + frame_interval.count());
}

FlutterSurface::FlutterSurface(const char* id, size_t pixel_width, size_t pixel_height,
                               double pixel_ratio,
                               const std::string& project_path,
                               const std::string& icudtl_path) :
                                                        id(id),
                                                        pixel_width(pixel_width),
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
            FlutterSurface* surface = (FlutterSurface*)user_data;
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
    event.pixel_ratio = pixel_ratio;

    result = FlutterEngineSendWindowMetricsEvent(engine, &event);
    if (result != kSuccess) {
        std::cout << "Failed to send window metrics event: " << result << std::endl;
    }

    // Setup the sk resources.
    texture = sk::tex_create(sk::tex_type_dynamic, sk::tex_format_bgra32);
    quad_mesh = sk::mesh_find(sk::default_id_mesh_quad);
    quad_mat = sk::material_copy(sk::material_find(sk::default_id_material));
    sk::material_set_transparency(quad_mat, sk::transparency_blend);
    sk::material_set_texture(quad_mat, "diffuse", texture);
}

FlutterSurface::~FlutterSurface()
{
    FlutterEngineShutdown(engine);
    free(render_surface);
    sk::tex_release(texture);
    sk::mesh_release(quad_mesh);
}

void FlutterSurface::Draw(sk::bounds_t bounds)
{
    // Blit from the Flutter surface into the Stereokit texture.
    sk::tex_set_colors(
            texture,
            (int32_t)(pixel_width),
            (int32_t)(pixel_height),
            render_surface);

    
    sk::button_state_ hover_state;
    sk::button_state_ click_state;

    sk::handed_ handedness = sk::handed_right;
    float offset = 0;

    sk::ui_settings_t prev_settings = sk::ui_get_settings();
    sk::ui_settings_t new_settings = prev_settings;
    // new_settings.depth = 0.05f;

    // Push the updated UI depth setting, if we updated it!
    sk::ui_settings(new_settings);

    sk::ui_button_behavior(
        bounds.center + (bounds.dimensions * 0.5f),
        sk::vec2(bounds.dimensions.x, bounds.dimensions.y),
        sk::ui_stack_hash(id),
        offset,
        click_state,
        hover_state);

    // Pop/restore the old UI settings.
    sk::ui_settings(prev_settings);
    
    // Draw the surface.
    render_add_mesh(quad_mesh, quad_mat, matrix_ts(bounds.center, bounds.dimensions));
    
    if (sk::ui_last_element_hand_used(sk::handed_right) & sk::button_state_active)
    {
        handedness = sk::handed_right;
    }
    else if (sk::ui_last_element_hand_used(sk::handed_left) & sk::button_state_active)
    {
        handedness = sk::handed_left;
    }
    
    touch_point_t touch_pt = GetTouchPoint(bounds, handedness);

    bool printed = false;

    if (hover_state & sk::button_state_just_active)
    {
        printed = true;
        std::cout << "Adding (" << touch_pt.pos.x << ", " << touch_pt.pos.y << ")";
        SendPointerEvent(touch_pt, FlutterPointerPhase::kAdd);
    }

    if (hover_state & sk::button_state_active &&
        !(hover_state & sk::button_state_changed) &&
        !(click_state & sk::button_state_active) &&
        !(click_state & sk::button_state_changed))
    {
        printed = true;
        std::cout << "Hover (" << touch_pt.pos.x << ", " << touch_pt.pos.y << ")";
        SendPointerEvent(touch_pt, FlutterPointerPhase::kHover);
    }

    if (click_state & sk::button_state_just_active)
    {
        printed = true;
        std::cout << "Down (" << touch_pt.pos.x << ", " << touch_pt.pos.y << ")";
        SendPointerEvent(touch_pt, FlutterPointerPhase::kDown);
    }

    if (click_state & sk::button_state_active)
    {
        printed = true;
        std::cout << "Mov (" << touch_pt.pos.x << ", " << touch_pt.pos.y << ")";
        SendPointerEvent(touch_pt, FlutterPointerPhase::kMove);
    }

    if (click_state & sk::button_state_just_inactive)
    {
        printed = true;
        std::cout << "Up (" << touch_pt.pos.x << ", " << touch_pt.pos.y << ")";
        SendPointerEvent(touch_pt, FlutterPointerPhase::kUp);
    }    

    if (hover_state & sk::button_state_just_inactive)
    {
        printed = true;
        std::cout << "Removing (" << touch_pt.pos.x << ", " << touch_pt.pos.y << ")";
        SendPointerEvent(touch_pt, FlutterPointerPhase::kRemove);
    }

    if (printed) std::cout << std::endl;
}

touch_point_t FlutterSurface::GetTouchPoint(sk::bounds_t bounds, sk::handed_ handedness)
{
    const sk::hand_t* hand   = input_hand(handedness);
    sk::hand_joint_t  joint  = hand->fingers[1][4];
    
    sk::vec3 pointOnPlane = sk::vec3(bounds.center.x, bounds.center.y, bounds.center.z - bounds.dimensions.z / 2);
    sk::vec3 planeNormal = sk::vec3_forward;
    sk::plane_t plane  = { planeNormal, -vec3_dot(pointOnPlane, planeNormal) };

    sk::vec3 at = sk::plane_point_closest(plane, sk::hierarchy_to_local_point(joint.position));

    sk::vec3 pt = (at - (bounds.center + (bounds.dimensions * 0.5f)));
    pt = sk::vec3(-pt.x / bounds.dimensions.x, -pt.y / bounds.dimensions.y, 0);

    sk::mesh_draw(sk::mesh_find(sk::default_id_mesh_sphere), sk::material_find(sk::default_id_material_unlit), sk::matrix_ts(at, sk::vec3_one * 0.01f));

    return touch_point_t {
        .pos = sk::vec2(pt.x, pt.y),
        .depth = pt.z,
        .radius = joint.radius,
        .handedness = handedness
    };
}

void FlutterSurface::SendPointerEvent(touch_point_t touchPoint, FlutterPointerPhase phase, int64_t time_offset)
{
    FlutterPointerEvent event = {};
    event.struct_size = sizeof(event);
    event.phase = phase;
    event.x = touchPoint.pos.x * pixel_width;
    event.y = touchPoint.pos.y * pixel_height;
    auto now_ns = std::chrono::nanoseconds(FlutterEngineGetCurrentTime());
    event.timestamp = std::chrono::duration_cast<std::chrono::microseconds>(now_ns).count();

    // We're emulating touchscreen input right now. This has the advantage
    // of proper drag-scrolling, but the disadvantage of not issuing any hover events. :(
    // Come on, Flutter...
    event.device_kind = kFlutterPointerDeviceKindTouch;
    event.device = touchPoint.handedness == sk::handed_left ? 0 : 1;

    // You need to populate these if you're using kFlutterPointerDeviceKindMouse.
    // event.buttons = (phase == kDown || phase == kMove || phase == kPanZoomStart || phase == kPanZoomUpdate || phase == kPanZoomEnd) ? kFlutterPointerButtonMousePrimary : 0;
    event.buttons = 0;
    FlutterEngineSendPointerEvent(engine, &event, 1);
}

void FlutterSurface::SetPixelRatio(double ratio)
{
    pixel_ratio = ratio;

    FlutterWindowMetricsEvent event = {};
    event.struct_size = sizeof(event);
    event.width = pixel_width;
    event.height = pixel_height;
    event.pixel_ratio = pixel_ratio;

    auto result = FlutterEngineSendWindowMetricsEvent(engine, &event);
    if (result != kSuccess) {
        std::cout << "Failed to send window metrics event: " << result << std::endl;
    }
}