#ifndef NA2_GLFW_EVENT_QUEUE_HPP
#define NA2_GLFW_EVENT_QUEUE_HPP

#include "Natrium2/Core/Event.hpp"

namespace Na2::Platform::Desktop
{
	class EventQueue {
	public:
		using iterator = ArrayList<Event>::iterator;
		using const_iterator = ArrayList<Event>::const_iterator;

		EventQueue(void) = default;
		~EventQueue(void) { this->destroy(); }

		void destroy(void);

		EventQueue(const EventQueue& other) = delete;
		EventQueue& operator=(const EventQueue& other) = delete;

		EventQueue(EventQueue&& other) noexcept;
		EventQueue& operator=(EventQueue&& other) noexcept;

		inline void bind(void) { EventQueue::s_Instance = this; }
		inline void unbind(void) { EventQueue::s_Instance = nullptr; }

		[[nodiscard]] static inline bool Exists(void) { return EventQueue::s_Instance; }
		[[nodiscard]] static inline View<EventQueue> Get(void) { return EventQueue::s_Instance; }

		EventQueue& poll(void);

		inline void add(Event& e) { m_Events.emplace(e); }
		inline void add(Event&& e) { m_Events.emplace(std::move(e)); }

		inline void clear(void) { m_Events.clear(); }

		[[nodiscard]] inline iterator at(u64 index) { return m_Events.at(index); }
		[[nodiscard]] inline const_iterator at(u64 index) const { return m_Events.at(index); }

		[[nodiscard]] inline Event& operator[](u64 index) { return m_Events[index]; }
		[[nodiscard]] inline const Event& operator[](u64 index) const { return m_Events[index]; }

		[[nodiscard]] inline iterator begin(void) { return m_Events.begin(); }
		[[nodiscard]] inline iterator end(void) { return m_Events.end(); }

		[[nodiscard]] inline const_iterator begin(void) const { return m_Events.begin(); }
		[[nodiscard]] inline const_iterator end(void) const { return m_Events.end(); }

		[[nodiscard]] inline const_iterator cbegin(void) const { return m_Events.cbegin(); }
		[[nodiscard]] inline const_iterator cend(void) const { return m_Events.cend(); }
	private:
		ArrayList<Event> m_Events{ 64 };

		static inline View<EventQueue> s_Instance = nullptr;
	};
} // namespace Na2::Platform::Desktop

#endif // GLFW_EVENT_QUEUE_HPP