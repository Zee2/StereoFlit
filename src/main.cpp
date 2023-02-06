#include <stereokit.h>
#include <stereokit_ui.h>
#include <flutter_embedder.h>
#include <flutter_surface.hpp>
#include <iostream>
#include <fstream>
#include <chrono>
#include <filesystem>
using namespace sk;

static_assert(FLUTTER_ENGINE_VERSION == 1,
              "This Flutter Embedder was authored against the stable Flutter "
              "API at version 1. There has been a serious breakage in the "
              "API. Please read the ChangeLog and take appropriate action "
              "before updating this assertion");

// Helpful container for a Flutter window,
// to keep track of an id for the window, a pose,
// and the handle to the FlutterSurface itself.
typedef struct {
    std::string id;
    pose_t windowPose;
    FlutterSurface* flutter;
} flutter_window_t;

std::vector<flutter_window_t> flutterWindows = { };

std::string flutter_project_path;
std::string flutter_icudtl_path;

// For the main options/menu window.
pose_t optionsWindowPose;

int main(int argc, const char* argv[]) {

    // Check arg count, remind usage
    if (argc != 3) {
        std::cout << "usage: flutterkit_embedder <path to flutter project> <path to icudtl.dat>" << std::endl;
        return 1;
    }

    sk_settings_t settings = {};
	settings.app_name           = "SKNativeTemplate";
	settings.assets_folder      = "Assets";
	settings.display_preference = display_mode_mixedreality;
    settings.blend_preference = display_blend_blend;
	if (!sk_init(settings))
		return 1;

    // Grab the path to the flutter project and icudtl
    flutter_project_path = argv[1];
    flutter_icudtl_path = argv[2];

    optionsWindowPose = ui_popup_pose(vec3_forward * 0.1f);

    sk_run([]() {
        ui_window_begin("Menu", optionsWindowPose);
        if (ui_button("Spawn Flutter window"))
        {
            auto num = flutterWindows.empty() ? 0 : flutterWindows.size();
            flutter_window_t newWindow;
            newWindow.id = "FlutterWindow" + std::to_string(num);
            newWindow.windowPose = ui_popup_pose(vec3_forward * 0.2f);
            newWindow.flutter = new FlutterSurface(
                ("FlutterSurface" + std::to_string(num)).c_str(),
                512, 128,
                1.0f,
                flutter_project_path,
                flutter_icudtl_path
            );

            flutterWindows.push_back(newWindow);
        }
        ui_window_end();

        for (auto& window : flutterWindows)
        {
            sk::ui_settings_t prev_settings = sk::ui_get_settings();
            sk::ui_settings_t new_settings = prev_settings;
            new_settings.margin = 0.001f;

            // Push the updated UI setting
            sk::ui_settings(new_settings);

            sk::vec2 flutter_size = sk::vec2(window.flutter->GetPixelWidth(), window.flutter->GetPixelHeight()) * 0.0005f;
            
            sk::vec2 window_size = sk::vec2(flutter_size.x + ui_get_margin() * 2, flutter_size.y + ui_get_margin() * 2);
            ui_window_begin(window.id.c_str(), window.windowPose, window_size, sk::ui_win_body, sk::ui_move_face_user);
            sk::bounds_t flutter_bounds = sk::ui_layout_reserve(sk::ui_layout_remaining());
            
            flutter_bounds.center.z -= 0.001f;
            flutter_bounds.dimensions.z += 0.001f;

            window.flutter->Draw(flutter_bounds);

            ui_label(("Pixel ratio: " + std::to_string(window.flutter->GetPixelRatio())).c_str());
            ui_sameline();
            float ratio = window.flutter->GetPixelRatio();
            ui_hslider("ratioslider", ratio, 0.5f, 2.5f);
            window.flutter->SetPixelRatio(ratio);

            ui_window_end();

            // Pop/restore the old UI settings.
            sk::ui_settings(prev_settings);
        }
	});

    // FlutterSurface is in charge of freeing things.
    for (int i = 0; i < flutterWindows.size(); i++)
    {
        delete flutterWindows[i].flutter;
    }

    return 0;
}