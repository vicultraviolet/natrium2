#if !defined(NA2_CONTEXT_HPP)
#define NA2_CONTEXT_HPP

#include "Natrium2/Core.hpp"

namespace Na2
{
	struct ContextInitInfo {

	};

	class Context {
	public:
		Context(void) = default;

		explicit Context(const ContextInitInfo& info);
		~Context(void) { this->destroy(); }

		void destroy(void);

		Context(const Context& other) = delete;
		Context& operator=(const Context& other) = delete;

		Context(Context&& other) noexcept;
		Context& operator=(Context&& other) noexcept;

		void bind(void) { Context::s_Context = this; }
		void unbind(void) { Context::s_Context = nullptr; }

		[[nodiscard]] static inline bool Exists(void) { return Context::s_Context; }
		[[nodiscard]] static inline View<Context> Get(void) { return Context::s_Context; }

		[[nodiscard]] inline const auto& exec_path(void) const { return m_ExecPath; }
		[[nodiscard]] inline const auto& exec_dir(void)  const { return m_ExecDir; }
		[[nodiscard]] inline const auto& exec_name(void) const { return m_ExecName; }

	private:
		std::filesystem::path m_ExecPath, m_ExecDir, m_ExecName;
		std::string_view m_Version;

		static inline View<Context> s_Context = nullptr;
	};
} // namespace Na2

#endif // NA2_CONTEXT_HPP