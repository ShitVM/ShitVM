#pragma once

#include <svm/core/ByteFile.hpp>

#include <ostream>

namespace svm {
	using ByteFile = core::ByteFile;

	inline std::ostream& operator<<(std::ostream& stream, const ByteFile& byteFile);
}

#include "detail/impl/ByteFile.hpp"