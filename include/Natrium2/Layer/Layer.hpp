#ifndef NA2_LAYER_HPP
#define NA2_LAYER_HPP

#include "Natrium2/Core/Event.hpp"

namespace Na2
{
	enum class LayerStateFlag : u8 {
		None = 0,

		Enabled   = NA2_BIT(0),
		Updatable = NA2_BIT(1),
		Visible   = NA2_BIT(2),

		All = Enabled | Updatable | Visible
	};

	class Layer {
	public:
		Layer(i64 priority = 0) : m_Priority(priority) {}
		virtual ~Layer(void) = default;

		virtual void on_attach(void) {}
		virtual void on_detach(void) {}

		virtual void on_event(Event& e) {}
		virtual void on_update(f64 dt) {}
		virtual void on_draw(void) {}
		virtual void on_imgui_draw(void) {}

		[[nodiscard]] bool enabled(void) const;
		[[nodiscard]] bool updatable(void) const;
		[[nodiscard]] bool visible(void) const;

		virtual void set_enabled(bool enabled);
		virtual void set_updatable(bool updatable);
		virtual void set_visible(bool visible);

		[[nodiscard]] LayerStateFlag state(void) const { return m_State; }
		void set_state(LayerStateFlag state) { m_State = state; }

		[[nodiscard]] inline i64 priority(void) const { return m_Priority; }
	private:
		friend class LayerManager;

		i64 m_Priority;
		LayerStateFlag m_State = LayerStateFlag::All;
	};

	template<typename T>
	concept DerivedLayer = std::is_base_of<Layer, T>::value && !std::is_same<Layer, T>::value;

	template<DerivedLayer T, typename... t_Args>
	inline Ref<T> MakeLayer(t_Args&&... __args) { return MakeRef<T>(std::forward<t_Args>(__args)...); }

	inline LayerStateFlag operator|(LayerStateFlag lhs, LayerStateFlag rhs) { return (LayerStateFlag)((u8)lhs | (u8)rhs); }
	inline LayerStateFlag operator&(LayerStateFlag lhs, LayerStateFlag rhs) { return (LayerStateFlag)((u8)lhs & (u8)rhs); }
	inline LayerStateFlag operator^(LayerStateFlag lhs, LayerStateFlag rhs) { return (LayerStateFlag)((u8)lhs ^ (u8)rhs); }
	inline LayerStateFlag operator~(LayerStateFlag state) { return (LayerStateFlag)(~(u8)state); }

	inline LayerStateFlag& operator|=(LayerStateFlag& lhs, LayerStateFlag rhs) { lhs = lhs | rhs; return lhs; }
	inline LayerStateFlag& operator&=(LayerStateFlag& lhs, LayerStateFlag rhs) { lhs = lhs & rhs; return lhs; }
	inline LayerStateFlag& operator^=(LayerStateFlag& lhs, LayerStateFlag rhs) { lhs = lhs ^ rhs; return lhs; }

	inline bool operator==(LayerStateFlag lhs, LayerStateFlag rhs) { return (u8)lhs == (u8)rhs; }
	inline bool operator!=(LayerStateFlag lhs, LayerStateFlag rhs) { return (u8)lhs != (u8)rhs; }
} // namespace Na2

#endif // NA2_LAYER_HPP