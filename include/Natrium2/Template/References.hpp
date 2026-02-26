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
		std::atomic<u64> strong_count = 0;
		std::atomic<u64> weak_count = 0;

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
	class Rc {
	public:
		using ControlBlock = RcControlBlock<T>;

		Rc(void) = default;
		~Rc(void) { this->release(); }

		Rc(nullptr_t) {}
		Rc& operator=(nullptr_t)
		{
			this->release();
			return *this;
		}

		explicit Rc(ControlBlock* cb)
		: m_ControlBlock(cb)
		{
			if (m_ControlBlock)
				m_ControlBlock->inc_strong_count();
		}

		template<typename... t_Args>
		static Rc Make(t_Args&&... __args)
		{
			return Rc(ControlBlock::Make(std::forward<t_Args>(__args)...));
		}

		Rc(Box<T>&& unique)
		: Rc(new ControlBlock(unique.release()))
		{}

		Rc(const Rc& other)
		: Rc(other.m_ControlBlock)
		{}

		Rc& operator=(const Rc& other)
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

		Rc(Rc&& other) noexcept
		: m_ControlBlock(std::exchange(other.m_ControlBlock, nullptr))
		{}

		Rc& operator=(Rc&& other) noexcept
		{
			if (m_ControlBlock == other.m_ControlBlock)
				return *this;

			this->release();
			if (other)
				m_ControlBlock = std::exchange(other.m_ControlBlock, nullptr);
			return *this;
		}

		void swap(Rc& other)
		{
			std::swap(m_ControlBlock, other.m_ControlBlock);
		}

		void release(void)
		{
			if (this->expired())
				return;

			m_ControlBlock->dec_strong_count();
			if (!m_ControlBlock->strong_count)
			{
				delete m_ControlBlock->ptr;
				m_ControlBlock->ptr = nullptr;

				if (!m_ControlBlock->weak_count)
					delete m_ControlBlock;
			}
			m_ControlBlock = nullptr;
		}

		template<typename U, std::enable_if_t<std::is_base_of_v<U, T>, int> = 0>
		[[nodiscard]] operator Rc<U>(void) const
		{
			return Rc<U>((typename Rc<U>::ControlBlock*)this->m_ControlBlock);
		}

		[[nodiscard]] inline u64 strong_count(void) const { return m_ControlBlock->strong_count.load(); }
		[[nodiscard]] inline u64 weak_count(void) const { return m_ControlBlock->weak_count.load(); }

		[[nodiscard]] inline T* ptr(void) const { return m_ControlBlock->ptr; }
		[[nodiscard]] inline T& operator*(void) const { return *m_ControlBlock->ptr; }
		[[nodiscard]] inline T* operator->(void) const { return m_ControlBlock->ptr; }

		[[nodiscard]] inline auto operator<=>(const Rc& other) const { return m_ControlBlock->ptr <=> other.m_ControlBlock->ptr; }
		[[nodiscard]] inline auto operator==(const Rc& other) const { return m_ControlBlock->ptr == other.m_ControlBlock->ptr; }

		[[nodiscard]] inline bool expired(void) const { return !m_ControlBlock || !m_ControlBlock->strong_count.load(); }
		[[nodiscard]] inline operator bool(void) const { return m_ControlBlock; }
	private:
		friend class Weak<T>;
		friend class Weak<const T>;

		template<typename To, typename From>
		friend Rc<To> static_ref_cast(const Rc<From>& from);

		template<typename To, typename From>
		friend Rc<To> dynamic_ref_cast(const Rc<From>& from);

		ControlBlock* m_ControlBlock = nullptr;
	};

	template<typename T>
	using Ref = Rc<T>;

	template<typename To, typename From>
	[[nodiscard]] Rc<To> static_ref_cast(const Rc<From>& from)
	{
		if (!from)
			return nullptr;

		return Rc<To>((RcControlBlock<To>*)from.m_ControlBlock);
	}

	template<typename To, typename From>
	[[nodiscard]] Rc<To> dynamic_ref_cast(const Rc<From>& from)
	{
		if (!from)
			return nullptr;

		using FromPtr = decltype(from.ptr());

		To* casted_ptr = dynamic_cast<To*>(const_cast<std::remove_const_t<std::remove_pointer_t<FromPtr>>*>(from.ptr()));
		if (!casted_ptr)
			return nullptr;
		return Rc<To>((RcControlBlock<To>*)from.m_ControlBlock);
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

		Weak(const Rc<T>& ref)
		: Weak(ref.m_ControlBlock)
		{}

		Weak& operator=(const Rc<T>& ref)
		{
			if (m_ControlBlock == ref.m_ControlBlock)
				return *this;

			this->release();
			if ((m_ControlBlock = ref.m_ControlBlock))
				m_ControlBlock->inc_weak_count();

			return *this;
		}

		template<typename U, std::enable_if_t<std::is_convertible_v<U*, T*>, int> = 0>
		Weak(const Rc<U>& ref)
		: Weak((const Rc<T>&)ref)
		{}

		template<typename U, std::enable_if_t<std::is_convertible_v<U*, T*>, int> = 0>
		Weak& operator=(const Rc<U>& ref)
		{
			return this->operator=((const Rc<T>&)ref);
		}

		void release(void)
		{
			if (this->expired())
				return;

			m_ControlBlock->dec_weak_count();
			if (!m_ControlBlock->weak_count && !m_ControlBlock->strong_count)
				delete m_ControlBlock;

			m_ControlBlock = nullptr;
		}

		[[nodiscard]] Rc<T> lock(void) const
		{
			return m_ControlBlock->strong_count.load() ? Rc<T>(m_ControlBlock) : nullptr;
		}

		template<typename U, std::enable_if_t<std::is_base_of_v<U, T>, int> = 0>
		[[nodiscard]] operator Weak<U>(void) const
		{
			return Weak<U>((typename Weak<U>::ControlBlock*)this->m_ControlBlock);
		}

		[[nodiscard]] inline bool expired(void) const { return !m_ControlBlock || !m_ControlBlock->strong_count.load(); }

		[[nodiscard]] inline u64 strong_count(void) const { return m_ControlBlock->strong_count.load(); }
		[[nodiscard]] inline u64 weak_count(void) const { return m_ControlBlock->weak_count.load(); }

		[[nodiscard]] inline operator bool(void) const { return m_ControlBlock;  }
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

		Rc<From> locked = from.lock();
		if (!locked)
			return nullptr;

		using FromPtr = decltype(locked.ptr());

		To* casted_ptr = dynamic_cast<To*>(const_cast<std::remove_const_t<std::remove_pointer_t<FromPtr>>*>(locked.ptr()));
		if (!casted_ptr)
			return nullptr;

		return Weak<To>((RcControlBlock<To>*)from.m_ControlBlock);
	}

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
		View(const Rc<U>& ref)
		: m_Ptr(ref ? (T*)ref.ptr() : nullptr)
		{}

		template<typename U, std::enable_if_t<std::is_convertible_v<U*, T*>, int> = 0>
		View(const Weak<U>& ref)
		: m_Ptr(ref ? (T*)ref.lock().ptr() : nullptr)
		{}

		template<typename U, std::enable_if_t<std::is_convertible_v<U*, T*>, int> = 0>
		View& operator=(const Box<U>& ref)
		{
			m_Ptr = (T*)ref.ptr();
			return *this;
		}

		template<typename U, std::enable_if_t<std::is_convertible_v<U*, T*>, int> = 0>
		View& operator=(const Rc<U>& ref)
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