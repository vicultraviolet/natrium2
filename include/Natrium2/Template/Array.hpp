#if !defined(NA2_ARRAY_HPP)
#define NA2_ARRAY_HPP

#include "Natrium2/Template/ArrayIterators.hpp"

namespace Na2
{
	template<typename T, u64 t_Capacity>
	class Array {
	public:
		using iterator = Array_Iterator<Array>;
		using reverse_iterator = Array_ReverseIterator<Array>;
		using const_iterator = Array_ConstIterator<Array>;
		using const_reverse_iterator = Array_ConstReverseIterator<Array>;
		using T_t = T;
	public:
		Array(void) = default;
		~Array(void) { this->clear(); }

		void clear(void)
		{
			for (u64 i = 0; i < m_Size; i++)
				std::destroy_at(this->ptr() + i);

			m_Size = 0;
		}

		template<typename... t_Args>
		Array(u64 size, t_Args&&... __args)
		: m_Size(size)
		{
			for (u64 i = 0; i < m_Size; i++)
				std::construct_at(this->ptr() + i, std::forward<t_Args>(__args)...);
		}

		template<Dereferenceable t_Iterator>
		Array(const t_Iterator& begin, const t_Iterator& end)
		: m_Size(std::distance(begin, end))
		{
			u64 i = 0;
			for (t_Iterator it = begin; it != end; it++)
				std::construct_at(this->ptr() + i++, *it);
		}

		template<Dereferenceable t_Iterator>
		void set(const t_Iterator& begin, const t_Iterator& end)
		{
			for (u64 i = 0; i < m_Size; i++)
				std::destroy_at(this->ptr() + i);

			m_Size = std::distance(begin, end);

			u64 i = 0;
			for (t_Iterator it = begin; it != end; it++)
				std::construct_at(this->ptr() + i++, *it);
		}

		Array(const T* buffer, u64 size)
		: m_Size(size)
		{
			for (u64 i = 0; i < m_Size; i++)
				std::construct_at(this->ptr() + i, buffer[i]);
		}

		Array(const std::initializer_list<T>& list)
		: Array(list.begin(), list.end())
		{}

		Array& operator=(const std::initializer_list<T>& list)
		{
			this->set(list.begin(), list.end());
		}

		Array(const Array& other)
		{
			for (u64 i = 0; i < other.m_Size; i++)
				std::construct_at(this->ptr() + i, other[i]);
		}

		Array& operator=(const Array& other)
		{
			if (this == &other)
				return *this;

			for (u64 i = 0; i < m_Size; i++)
				std::destroy_at(this->ptr() + i);

			m_Size = other.m_Size;
			for (u64 i = 0; i < other.m_Size; i++)
				std::construct_at(this->ptr() + i, other[i]);

			return *this;
		}

		Array(Array&& other) noexcept
		{
			for (u64 i = 0; i < other.m_Size; i++)
				std::construct_at(this->ptr() + i, std::move(other[i]));

			m_Size = std::exchange(other.m_Size, 0);
		}

		Array& operator=(Array&& other) noexcept
		{
			if (this == &other)
				return *this;

			for (u64 i = 0; i < m_Size; i++)
				std::destroy_at(this->ptr() + i);

			for (u64 i = 0; i < other.m_Size; i++)
				std::construct_at(this->ptr() + i, std::move(other[i]));

			m_Size = std::exchange(other.m_Size, 0);

			return *this;
		}

		void set_size(u64 size)
		{
			NA2_ASSERT(t_Capacity >= size, "Failed to resize ArrayList: specified size is bigger than capacity!");
			m_Size = size;
		}

		template<typename... t_Args>
		u64 emplace(t_Args&&... __args)
		{
			std::construct_at(this->ptr() + m_Size, std::forward<t_Args>(__args)...);
			return m_Size++;
		}

		template<typename... t_Args>
		inline u64 emplace_back(t_Args&&... __args) { return this->emplace(std::forward<t_Args>(__args)...); }

		inline void pop(void) { std::destroy_at(this->ptr() + --m_Size); }
		inline void pop_back(void) { return this->pop(); }

		[[nodiscard]] inline iterator at_s(i64 index) { return this->ptr() + index; }
		[[nodiscard]] inline const_iterator at_s(i64 index) const { return this->ptr() + index; }

		[[nodiscard]] inline iterator at(u64 index) { return this->ptr() + index; }
		[[nodiscard]] inline const_iterator at(u64 index) const { return this->ptr() + index; }

		[[nodiscard]] inline iterator begin(void) { return this->at(0); }
		[[nodiscard]] inline const_iterator begin(void) const { return this->at(0); }
		[[nodiscard]] inline const_iterator cbegin(void) const { return this->at(0); }

		[[nodiscard]] inline iterator end(void) { return this->at(m_Size); }
		[[nodiscard]] inline const_iterator end(void) const { return this->at(m_Size); }
		[[nodiscard]] inline const_iterator cend(void) const { return this->at(m_Size); }

		[[nodiscard]] inline reverse_iterator rbegin(void) { return this->at_s(m_Size - 1).reverse(); }
		[[nodiscard]] inline const_reverse_iterator rbegin(void) const { return this->at_s(m_Size - 1).reverse(); }
		[[nodiscard]] inline const_reverse_iterator crbegin(void) const { return this->at_s(m_Size - 1).reverse(); }

		[[nodiscard]] inline reverse_iterator rend(void) { return this->at_s(-1).reverse(); }
		[[nodiscard]] inline const_reverse_iterator rend(void) const { return this->at_s(-1).reverse(); }
		[[nodiscard]] inline const_reverse_iterator crend(void) const { return this->at_s(-1).reverse(); }

		[[nodiscard]] inline T& front(void) { return *this->begin(); }
		[[nodiscard]] inline const T& front(void) const { return *this->begin(); }

		[[nodiscard]] inline T& back(void) { return *this->rbegin(); }
		[[nodiscard]] inline const T& back(void) const { return *this->rbegin(); }

		[[nodiscard]] inline T& operator[](u64 index) { return *this->at(index); }
		[[nodiscard]] inline const T& operator[](u64 index) const { return *this->at(index); }

		[[nodiscard]] inline T* ptr(void) { return (T*)m_Buffer; }
		[[nodiscard]] inline const T* ptr(void) const { return (T*)m_Buffer; }

		[[nodiscard]] inline constexpr u64 capacity(void) const { return t_Capacity; }
		[[nodiscard]] inline u64 size(void) const { return m_Size; }
		[[nodiscard]] inline u64 free_space(void) const { return t_Capacity - m_Size; }

		[[nodiscard]] inline bool empty(void) const { return !m_Size; }
		[[nodiscard]] inline bool full(void) const { return m_Size == t_Capacity; }
	private:
		alignas(T) Byte m_Buffer[t_Capacity * sizeof(T)];
		u64 m_Size = 0;
	};
} // namespace Na2

#endif // NA2_ARRAY_HPP