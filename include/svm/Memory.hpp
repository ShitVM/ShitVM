#pragma once

#include <svm/Macro.hpp>

#include <cstddef>

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

	template<typename T>
	T ReverseEndian(const T& value) noexcept;

	template<typename T = void*>
	std::size_t Pade(std::size_t dataSize) noexcept;
}

#include "detail/impl/Memory.hpp"