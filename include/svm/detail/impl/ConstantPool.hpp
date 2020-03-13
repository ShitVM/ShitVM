#pragma once
#include <svm/ConstantPool.hpp>

#include <type_traits>

namespace svm {
	template<typename T>
	const T& ConstantPool::GetConstant(std::uint32_t index) const noexcept {
		if constexpr (std::is_same_v<IntObject, T>) return m_IntPool[index - GetOffset<T>()];
		else if constexpr (std::is_same_v<LongObject, T>) return m_LongPool[index - GetOffset<T>()];
		else if constexpr (std::is_same_v<DoubleObject, T>) return m_DoublePool[index - GetOffset<T>()];
		else static_assert(false);
	}
	template<typename T>
	std::uint32_t ConstantPool::GetOffset() const noexcept {
		if constexpr (std::is_same_v<IntObject, T>) return GetIntOffset();
		else if constexpr (std::is_same_v<LongObject, T>) return GetLongOffset();
		else if constexpr (std::is_same_v<DoubleObject, T>) return GetDoubleOffset();
		else static_assert(false);
	}
	template<typename T>
	std::uint32_t ConstantPool::GetCount() const noexcept {
		if constexpr (std::is_same_v<IntObject, T>) return GetIntCount();
		else if constexpr (std::is_same_v<LongObject, T>) return GetLongCount();
		else if constexpr (std::is_same_v<DoubleObject, T>) return GetDoubleCount();
		else static_assert(false);
	}
}