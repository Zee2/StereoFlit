#include <stereokit.h>
#include <stereokit_ui.h>
#include <flutter_embedder.h>
#include <flutter_surface.hpp>
#include <iostream>
#include <fstream>
#include <chrono>
#include <filesystem>
using namespace sk;

FlutterEngine engine;

double dpi = 2.0;
pose_t windowPose = pose_identity;

color32* render_surface;
sk::tex_t flutter_texture;

mesh_t     quad_mesh;
mesh_t     cube_mesh;
material_t cube_mat;
pose_t     cube_pose = {{0,0,-0.5f}, quat_identity};

static_assert(FLUTTER_ENGINE_VERSION == 1,
              "This Flutter Embedder was authored against the stable Flutter "
              "API at version 1. There has been a serious breakage in the "
              "API. Please read the ChangeLog and take appropriate action "
              "before updating this assertion");

void printUsage() {
  std::cout << "usage: flutterkit_embedder <path to flutter project> <path to icudtl.dat>"
            << std::endl;
}

bool FileExists(std::string filename) {
    std::ifstream file(filename);
    if(file.is_open()) {
        return 1;
        file.close();
    }
    else {
        return 0;
    }
}

void OnVsync(intptr_t baton) {
    std::chrono::nanoseconds current_time =
        std::chrono::nanoseconds(FlutterEngineGetCurrentTime());
    std::chrono::nanoseconds frame_interval = std::chrono::nanoseconds((long)((1/60.0) * 1000000000.0));
    FlutterEngineOnVsync(engine, baton,
        current_time.count() + frame_interval.count(),
        current_time.count() + (frame_interval * 2).count());
}

bool RunFlutter(const std::string& project_path,
                const std::string& icudtl_path) {

    std::cout << "cwd = " << std::filesystem::current_path() << std::endl;

    // Build our Flutter renderer config.
    FlutterRendererConfig config = {};

    render_surface = (color32*)malloc(sizeof(color32) * 512 * 512);

    config.type = kSoftware;
    config.software.struct_size = sizeof(FlutterSoftwareRendererConfig);
    config.software.surface_present_callback = [](void* user_data,
                                                const void* allocation,
                                                size_t row_bytes,
                                                size_t height) {
        std::cout << "Blit" << std::endl;
        memcpy(render_surface, allocation, row_bytes * height);
        return true;
    };

    std::string assets_path = project_path + "/build/flutter_assets";
    std::string blob_path = assets_path + "/kernel_blob.bin";

    std::cout << "Blob path: " << blob_path << std::endl;

    if (FileExists(blob_path))
    {
        std::cout << "Found Flutter kernel blob inside flutter project at " << blob_path << std::endl;
    }
    else
    {
        std::cout << "Couldn't find kernel blob, Flutter will complain about this! It probably needs an absolute path to the project. (Looked at: " + blob_path << ")" << std::endl;
    }
    
    FlutterProjectArgs args = {
        .struct_size = sizeof(FlutterProjectArgs),
        .assets_path = assets_path.c_str(),
        .icu_data_path = icudtl_path.c_str(),  // Find this in your bin/cache directory.
        // .compositor = &compositor
        .vsync_callback = [](void* user_data, intptr_t baton) -> void {
            std::cout << "Vsync" << std::endl;
            OnVsync(baton);
        }
    };

    // FlutterEngine engine = nullptr;
    FlutterEngineResult result =
        FlutterEngineRun(FLUTTER_ENGINE_VERSION, &config,  // renderer
                        &args, nullptr, &engine);
                        
    if (result != kSuccess || engine == nullptr) {
        std::cout << "Could not run the Flutter Engine." << std::endl;
        return false;
    }

    FlutterEngineDisplay display = {};
    display.struct_size = sizeof(FlutterEngineDisplay);
    display.display_id = 0;
    display.single_display = true;
    display.refresh_rate = 60.0;

    std::vector<FlutterEngineDisplay> displays = {display};
    FlutterEngineNotifyDisplayUpdate(engine,
                                        kFlutterEngineDisplaysUpdateTypeStartup,
                                        displays.data(), displays.size());

    FlutterWindowMetricsEvent event = {};
    event.struct_size = sizeof(event);
    event.width = 512;
    event.height = 512;
    event.pixel_ratio = 2.0;
    FlutterEngineSendWindowMetricsEvent(
        engine,
        &event);

    return true;
}

void SendDPIUpdate(double pixel_ratio)
{
    FlutterWindowMetricsEvent event = {};
    event.struct_size = sizeof(event);
    event.width = 512;
    event.height = 512;
    event.pixel_ratio = pixel_ratio;
    FlutterEngineSendWindowMetricsEvent(
        engine,
        &event);
}




// Borrowed from https://github.com/StereoKit/StereoKit-Browser/blob/main/Browser.cs
touch_point_t TouchPoint(bounds_t bounds, handed_ handedness)
{
    const hand_t* hand   = input_hand(handedness);
    hand_joint_t  joint  = hand->fingers[1][4];
    
    vec3 pointOnPlane = vec3(bounds.center.x, bounds.center.y, bounds.center.z - bounds.dimensions.z / 2);
    vec3 planeNormal = vec3_forward;
    plane_t plane  = { planeNormal, -vec3_dot(pointOnPlane, planeNormal) };

    vec3 at = plane_point_closest(plane, hierarchy_to_local_point(joint.position));

    vec3 pt = (at - (bounds.center + (bounds.dimensions*0.5f)));
    pt = vec3(-pt.x / bounds.dimensions.x, -pt.y / bounds.dimensions.y, 0);

    return touch_point_t {
        .pos = vec2(pt.x * 512, pt.y * 512),
        .radius = joint.radius
    };
}

// FlutterSurface* flutterSurface;

int main(int argc, const char* argv[]) {

    // Check arg count, remind usage
    if (argc != 3) {
        printUsage();
        return 1;
    }

    sk_settings_t settings = {};
	settings.app_name           = "SKNativeTemplate";
	settings.assets_folder      = "Assets";
	settings.display_preference = display_mode_mixedreality;
	if (!sk_init(settings))
		return 1;

    // Grab the path to the flutter project and icudtl
    std::string flutter_project_path = argv[1];
    std::string flutter_icudtl_path = argv[2];

    std::cout << "Booting Flutter app at path: " << flutter_project_path << std::endl;

    // flutterSurface = new FlutterSurface(
    //     1024, 1024,
    //     2.0,
    //     flutter_project_path,
    //     flutter_icudtl_path
    // );

    // Set up the texture we'll be blitting to.
    // tex_type_dynamic for software rendering for now.
    flutter_texture = sk::tex_create(tex_type_dynamic, tex_format_rgba32);

    RunFlutter(flutter_project_path, flutter_icudtl_path);

    quad_mesh = mesh_find(default_id_mesh_quad);
    cube_mesh = mesh_gen_rounded_cube(vec3_one * 0.1f, 0.02f, 4);
    cube_mat  = material_copy(material_find(default_id_material));
    material_set_texture(cube_mat, "diffuse", flutter_texture);

    sk_run([]() {
        ui_handle_begin("Cube", cube_pose, mesh_get_bounds(cube_mesh), false);
        render_add_mesh(cube_mesh, cube_mat, matrix_identity);
        ui_handle_end();

        // sk::tex_set_colors(
        //     flutter_texture,
        //     (int32_t)(flutterSurface->GetPixelWidth()),
        //     (int32_t)(flutterSurface->GetPixelHeight()),
        //     flutterSurface->GetRenderSurface());

        sk::tex_set_colors(
            flutter_texture,
            512,
            512,
            render_surface
        );

        ui_window_begin("Flutter", windowPose, vec2_one * 0.3f);

        sk::bounds_t flutter_bounds = ui_layout_reserve(vec2_one * ui_layout_remaining().x);
        flutter_bounds.center.z += 0.01f;
        flutter_bounds.dimensions.z += 0.03f;

        handed_ handedness;
        button_state_ press_state = ui_volumei_at("flutterSurface", flutter_bounds, ui_confirm_push, &handedness);

        render_add_mesh(quad_mesh, cube_mat, matrix_ts(flutter_bounds.center + vec3_forward * 0.015f, flutter_bounds.dimensions));
        
        touch_point_t touch = TouchPoint(flutter_bounds, handedness);

        if (press_state & button_state_just_active)
        {
            std::cout << "Press down!" << std::endl;
            // flutterSurface->SendPointerEvent(touch, kDown);
            FlutterPointerEvent event = {};
            event.struct_size = sizeof(event);
            event.phase = kDown;
            event.x = touch.pos.x;
            event.y = touch.pos.y;
            event.timestamp = FlutterEngineGetCurrentTime();
            FlutterEngineSendPointerEvent(engine, &event, 1);
        }
        if (press_state & button_state_active)
        {
            std::cout << "Move!" << std::endl;
            // flutterSurface->SendPointerEvent(touch, kMove);
            FlutterPointerEvent event = {};
            event.struct_size = sizeof(event);
            event.phase = kMove;
            event.x = touch.pos.x;
            event.y = touch.pos.y;
            event.timestamp = FlutterEngineGetCurrentTime();
            FlutterEngineSendPointerEvent(engine, &event, 1);
        }
        if (press_state & button_state_just_inactive)
        {
            std::cout << "Up!" << std::endl;
            // flutterSurface->SendPointerEvent(touch, kUp);
            FlutterPointerEvent event = {};
            event.struct_size = sizeof(event);
            event.phase = kUp;
            event.x = touch.pos.x;
            event.y = touch.pos.y;
            event.timestamp = FlutterEngineGetCurrentTime();
            FlutterEngineSendPointerEvent(engine, &event, 1);
        }

        // ui_label("Pixel ratio (dpi)");
        // ui_sameline();
        // if (ui_hslider_f64("DPI", dpi, 1.0, 4.0))
        // {
        //     SendDPIUpdate(dpi);
        // }
        ui_window_end();
	});

    tex_release(flutter_texture);
    // free(flutterSurface);
    free(render_surface);

    return 0;
}