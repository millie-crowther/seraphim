#ifndef WINDOW_H
#define WINDOW_H

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <memory>

#include "maths/matrix.h"
#include "ui/keyboard.h"
#include "ui/mouse.h"

namespace srph {
	struct window_t {
		// private fields
		u32vec2_t size;
		GLFWwindow *window;
		 std::unique_ptr < keyboard_t > keyboard;
		 std::unique_ptr < mouse_t > mouse;

		// constructors and destructors
		 window_t(u32vec2_t size);
		~window_t();

		void show();

		// accessors
		GLFWwindow *get_window() const;
		u32vec2_t get_size() const;
		bool should_close() const;

		 keyboard_t & get_keyboard() const;
		 mouse_t & get_mouse() const;
	};
} void window_set_title(srph::window_t * window, const char *title);

#endif
