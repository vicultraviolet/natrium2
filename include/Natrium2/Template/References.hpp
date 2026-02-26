#if !defined(NA2_REF_HPP)
#define NA2_REF_HPP

#include "Natrium2/Core.hpp"

namespace Na2
{
	template<typename T>
	class Box {
	public:
		Box(void) = default;
		~Box(void) { this->destroy(); }

		Box(nullptr_t) {};
		Box& operator=(nullptr_t)
		{
			this->destroy();
			return *this;
		}

		Box(T* ptr) : m_Ptr(ptr) {}
		Box& operator=(T* ptr)
		{
			this->destroy();
			m_Ptr = ptr;
			return *this;
		}

		template<typename... t_Args>
		static Box Make(t_Args&&... __args)
		{
			return Box(new T(std::forward<t_Args>(__args)...));
		}

		Box(const Box& other) = delete;
		Box& operator=(const Box& other) = delete;

		Box(Box&& other) noexcept
		: m_Ptr(std::exchange(other.m_Ptr, nullptr))
		{}

		Box& operator=(Box&& other) noexcept
		{
			this->destroy();
			m_Ptr = std::exchange(other.m_Ptr, nullptr);
			return *this;
		}

		void swap(Box& other)
		{
			std::swap(m_Ptr, other.m_Ptr);
		}

		void destroy(void)
		{
			delete m_Ptr;
			m_Ptr = nullptr;
		}

		T* release(void)
		{
			if (!m_Ptr)
				return nullptr;

			T* temp = m_Ptr;
			m_Ptr = nullptr;
			return temp;
		}

		template<typename U, std::enable_if_t<std::is_base_of_v<U, T>, int> = 0>
		[[nodiscard]] inline operator Box<U>(void)&& { return Box<U>(this->release()); }

		[[nodiscard]] inline T* ptr(void) const { return m_Ptr; }
		[[nodiscard]] inline T& operator*(void) const { return *m_Ptr; }
		[[nodiscard]] inline T* operator->(void) const { return m_Ptr; }

		[[nodiscard]] inline auto operator<=>(const Box& other) const { return m_Ptr <=> other.m_Ptr; }
		[[nodiscard]] inline auto operator==(const Box& other) const { return m_Ptr == other.m_Ptr; }

		[[nodiscard]] inline operator bool(void) const { return m_Ptr; }
	private:
		template<typename To, typename From>
		friend Box<To> static_ref_cast(Box<From>&& from);

		template<typename To, typename From>
		friend Box<To> dynamic_ref_cast(Box<From>&& from);

		T* m_Ptr = nullptr;
	};

	template<typename To, typename From>
	[[nodiscard]] Box<To> static_ref_cast(Box<From>&& from)
	{
		using FromPtr = decltype(from.ptr());
		To* casted = (To*)const_cast<std::remove_const_t<std::remove_pointer_t<FromPtr>>*>(from.ptr());
		from.release();
		return Box<To>(casted);
	}

	template<typename To, typename From>
	[[nodiscard]] Box<To> dynamic_ref_cast(Box<From>&& from)
	{
		using FromPtr = decltype(from.ptr());
		To* casted = dynamic_cast<To*>(const_cast<std::remove_const_t<std::remove_pointer_t<FromPtr>>*>(from.ptr()));
		if (!casted)
			return nullptr;
		from.release();
		return Box<To>(casted);
	}

	template<typename T>
	struct RcControlBlock {
		T* ptr = nullptr;
		std::atomic<u32> strong_count = 0;
		std::atomic<u32> weak_count = 0;

		RcControlBlock(void) = default;
		~RcControlBlock(void) { delete ptr; }

		RcControlBlock(T* ptr) : ptr(ptr) {}

		template<typename... t_Args>
		static RcControlBlock* Make(t_Args&&... __args)
		{
			return new RcControlBlock(new T(std::forward<t_Args>(__args)...));
		}

		void inc_strong_count(void) { this->strong_count.fetch_add(1, std::memory_order_relaxed); }
		void dec_strong_count(void) { this->strong_count.fetch_sub(1, std::memory_order_relaxed); }

		void inc_weak_count(void) { this->weak_count.fetch_add(1, std::memory_order_relaxed); }
		void dec_weak_count(void) { this->weak_count.fetch_sub(1, std::memory_order_relaxed); }
	};

	template<typename T>
	class Weak;

	template<typename T>
	class NonIntrusiveRc {
	public:
		using ControlBlock = RcControlBlock<T>;

		NonIntrusiveRc(void) = default;
		~NonIntrusiveRc(void) { this->release(); }

		NonIntrusiveRc(nullptr_t) {}
		NonIntrusiveRc& operator=(nullptr_t)
		{
			this->release();
			return *this;
		}

		explicit NonIntrusiveRc(ControlBlock* cb)
		: m_ControlBlock(cb)
		{
			if (m_ControlBlock)
				m_ControlBlock->inc_strong_count();
		}

		template<typename... t_Args>
		static NonIntrusiveRc Make(t_Args&&... __args)
		{
			return NonIntrusiveRc(ControlBlock::Make(std::forward<t_Args>(__args)...));
		}

		NonIntrusiveRc(Box<T>&& unique)
		: NonIntrusiveRc(new ControlBlock(unique.release()))
		{}

		NonIntrusiveRc(const NonIntrusiveRc& other)
		: NonIntrusiveRc(other.m_ControlBlock)
		{}

		NonIntrusiveRc& operator=(const NonIntrusiveRc& other)
		{
			if (m_ControlBlock == other.m_ControlBlock)
				return *this;

			this->release();
			if (other)
			{
				m_ControlBlock = other.m_ControlBlock;
				m_ControlBlock->inc_strong_count();
			}

			return *this;
		}

		NonIntrusiveRc(NonIntrusiveRc&& other) noexcept
		: m_ControlBlock(std::exchange(other.m_ControlBlock, nullptr))
		{}

		NonIntrusiveRc& operator=(NonIntrusiveRc&& other) noexcept
		{
			if (m_ControlBlock == other.m_ControlBlock)
				return *this;

			this->release();
			if (other)
				m_ControlBlock = std::exchange(other.m_ControlBlock, nullptr);

			return *this;
		}

		void swap(NonIntrusiveRc& other)
		{
			std::swap(m_ControlBlock, other.m_ControlBlock);
		}

		void release(void)
		{
			if (this->expired())
				return;

			m_ControlBlock->dec_strong_count();
			if (!m_ControlBlock->strong_count.load())
			{
				delete m_ControlBlock->ptr;
				m_ControlBlock->ptr = nullptr;

				if (!m_ControlBlock->weak_count.load())
					delete m_ControlBlock;
			}
			m_ControlBlock = nullptr;
		}

		template<typename U, std::enable_if_t<std::is_base_of_v<U, T>, int> = 0>
		[[nodiscard]] operator NonIntrusiveRc<U>(void) const
		{
			return NonIntrusiveRc<U>((typename NonIntrusiveRc<U>::ControlBlock*)m_ControlBlock);
		}

		[[nodiscard]] inline u32 strong_count(void) const { return m_ControlBlock->strong_count.load(); }
		[[nodiscard]] inline u32 weak_count(void) const { return m_ControlBlock->weak_count.load(); }

		[[nodiscard]] inline T* ptr(void) const { return m_ControlBlock->ptr; }
		[[nodiscard]] inline T& operator*(void) const { return *m_ControlBlock->ptr; }
		[[nodiscard]] inline T* operator->(void) const { return m_ControlBlock->ptr; }

		[[nodiscard]] inline auto operator<=>(const NonIntrusiveRc& other) const { return m_ControlBlock->ptr <=> other.m_ControlBlock->ptr; }
		[[nodiscard]] inline auto operator==(const NonIntrusiveRc& other) const { return m_ControlBlock == other.m_ControlBlock; }

		[[nodiscard]] inline bool expired(void) const { return !m_ControlBlock || !m_ControlBlock->strong_count.load(); }
		[[nodiscard]] inline operator bool(void) const { return m_ControlBlock; }
	private:
		friend class Weak<T>;
		friend class Weak<const T>;

		template<typename To, typename From>
		friend NonIntrusiveRc<To> static_ref_cast(const NonIntrusiveRc<From>& from);

		template<typename To, typename From>
		friend NonIntrusiveRc<To> dynamic_ref_cast(const NonIntrusiveRc<From>& from);

		ControlBlock* m_ControlBlock = nullptr;
	};

	template<typename To, typename From>
	[[nodiscard]] NonIntrusiveRc<To> static_ref_cast(const NonIntrusiveRc<From>& from)
	{
		if (!from)
			return nullptr;

		return NonIntrusiveRc<To>((RcControlBlock<To>*)from.m_ControlBlock);
	}

	template<typename To, typename From>
	[[nodiscard]] NonIntrusiveRc<To> dynamic_ref_cast(const NonIntrusiveRc<From>& from)
	{
		if (!from)
			return nullptr;

		using FromPtr = decltype(from.ptr());

		To* casted_ptr = dynamic_cast<To*>(const_cast<std::remove_const_t<std::remove_pointer_t<FromPtr>>*>(from.ptr()));
		if (!casted_ptr)
			return nullptr;

		return NonIntrusiveRc<To>((RcControlBlock<To>*)from.m_ControlBlock);
	}

	template<typename T>
	class Weak {
	public:
		using ControlBlock = RcControlBlock<T>;

		Weak(void) = default;
		~Weak(void) { this->release(); }

		Weak(nullptr_t) {}
		Weak& operator=(nullptr_t)
		{
			this->release();
			return *this;
		}

		explicit Weak(ControlBlock* cb) : m_ControlBlock(cb)
		{
			if (m_ControlBlock)
				m_ControlBlock->inc_weak_count();
		}

		Weak(const Weak& other)
		: Weak(other.m_ControlBlock)
		{}

		Weak& operator=(const Weak& other)
		{
			if (m_ControlBlock == other.m_ControlBlock)
				return *this;

			this->release();
			if (other)
			{
				m_ControlBlock = other.m_ControlBlock;
				m_ControlBlock->inc_weak_count();
			}
			return *this;
		}

		Weak(Weak&& other) noexcept
		: m_ControlBlock(std::exchange(other.m_ControlBlock, nullptr))
		{}

		Weak& operator=(Weak&& other) noexcept
		{
			if (m_ControlBlock == other.m_ControlBlock)
				return *this;

			this->release();
			if (other)
				m_ControlBlock = std::exchange(other.m_ControlBlock, nullptr);

			return *this;
		}

		Weak(const NonIntrusiveRc<T>& ref)
		: Weak(ref.m_ControlBlock)
		{}

		Weak& operator=(const NonIntrusiveRc<T>& ref)
		{
			if (m_ControlBlock == ref.m_ControlBlock)
				return *this;

			this->release();
			if ((m_ControlBlock = ref.m_ControlBlock))
				m_ControlBlock->inc_weak_count();

			return *this;
		}

		template<typename U, std::enable_if_t<std::is_convertible_v<U*, T*>, int> = 0>
		Weak(const NonIntrusiveRc<U>& ref)
		: Weak((const NonIntrusiveRc<T>&)ref)
		{}

		template<typename U, std::enable_if_t<std::is_convertible_v<U*, T*>, int> = 0>
		Weak& operator=(const NonIntrusiveRc<U>& ref)
		{
			return this->operator=((const NonIntrusiveRc<T>&)ref);
		}

		void release(void)
		{
			if (this->expired())
				return;

			m_ControlBlock->dec_weak_count();
			if (!m_ControlBlock->weak_count.load() && !m_ControlBlock->strong_count.load())
				delete m_ControlBlock;

			m_ControlBlock = nullptr;
		}

		[[nodiscard]] NonIntrusiveRc<T> lock(void) const
		{
			return m_ControlBlock->strong_count.load() ? NonIntrusiveRc<T>(m_ControlBlock) : nullptr;
		}

		template<typename U, std::enable_if_t<std::is_base_of_v<U, T>, int> = 0>
		[[nodiscard]] operator Weak<U>(void) const
		{
			return Weak<U>((typename Weak<U>::ControlBlock*)this->m_ControlBlock);
		}

		[[nodiscard]] inline auto operator==(const Weak& other) const { return m_ControlBlock == other.m_ControlBlock; }

		[[nodiscard]] inline bool expired(void) const { return !m_ControlBlock || !m_ControlBlock->strong_count.load(); }

		[[nodiscard]] inline u64 strong_count(void) const { return m_ControlBlock->strong_count.load(); }
		[[nodiscard]] inline u64 weak_count(void) const { return m_ControlBlock->weak_count.load(); }

		[[nodiscard]] inline operator bool(void) const { return m_ControlBlock; }
	private:
		template<typename To, typename From>
		friend Weak<To> static_ref_cast(const Weak<From>& from);

		template<typename To, typename From>
		friend Weak<To> dynamic_ref_cast(const Weak<From>& from);

		ControlBlock* m_ControlBlock = nullptr;
	};

	template<typename To, typename From>
	[[nodiscard]] Weak<To> static_ref_cast(const Weak<From>& from)
	{
		if (!from)
			return nullptr;

		return Weak<To>((RcControlBlock<To>*)from.m_ControlBlock);
	}

	template<typename To, typename From>
	[[nodiscard]] Weak<To> dynamic_ref_cast(const Weak<From>& from)
	{
		if (!from)
			return nullptr;

		NonIntrusiveRc<From> locked = from.lock();
		if (!locked)
			return nullptr;

		using FromPtr = decltype(locked.ptr());

		To* casted_ptr = dynamic_cast<To*>(const_cast<std::remove_const_t<std::remove_pointer_t<FromPtr>>*>(locked.ptr()));
		if (!casted_ptr)
			return nullptr;

		return Weak<To>((RcControlBlock<To>*)from.m_ControlBlock);
	}

	class IntrusiveRcBase {
	public:
		void inc_strong_count(void) const { m_StrongCount.fetch_add(1, std::memory_order_relaxed); }
		void dec_strong_count(void) const { m_StrongCount.fetch_sub(1, std::memory_order_relaxed); }

		[[nodiscard]] inline u32 strong_count(void) const { return m_StrongCount.load(); }
	private:
		mutable std::atomic<u32> m_StrongCount = 0;
	};

	template<typename T>
	class IntrusiveRc {
	public:
		IntrusiveRc(void) = default;
		~IntrusiveRc(void) { this->release(); }

		IntrusiveRc(nullptr_t) {}
		IntrusiveRc& operator=(nullptr_t)
		{
			this->release();
			return *this;
		}

		explicit IntrusiveRc(T* ptr)
		: m_Ptr(ptr)
		{
			if (m_Ptr)
				m_Ptr->inc_strong_count();
		}

		template<typename... t_Args>
		static IntrusiveRc Make(t_Args&&... __args)
		{
			return IntrusiveRc(new T(std::forward<t_Args>(__args)...));
		}

		IntrusiveRc(Box<T>&& unique)
		: m_Ptr(unique.release())
		{}

		IntrusiveRc(const IntrusiveRc& other)
		: IntrusiveRc(other.m_Ptr)
		{}

		IntrusiveRc& operator=(const IntrusiveRc& other)
		{
			if (m_Ptr == other.m_Ptr)
				return *this;

			this->release();
			if (other)
			{
				m_Ptr = other.m_Ptr;
				m_Ptr->inc_strong_count();
			}

			return *this;
		}

		IntrusiveRc(IntrusiveRc&& other) noexcept
		: m_Ptr(std::exchange(other.m_Ptr, nullptr))
		{}

		IntrusiveRc& operator=(IntrusiveRc&& other) noexcept
		{
			if (m_Ptr == other.m_Ptr)
				return *this;

			this->release();
			if (other)
				m_Ptr = std::exchange(other.m_Ptr, nullptr);

			return *this;
		}

		void swap(IntrusiveRc& other)
		{
			std::swap(m_Ptr, other.m_Ptr);
		}

		void release(void)
		{
			if (!m_Ptr)
				return;

			m_Ptr->dec_strong_count();
			if (!m_Ptr->strong_count())
			{
				delete m_Ptr;
			}
			m_Ptr = nullptr;
		}

		template<typename U, std::enable_if_t<std::is_base_of_v<U, T>, int> = 0>
		[[nodiscard]] operator IntrusiveRc<U>(void) const
		{
			return IntrusiveRc<U>((U*)m_Ptr);
		}

		[[nodiscard]] inline u64 strong_count(void) const { return m_Ptr->strong_count(); }

		[[nodiscard]] inline T* ptr(void) const { return m_Ptr; }
		[[nodiscard]] inline T& operator*(void) const { return *m_Ptr; }
		[[nodiscard]] inline T* operator->(void) const { return m_Ptr; }

		[[nodiscard]] inline auto operator<=>(const IntrusiveRc& other) const { return m_Ptr <=> other.m_Ptr; }
		[[nodiscard]] inline auto operator==(const IntrusiveRc& other) const { return m_Ptr == other.m_Ptr; }

		[[nodiscard]] inline operator bool(void) const { return m_Ptr; }
	private:
		template<typename To, typename From>
		friend IntrusiveRc<To> static_ref_cast(const IntrusiveRc<From>& from);

		template<typename To, typename From>
		friend IntrusiveRc<To> dynamic_ref_cast(const IntrusiveRc<From>& from);

		T* m_Ptr = nullptr;
	};

	template<typename To, typename From>
	[[nodiscard]] IntrusiveRc<To> static_ref_cast(const IntrusiveRc<From>& from)
	{
		if (!from)
			return nullptr;

		return IntrusiveRc<To>((To*)from.m_Ptr);
	}

	template<typename To, typename From>
	[[nodiscard]] IntrusiveRc<To> dynamic_ref_cast(const IntrusiveRc<From>& from)
	{
		if (!from)
			return nullptr;

		using FromPtr = decltype(from.m_Ptr);

		To* casted_ptr = dynamic_cast<To*>(const_cast<std::remove_const_t<std::remove_pointer_t<FromPtr>>*>(from.m_Ptr));
		if (!casted_ptr)
			return nullptr;
		return IntrusiveRc<To>((To*)from.m_Ptr);
	}

	template<typename T>
	using Rc = std::conditional_t<
		std::is_base_of_v<IntrusiveRcBase, T>,
		IntrusiveRc<T>,
		NonIntrusiveRc<T>
	>;

	template<typename T>
	class View {
	public:
		View(void) = default;
		~View(void) = default;

		View(nullptr_t) {}
		View& operator=(nullptr_t)
		{
			this->destroy();
			return *this;
		}

		View(T* ptr) : m_Ptr(ptr) {}
		View& operator=(T* ptr)
		{
			m_Ptr = ptr;
			return *this;
		}

		template<typename U, std::enable_if_t<std::is_convertible_v<U*, T*>, int> = 0>
		View(const Box<U>& ref)
		: m_Ptr((T*)ref.ptr())
		{}

		template<typename U, std::enable_if_t<std::is_convertible_v<U*, T*>, int> = 0>
		View(const NonIntrusiveRc<U>& ref)
		: m_Ptr(ref ? (T*)ref.ptr() : nullptr)
		{}

		template<typename U, std::enable_if_t<std::is_convertible_v<U*, T*>, int> = 0>
		View(const Weak<U>& ref)
		: m_Ptr(ref ? (T*)ref.lock().ptr() : nullptr)
		{}

		template<typename U, std::enable_if_t<std::is_convertible_v<U*, T*>, int> = 0>
		View(const IntrusiveRc<U>& ref)
		: m_Ptr(ref ? (T*)ref.ptr() : nullptr)
		{}

		template<typename U, std::enable_if_t<std::is_convertible_v<U*, T*>, int> = 0>
		View& operator=(const Box<U>& ref)
		{
			m_Ptr = (T*)ref.ptr();
			return *this;
		}

		template<typename U, std::enable_if_t<std::is_convertible_v<U*, T*>, int> = 0>
		View& operator=(const NonIntrusiveRc<U>& ref)
		{
			m_Ptr = ref ? (T*)ref.ptr() : nullptr;
			return *this;
		}

		template<typename U, std::enable_if_t<std::is_convertible_v<U*, T*>, int> = 0>
		View& operator=(const Weak<U>& ref)
		{
			m_Ptr = ref ? (T*)ref.lock().ptr() : nullptr;
			return *this;
		}

		template<typename U, std::enable_if_t<std::is_convertible_v<U*, T*>, int> = 0>
		View& operator=(const IntrusiveRc<U>& ref)
		{
			m_Ptr = ref ? (T*)ref.ptr() : nullptr;
			return *this;
		}

		View(const View& other) = default;
		View& operator=(const View& other)
		{
			m_Ptr = other.m_Ptr;
			return *this;
		}

		View(View&& other) noexcept
		: m_Ptr(std::exchange(other.m_Ptr, nullptr))
		{}

		View& operator=(View&& other) noexcept
		{
			if (m_Ptr == other.m_Ptr)
				return *this;

			m_Ptr = std::exchange(other.m_Ptr, nullptr);
			return *this;
		}

		void swap(View& other)
		{
			std::swap(m_Ptr, other.m_Ptr);
		}

		void destroy(void)
		{
			m_Ptr = nullptr;
		}

		T* release(void)
		{
			if (!m_Ptr)
				return nullptr;

			T* temp = m_Ptr;
			m_Ptr = nullptr;
			return temp;
		}

		template<typename U, std::enable_if_t<std::is_base_of_v<U, T>, int> = 0>
		[[nodiscard]] inline operator View<U>(void) const { return View<U>(m_Ptr); }

		[[nodiscard]] inline T* ptr(void) const { return m_Ptr; }
		[[nodiscard]] inline T& operator*(void) const { return *m_Ptr; }
		[[nodiscard]] inline T* operator->(void) const { return m_Ptr; }

		[[nodiscard]] inline auto operator<=>(const View& other) const { return m_Ptr <=> other.m_Ptr; }
		[[nodiscard]] inline auto operator==(const View& other) const { return m_Ptr == other.m_Ptr; }

		[[nodiscard]] inline operator bool(void) const { return m_Ptr; }
	private:
		template<typename To, typename From>
		friend View<To> static_ref_cast(const View<From>& from);

		template<typename To, typename From>
		friend View<To> dynamic_ref_cast(const View<From>& from);

		T* m_Ptr = nullptr;
	};

	template<typename To, typename From>
	[[nodiscard]] View<To> static_ref_cast(const View<From>& from)
	{
		using FromPtr = decltype(from.ptr());
		To* casted = (To*)const_cast<std::remove_const_t<std::remove_pointer_t<FromPtr>>*>(from.ptr());
		return View<To>(casted);
	}

	template<typename To, typename From>
	[[nodiscard]] View<To> dynamic_ref_cast(const View<From>& from)
	{
		using FromPtr = decltype(from.ptr());
		To* casted = dynamic_cast<To*>(const_cast<std::remove_const_t<std::remove_pointer_t<FromPtr>>*>(from.ptr()));
		return View<To>(casted);
	}
} // namespace Na2

#endif // NA2_REF_HPP