#ifndef NA2_INPUT_HPP
#define NA2_INPUT_HPP

#include "Natrium2/Core/InputConstants.hpp"
#include "Natrium2/Core/Event.hpp"

namespace Na2
{
	class Input {
	public:
		struct MousePos {
			f32 x, y;
		};

		Input(void) { this->reset(); }
		~Input(void) = default;

		void reset(void);
		void on_event(const Event& e);

		[[nodiscard]] inline bool key(Key key) const { return m_Keys.test((u64)key); }
		[[nodiscard]] inline bool mouse_button(MouseButton button) const { return m_MouseButtons.test((u64)button); }

		[[nodiscard]] inline float mouse_x(void) const { return m_MousePos.x; }
		[[nodiscard]] inline float mouse_y(void) const { return m_MousePos.y; }

		[[nodiscard]] inline bool gamepad_connected(JoystickID jid) const { return m_Gamepads.test(jid); }
		[[nodiscard]] inline bool gamepad_button(JoystickID jid, GamepadButton button) const { return m_GamepadButtons.test(this->_GamepadButtonIndex(jid, button)); }
		[[nodiscard]] inline float gamepad_axis(JoystickID jid, GamepadAxis axis) const { return m_GamepadAxes[this->_GamepadAxisIndex(jid, axis)]; }
	private:
		[[nodiscard]] static constexpr inline u64 _GamepadButtonIndex(JoystickID jid, GamepadButton button) { return (u64)jid * ((u64)GamepadButton::Last + 1) + (u64)button; }
		[[nodiscard]] static constexpr inline u64 _GamepadAxisIndex(JoystickID jid, GamepadAxis axis) { return (u64)jid * ((u64)GamepadAxis::Last + 1) + (u64)axis; }
	private:
		std::bitset<(u64)Key::Last + 1> m_Keys;

		std::bitset<(u64)MouseButton::Last + 1> m_MouseButtons;
		MousePos m_MousePos;

		std::bitset<(u64)k_MaxJoysticks + 1> m_Gamepads;
		std::bitset<((u64)GamepadButton::Last + 1) * ((u64)k_MaxJoysticks + 1)> m_GamepadButtons;
		std::array<f32, ((u64)GamepadAxis::Last + 1) * ((u64)k_MaxJoysticks + 1)> m_GamepadAxes;
	};
} // namespace Na2

#endif // NA2_INPUT_HPP