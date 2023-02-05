#include <stereokit.h>
#include <stereokit_ui.h>
#include <flutter_embedder.h>
#include <flutter_surface.hpp>
#include <iostream>
#include <fstream>
#include <chrono>
#include <filesystem>
using namespace sk;

double dpi = 2.0;
pose_t windowPose = pose_identity;

sk::tex_t flutter_texture;

mesh_t     quad_mesh;
mesh_t     cube_mesh;
material_t cube_mat;
pose_t     cube_pose = {{0,0,-0.5f}, quat_identity};

// The Flutter surface we can reference inside StereoKit.
FlutterSurface* flutterSurface;

static_assert(FLUTTER_ENGINE_VERSION == 1,
              "This Flutter Embedder was authored against the stable Flutter "
              "API at version 1. There has been a serious breakage in the "
              "API. Please read the ChangeLog and take appropriate action "
              "before updating this assertion");

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
	if (!sk_init(settings))
		return 1;

    // Grab the path to the flutter project and icudtl
    std::string flutter_project_path = argv[1];
    std::string flutter_icudtl_path = argv[2];

    std::cout << "Booting Flutter app at path: " << flutter_project_path << std::endl;

    // FlutterSurface wraps the Flutter engine/embedder/etc,
    // as well as performs input handling niceties for us.
    flutterSurface = new FlutterSurface(
        "MyFlutterSurface",
        512, 512,
        1.0,
        flutter_project_path,
        flutter_icudtl_path
    );

    sk_run([]() {
        ui_window_begin("Flutter", windowPose, vec2_one * 0.3f, sk::ui_win_body, sk::ui_move_face_user);
        sk::bounds_t flutter_bounds = ui_layout_reserve(vec2_one * ui_layout_remaining().x);
        flutter_bounds.center.z -= 0.001f;
        flutter_bounds.dimensions.z += 0.001f;

        flutterSurface->Draw(flutter_bounds);

        ui_window_end();
	});

    // FlutterSurface is in charge of freeing things.
    delete flutterSurface;

    return 0;
}