#ifndef NA2_EVENT_HPP
#define NA2_EVENT_HPP

#include "Natrium2/Core.hpp"
#include "Natrium2/Core/InputConstants.hpp"

#define NA2_EVENT_BASE(x) struct {\
						Na2::EventType type = Na2::EventType::x;\
						bool handled = false;\
						View<Na2::Window> window;\
					 };

namespace Na2
{
#ifdef NA2_USE_GLFW
	namespace Platform::Desktop { class GLFW_Window; }
	using Window = Platform::Desktop::GLFW_Window;
#endif // NA2_USE_GLFW

	enum class EventType : u8 {
		None = 0,
		KeyPressed, KeyReleased,
		CharInput,
		WindowResized, WindowClosed, WindowFocused, WindowLostFocus, WindowMinimized, WindowRestored,
		MouseMoved, MouseScrolled, MouseButtonPressed, MouseButtonReleased,
		GamepadConnected, GamepadDisconnected,
		GamepadButtonPressed, GamepadButtonReleased, GamepadAxisMoved
	};

	struct Event_KeyPressed {
		NA2_EVENT_BASE(KeyPressed)
		Key key;
		KeyModFlag mod;
		bool repeat;
		u32 padding = 0;
	};

	struct Event_KeyReleased {
		NA2_EVENT_BASE(KeyReleased)
		Key key;
		KeyModFlag mod;
		u32 padding = 0;
	};

	struct Event_CharInput {
		NA2_EVENT_BASE(CharInput)
		char32_t codepoint = 0;
	};

	struct Event_WindowResized {
		NA2_EVENT_BASE(WindowResized)
		u32 width, height;
	};

	struct Event_WindowClosed {
		NA2_EVENT_BASE(WindowClosed)
		u32 padding1 = 0, padding2 = 0;
	};

	struct Event_WindowFocused {
		NA2_EVENT_BASE(WindowFocused)
		u32 padding1 = 0, padding2 = 0;
	};
	struct Event_WindowLostFocus {
		NA2_EVENT_BASE(WindowLostFocus)
		u32 padding1 = 0, padding2 = 0;
	};

	struct Event_WindowMinimized {
		NA2_EVENT_BASE(WindowMinimized)
		u32 padding1 = 0, padding2 = 0;
	};
	struct Event_WindowRestored {
		NA2_EVENT_BASE(WindowRestored)
		u32 padding1 = 0, padding2 = 0;
	};

	struct Event_MouseMoved {
		NA2_EVENT_BASE(MouseMoved)
		f32 x, y;
	};

	struct Event_MouseScrolled {
		NA2_EVENT_BASE(MouseScrolled)
		f32 x_offset, y_offset;
	};

	struct Event_MouseButtonPressed {
		NA2_EVENT_BASE(MouseButtonPressed)
		MouseButton button;
		u32 padding = 0;
	};
	struct Event_MouseButtonReleased {
		NA2_EVENT_BASE(MouseButtonReleased)
		MouseButton button;
		u32 padding = 0;
	};

	struct Event_GamepadConnected {
		NA2_EVENT_BASE(GamepadConnected)
		JoystickID joystick_id;
		u32 padding = 0;
	};

	struct Event_GamepadDisconnected {
		NA2_EVENT_BASE(GamepadDisconnected)
		JoystickID joystick_id;
		u32 padding = 0;
	};

	struct Event_GamepadButtonPressed {
		NA2_EVENT_BASE(GamepadButtonPressed)
		JoystickID joystick_id;
		GamepadButton button;
		u32 padding = 0;
	};

	struct Event_GamepadButtonReleased {
		NA2_EVENT_BASE(GamepadButtonReleased)
		JoystickID joystick_id;
		GamepadButton button;
		u32 padding = 0;
	};

	struct Event_GamepadAxisMoved {
		NA2_EVENT_BASE(GamepadAxisMoved)
		JoystickID joystick_id;
		GamepadAxis axis;
		f32 value;
	};

	union Event {
		struct {
			EventType type;
			bool handled;
			Window* window;
			u32 padding1, padding2;
		};

		Event_KeyPressed key_pressed;
		Event_KeyReleased key_released;

		Event_CharInput char_input;

		Event_WindowResized window_resized;
		Event_WindowClosed window_closed;

		Event_WindowFocused window_focused;
		Event_WindowLostFocus window_lost_focus;

		Event_WindowMinimized window_minimized;
		Event_WindowRestored window_restored;

		Event_MouseMoved mouse_moved;
		Event_MouseScrolled mouse_scrolled;

		Event_MouseButtonPressed mouse_button_pressed;
		Event_MouseButtonReleased mouse_button_released;

		Event_GamepadConnected gamepad_connected;
		Event_GamepadDisconnected gamepad_disconnected;

		Event_GamepadButtonPressed gamepad_button_pressed;
		Event_GamepadButtonReleased gamepad_button_released;

		Event_GamepadAxisMoved gamepad_axis_moved;
	};
} // namespace Na2

#endif // NA2_EVENT_HPP