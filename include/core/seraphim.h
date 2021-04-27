#ifndef SERAPHIM_H
#define SERAPHIM_H

#include "core/debug.h"

#include <condition_variable>
#include <set>
#include <string>
#include <vector>

#include "physics/physics.h"
#include "render/renderer.h"
#include "ui/window.h"
#include "device.h"

#define SERAPHIM_MAX_SUBSTANCES 100
#define SERAPHIM_MAX_SDFS 100
#define SERAPHIM_MAX_TITLE_LENGTH 256

namespace srph {
	struct seraphim_t {
		char title[SERAPHIM_MAX_TITLE_LENGTH];

		// debug fields
#if SERAPHIM_DEBUG
		VkDebugReportCallbackEXT callback;

		bool check_validation_layers();

		bool setup_debug_callback();

#endif
		// initialisation functions
		void create_instance();

		 std::vector < const char *>get_required_extensions();

		 std::unique_ptr < device_t > device;
		 std::unique_ptr < renderer_t > renderer;
		 std::unique_ptr < window_t > window;
		srph_physics physics;

		VkInstance instance;
		VkSurfaceKHR surface;

		u32vec2_t work_group_count;
		u32vec2_t work_group_size;

		 std::shared_ptr < camera_t > test_camera;

		size_t num_substances;
		substance_t substances[SERAPHIM_MAX_SUBSTANCES];

		size_t num_sdfs;
		sdf_t sdfs[SERAPHIM_MAX_SDFS];

		bool fps_monitor_quit;

		void monitor_fps();

		 std::thread fps_monitor_thread;
		 std::condition_variable fps_cv;

		 seraphim_t();

		void run();
	};
} sdf_t *seraphim_create_sdf(srph::seraphim_t * seraphim, sdf_func_t f,
			     void *data);
substance_t *srph_create_substance(srph::seraphim_t * srph, form_t * form,
				   matter_t * matter);

void srph_cleanup(srph::seraphim_t * engine);

#endif
