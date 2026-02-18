#ifndef NA2_WINDOW_HPP
#define NA2_WINDOW_HPP

#ifdef NA2_USE_GLFW

	#include "Natrium2/Platform/Desktop/GLFW_Window.hpp"

namespace Na2 { using Window = Platform::Desktop::GLFW_Window; }

#endif // NA2_USE_GLFW

#endif // "NA2_WINDOW_HPP"