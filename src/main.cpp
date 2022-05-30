#include <stereokit.h>
#include <stereokit_ui.h>
#include <flutter_embedder.h>
#include <iostream>
#include <fstream>
using namespace sk;

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

void* render_surface;

int test = 0;

FlutterEngine engine = nullptr;

bool RunFlutter(const std::string& project_path,
                const std::string& icudtl_path) {

    // Build our Flutter renderer config.
    FlutterRendererConfig config = {};

    // render_surface = malloc(1024*1024*24);

    config.type = kSoftware;
    config.software.struct_size = sizeof(FlutterSoftwareRendererConfig);
    config.software.surface_present_callback = [](void* userdata,
                                               const void* sw_rendered_data,
                                               size_t width,
                                               size_t height) -> bool {
        
        std::cout << "Blitting sw-rendered frame with dimensions " << width << ", " << height << std::endl;
        // std::memcpy(render_surface, sw_rendered_data, width * height);
        test++;
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
        .icu_data_path =
            icudtl_path.c_str(),  // Find this in your bin/cache directory.
        // .compositor = &compositor
    };

    // FlutterEngine engine = nullptr;
    FlutterEngineResult result =
        FlutterEngineRun(FLUTTER_ENGINE_VERSION, &config,  // renderer
                        &args, nullptr, &engine);
    if (result != kSuccess || engine == nullptr) {
        std::cout << "Could not run the Flutter Engine." << std::endl;
        return false;
    }

    return true;
}

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

    RunFlutter(flutter_project_path, flutter_icudtl_path);

    cube_mesh = mesh_gen_rounded_cube(vec3_one * 0.1f, 0.02f, 4);
    cube_mat  = material_find        (default_id_material_ui);

    sk_run([]() {
        ui_handle_begin("Cube", cube_pose, mesh_get_bounds(cube_mesh), false);
        render_add_mesh(cube_mesh, cube_mat, matrix_identity);
        ui_handle_end();
	});

    return 0;
}