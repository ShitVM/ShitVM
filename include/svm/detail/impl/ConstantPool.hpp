#pragma once
#include <svm/ConstantPool.hpp>

namespace svm {
	inline std::ostream& operator<<(std::ostream& stream, const ConstantPool& constantPool) {
		return core::operator<<(stream, constantPool);
	}
}