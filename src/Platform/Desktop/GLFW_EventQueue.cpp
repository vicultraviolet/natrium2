#include "Pch.hpp"
#include "Natrium2/Platform/Desktop/GLFW_EventQueue.hpp"

#ifdef NA2_USE_GLFW

#include "Natrium2/Platform/Desktop/GLFW_InputConstants.hpp"
#include <GLFW/glfw3.h>

namespace Na2::Platform::Desktop
{
	bool IsGamepadPresent(JoystickID _jid)
	{
		int jid = (int)_jid - 1;
		return glfwJoystickPresent(jid) && glfwJoystickIsGamepad(jid);
	}

	ArrayList<GLFWgamepadstate> previousGamepadStates(16);

	void EventQueue::destroy(void)
	{
		if (EventQueue::s_Instance == this)
			EventQueue::s_Instance = nullptr;

		m_Events.destroy();
	}

	EventQueue::EventQueue(EventQueue&& other) noexcept
	: m_Events(std::move(other.m_Events))
	{
		if (EventQueue::s_Instance == &other)
			EventQueue::s_Instance = this;
	}

	EventQueue& EventQueue::operator=(EventQueue&& other) noexcept
	{
		if (this == &other)
			return *this;

		m_Events = std::move(other.m_Events);

		if (EventQueue::s_Instance == &other)
			EventQueue::s_Instance = this;

		return *this;
	}

	EventQueue& EventQueue::poll(void)
	{
		m_Events.clear();
		glfwPollEvents();

		for (int jid = GLFW_JOYSTICK_1; jid <= GLFW_JOYSTICK_16; jid++)
		{
			if (glfwJoystickPresent(jid) && glfwJoystickIsGamepad(jid))
			{
				GLFWgamepadstate current_state;
				if (glfwGetGamepadState(jid, &current_state))
				{
					auto& previous_state = previousGamepadStates[jid];

					for (int button = 0; button <= GLFW_GAMEPAD_BUTTON_LAST; button++)
					{
						if (current_state.buttons[button] == previous_state.buttons[button])
							continue;

						if (current_state.buttons[button] == GLFW_PRESS)
							m_Events.emplace(Event{.gamepad_button_pressed = {
								EventType::GamepadButtonPressed,
								false,
								nullptr,
								(u8)(jid + 1),
								GamepadButtonFromGLFW(button)
							}});
						else
							m_Events.emplace(Event{.gamepad_button_released = {
								EventType::GamepadButtonReleased,
								false,
								nullptr,
								(u8)(jid + 1),
								GamepadButtonFromGLFW(button)
							}});
					}

					for (int axis = 0; axis <= GLFW_GAMEPAD_AXIS_LAST; ++axis)
					{
						if (current_state.axes[axis] == previous_state.axes[axis])
							continue;

						m_Events.emplace(Event{.gamepad_axis_moved = {
							EventType::GamepadAxisMoved,
							false,
							nullptr,
							(u8)(jid + 1),
							GamepadAxisFromGLFW(axis),
							current_state.axes[axis]
						}});
					}

					previous_state = current_state;
				}
			}
		}

		return *this;
	}
} // namespace Na2::Platform::Desktop

#endif // NA2_USE_GLFW
