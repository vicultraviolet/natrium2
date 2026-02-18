#ifndef NA2_ERROR_CODES_HPP
#define NA2_ERROR_CODES_HPP

#include "Natrium2/Core.hpp"

namespace Na2
{
	enum class FileErrorCode : u8 {
		None = 0,
		NotFound, PermissionDenied, InvalidFormat, Unknown
	};

	enum class ShaderErrorCode : u8 {
		None = 0,
		CompilationFailed, InvalidStage, InvalidEntryPoint, Unknown
	};
} // namespace Na2

#endif // NA2_ERROR_CODES_HPP