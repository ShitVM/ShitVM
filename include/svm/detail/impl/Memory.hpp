#pragma once
#include <svm/Memory.hpp>

#include <algorithm>
#include <cstdint>

namespace svm {
	template<typename T>
	T ReverseEndian(const T& value) noexcept {
		union Transformer {
			std::uint8_t Bytes[sizeof(value)];
		} temp;
		temp = reinterpret_cast<const Transformer&>(value);
		std::reverse(temp.Bytes, temp.Bytes + sizeof(value));
		return reinterpret_cast<T&>(temp);
	}

	template<typename T>
	std::size_t Pade(std::size_t dataSize) noexcept {
		const std::size_t temp = dataSize / sizeof(T) * sizeof(T);
		if (dataSize == temp) return dataSize;
		else return temp + sizeof(void*);
	}
}