#ifndef NA2_GLFW_INPUT_CONSTANTS_HPP
#define NA2_GLFW_INPUT_CONSTANTS_HPP

#include "Natrium2/Core/InputConstants.hpp"

namespace Na2::Platform::Desktop
{
	int InputActionToGLFW(InputAction action);
	InputAction InputActionFromGLFW(int action);

	int KeyToGLFW(Key key);
	Key KeyFromGLFW(int key);

	int KeyModToGLFW(KeyModFlag mod);
	KeyModFlag KeyModFromGLFW(int mod);

	int MouseButtonToGLFW(MouseButton button);
	MouseButton MouseButtonFromGLFW(int button);

	int GamepadButtonToGLFW(GamepadButton button);
	GamepadButton GamepadButtonFromGLFW(int button);

	int GamepadAxisToGLFW(GamepadAxis axis);
	GamepadAxis GamepadAxisFromGLFW(int axis);

	int HatToGLFW(HatFlag hat);
	HatFlag HatFromGLFW(int hat);
} // Na2::Platform::Desktop

#endif // NA2_GLFW_INPUT_CONSTANTS_HPP