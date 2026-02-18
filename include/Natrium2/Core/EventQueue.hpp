#ifndef NA2_EVENT_QUEUE_HPP
#define NA2_EVENT_QUEUE_HPP

#ifdef NA2_USE_GLFW
	#include "Natrium2/Platform/Desktop/GLFW_EventQueue.hpp"
	namespace Na2 { using EventQueue = Platform::Desktop::EventQueue; } 
#endif // NA2_USE_GLFW

#endif // NA2_EVENT_QUEUE_HPP