#pragma once

#include <svm/Macro.hpp>

namespace svm {
	enum class Endian {
		Little,
		Big,
	};

#ifdef SVM_LITTLE
	constexpr Endian GetEndian() noexcept {
		return Endian::Little;
	}
#else
	Endian GetEndian() noexcept;
#endif
}