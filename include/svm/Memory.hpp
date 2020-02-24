#pragma once

#include <svm/Macro.hpp>

#include <algorithm>
#include <cstdint>

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
	T ReverseEndian(const T& value) noexcept {
		union transformer {
			std::uint8_t Bytes[sizeof(value)];
		} temp;
		temp = *reinterpret_cast<const transformer*>(&value);
		std::reverse(temp.Bytes, temp.Bytes + sizeof(value));
		return *reinterpret_cast<T*>(&temp);
	}
}