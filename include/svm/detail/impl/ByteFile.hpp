#pragma once
#include <svm/ByteFile.hpp>

namespace svm {
	inline std::ostream& operator<<(std::ostream& stream, const ByteFile& byteFile) {
		return core::operator<<(stream, byteFile);
	}
}