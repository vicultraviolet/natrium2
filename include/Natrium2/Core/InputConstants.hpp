#ifndef NA2_INPUT_CONSTANTS_HPP
#define NA2_INPUT_CONSTANTS_HPP

#include "Natrium2/Core.hpp"

namespace Na2
{
	enum class InputAction : u8 {
		None = 0,
		Release, Press, Repeat
	};

	enum class Key : u8 {
		None = 0,

		_0, _1, _2, _3, _4, _5, _6, _7, _8, _9,
		A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T, U, V, W, X, Y, Z,

		Apostrophe,
		Comma,
		Minus,
		Period,
		Slash,
		Semicolon,
		Equal,
		LeftBracket, RightBracket,
		Backslash,
		GraveAccent,

		World1, World2,

		LeftShift, RightShift,
		LeftControl, RightControl,
		LeftAlt, RightAlt,
		LeftSuper, RightSuper,
		CapsLock,

		Escape,
		Space,
		Tab,
		Enter,
		Backspace,
		Insert, Delete,

		Left, Right, Up, Down,
		PageUp, PageDown,
		Home, End,

		ScrollLock,
		NumLock,
		PrintScreen,
		Pause,
		Menu,

		F1,  F2,  F3,  F4,  F5,  F6,  F7,  F8,  F9,  F10, F11, F12,
		F13, F14, F15, F16, F17, F18, F19, F20, F21, F22, F23, F24, F25,

		KP0, KP1, KP2, KP3, KP4, KP5, KP6, KP7, KP8, KP9,
		KPDecimal,
		KPDivide,
		KPMultiply,
		KPSubtract,
		KPAdd,
		KPEnter,
		KPEqual,

		Last = KPEqual
	};

	enum class KeyModFlag : u8 {
		None = 0,
		Shift    = NA2_BIT(0),
		Control  = NA2_BIT(1),
		Alt      = NA2_BIT(2),
		Super    = NA2_BIT(3),
		CapsLock = NA2_BIT(4),
		NumLock  = NA2_BIT(5),
	};

	enum class MouseButton : u8 {
		None = 0,
		Button1, Button2, Button3, Button4, Button5, Button6, Button7, Button8,

		Last = Button8,

		Left = Button1,
		Right = Button2,
		Middle = Button3,
	};

	using JoystickID = u8;

	enum class GamepadButton : u8 {
		None = 0,
		A, B, X, Y,
		Left, Right, Up, Down,
		LeftBumper, RightBumper,
		LeftThumb, RightThumb,
		Back, Start,
		Guide,

		Last = Guide,

		Cross = A,
		Circle = B,
		Square = X,
		Triangle = Y
	};

	enum class GamepadAxis : u8 {
		None = 0,
		LeftX, LeftY,
		RightX, RightY,
		LeftTrigger, RightTrigger,

		Last = RightTrigger
	};

	enum class HatFlag : u8 {
		None  = 0,
		Left  = NA2_BIT(0),
		Right = NA2_BIT(1),
		Up    = NA2_BIT(2),
		Down  = NA2_BIT(3),

		Centered = None,
		UpLeft = Up | Left,
		UpRight = Up | Right,
		DownLeft = Down | Left,
		DownRight = Down | Right
	};

	inline KeyModFlag operator|(KeyModFlag lhs, KeyModFlag rhs) { return (KeyModFlag)((u8)lhs | (u8)rhs); }
	inline KeyModFlag operator&(KeyModFlag lhs, KeyModFlag rhs) { return (KeyModFlag)((u8)lhs & (u8)rhs); }
	inline KeyModFlag operator^(KeyModFlag lhs, KeyModFlag rhs) { return (KeyModFlag)((u8)lhs ^ (u8)rhs); }
	inline KeyModFlag operator~(KeyModFlag state) { return (KeyModFlag)(~(u8)state); }

	inline KeyModFlag& operator|=(KeyModFlag& lhs, KeyModFlag rhs) { lhs = lhs | rhs; return lhs; }
	inline KeyModFlag& operator&=(KeyModFlag& lhs, KeyModFlag rhs) { lhs = lhs & rhs; return lhs; }
	inline KeyModFlag& operator^=(KeyModFlag& lhs, KeyModFlag rhs) { lhs = lhs ^ rhs; return lhs; }

	inline bool operator==(KeyModFlag lhs, KeyModFlag rhs) { return (u8)lhs == (u8)rhs; }
	inline bool operator!=(KeyModFlag lhs, KeyModFlag rhs) { return (u8)lhs != (u8)rhs; }

	inline HatFlag operator|(HatFlag lhs, HatFlag rhs) { return (HatFlag)((u8)lhs | (u8)rhs); }
	inline HatFlag operator&(HatFlag lhs, HatFlag rhs) { return (HatFlag)((u8)lhs & (u8)rhs); }
	inline HatFlag operator^(HatFlag lhs, HatFlag rhs) { return (HatFlag)((u8)lhs ^ (u8)rhs); }
	inline HatFlag operator~(HatFlag state) { return (HatFlag)(~(u8)state); }

	inline HatFlag& operator|=(HatFlag& lhs, HatFlag rhs) { lhs = lhs | rhs; return lhs; }
	inline HatFlag& operator&=(HatFlag& lhs, HatFlag rhs) { lhs = lhs & rhs; return lhs; }
	inline HatFlag& operator^=(HatFlag& lhs, HatFlag rhs) { lhs = lhs ^ rhs; return lhs; }

	inline bool operator==(HatFlag lhs, HatFlag rhs) { return (u8)lhs == (u8)rhs; }
	inline bool operator!=(HatFlag lhs, HatFlag rhs) { return (u8)lhs != (u8)rhs; }
} // namespace Na2

#endif // NA2_INPUT_CONSTANTS_HPP