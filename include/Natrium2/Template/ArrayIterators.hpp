#if !defined(NA2_ARRAY_ITERATOR_HPP)
#define NA2_ARRAY_ITERATOR_HPP

#include "Natrium2/Core.hpp"

namespace Na2
{
	template<typename t_Array>
	class Array_Iterator {
		using T = t_Array::T_t;
		using This = Array_Iterator<t_Array>;
	public:
		inline This& operator++(void) { m_Ptr++; return *this; }
		inline This  operator++(int) { m_Ptr++; return This(m_Ptr - 1); }

		inline This& operator--(void) { m_Ptr--; return *this; }
		inline This  operator--(int) { m_Ptr--; return This(m_Ptr + 1); }

		inline This& operator+=(u64 amount) { m_Ptr += amount; return *this; }
		inline This& operator-=(u64 amount) { m_Ptr -= amount; return *this; }

		[[nodiscard]] inline This operator+(u64 amount) const { return This(m_Ptr + amount); }
		[[nodiscard]] inline This operator-(u64 amount) const { return This(m_Ptr - amount); }

		[[nodiscard]] inline T* operator->(void) const { return m_Ptr; }
		[[nodiscard]] inline T& operator*(void) const { return *m_Ptr; }
		[[nodiscard]] inline T* get(void) { return m_Ptr; }
		[[nodiscard]] inline t_Array::reverse_iterator reverse(void) { return m_Ptr; }

		[[nodiscard]] inline bool operator==(const This& other)  const { return m_Ptr == other.m_Ptr; }
		[[nodiscard]] inline auto operator<=>(const This& other) const { return m_Ptr <=> other.m_Ptr; }

		Array_Iterator(T* ptr = nullptr)
		: m_Ptr(ptr) {}
	private:
		T* m_Ptr;
	};

	template<typename t_Array>
	class Array_ReverseIterator {
		using T = t_Array::T_t;
		using This = Array_ReverseIterator<t_Array>;
	public:
		inline This& operator++(void) { m_Ptr--; return *this; }
		inline This  operator++(int) { m_Ptr--; return This(m_Ptr + 1); }

		inline This& operator--(void) { m_Ptr++; return *this; }
		inline This  operator--(int) { m_Ptr++; return This(m_Ptr - 1); }

		inline This& operator+=(u64 amount) { m_Ptr -= amount; return *this; }
		inline This& operator-=(u64 amount) { m_Ptr += amount; return *this; }

		[[nodiscard]] inline This operator+(u64 amount) const { return This(m_Ptr - amount); }
		[[nodiscard]] inline This operator-(u64 amount) const { return This(m_Ptr + amount); }

		[[nodiscard]] inline T* operator->(void) const { return m_Ptr; }
		[[nodiscard]] inline T& operator*(void) const { return *m_Ptr; }
		[[nodiscard]] inline T* get(void) { return m_Ptr; }
		[[nodiscard]] inline t_Array::iterator reverse(void) { return m_Ptr; }

		[[nodiscard]] inline bool operator==(const This& other)  const { return m_Ptr == other.m_Ptr; }
		[[nodiscard]] inline auto operator<=>(const This& other) const { return other.m_Ptr <=> m_Ptr; }

		Array_ReverseIterator(T* ptr = nullptr)
		: m_Ptr(ptr) {}
	private:
		T* m_Ptr;
	};

	template<typename t_Array>
	class Array_ConstIterator {
		using T = t_Array::T_t;
		using This = Array_ConstIterator<t_Array>;
	public:
		inline This& operator++(void) { m_Ptr++; return *this; }
		inline This  operator++(int) { m_Ptr++; return This(m_Ptr - 1); }

		inline This& operator--(void) { m_Ptr--; return *this; }
		inline This  operator--(int) { m_Ptr--; return This(m_Ptr + 1); }

		inline This& operator+=(u64 amount) { m_Ptr += amount; return *this; }
		inline This& operator-=(u64 amount) { m_Ptr -= amount; return *this; }

		[[nodiscard]] inline This operator+(u64 amount) const { return This(m_Ptr + amount); }
		[[nodiscard]] inline This operator-(u64 amount) const { return This(m_Ptr - amount); }

		[[nodiscard]] inline const T* operator->(void) const { return m_Ptr; }
		[[nodiscard]] inline const T& operator*(void) const { return *m_Ptr; }
		[[nodiscard]] inline const T* get(void) { return m_Ptr; }
		[[nodiscard]] inline t_Array::const_reverse_iterator reverse(void) { return m_Ptr; }

		[[nodiscard]] inline bool operator==(const This& other) const { return m_Ptr == other.m_Ptr; }
		[[nodiscard]] inline auto operator<=>(const This& other) const { return m_Ptr <=> other.m_Ptr; }

		Array_ConstIterator(const T* const ptr = nullptr)
		: m_Ptr(ptr) {}
	private:
		const T* m_Ptr;
	};

	template<typename t_Array>
	class Array_ConstReverseIterator {
		using T = t_Array::T_t;
		using This = Array_ConstReverseIterator<t_Array>;
	public:
		inline This& operator++(void) { m_Ptr--; return *this; }
		inline This  operator++(int) { m_Ptr--; return This(m_Ptr + 1); }

		inline This& operator--(void) { m_Ptr++; return *this; }
		inline This  operator--(int) { m_Ptr++; return This(m_Ptr - 1); }

		inline This& operator+=(u64 amount) { m_Ptr -= amount; return *this; }
		inline This& operator-=(u64 amount) { m_Ptr += amount; return *this; }

		[[nodiscard]] inline This operator+(u64 amount) const { return This(m_Ptr - amount); }
		[[nodiscard]] inline This operator-(u64 amount) const { return This(m_Ptr + amount); }

		[[nodiscard]] inline const T* operator->(void) const { return m_Ptr; }
		[[nodiscard]] inline const T& operator*(void)  const { return *m_Ptr; }
		[[nodiscard]] inline const T* get(void) { return m_Ptr; }
		[[nodiscard]] inline t_Array::const_iterator reverse(void) { return m_Ptr; }

		[[nodiscard]] inline bool operator==(const This& other)  const { return m_Ptr == other.m_Ptr; }
		[[nodiscard]] inline auto operator<=>(const This& other) const { return other.m_Ptr <=> m_Ptr; }

		Array_ConstReverseIterator(const T* ptr = nullptr)
		: m_Ptr(ptr) {}
	private:
		const T* m_Ptr;
	};
} // namespace Na2

#endif // NA2_ARRAY_ITERATOR_HPP