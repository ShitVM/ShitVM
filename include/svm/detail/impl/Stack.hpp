#pragma once
#include <svm/Stack.hpp>

namespace svm {
	template<typename T>
	bool Stack::Push(const T& value) noexcept {
		if (GetFreeSize() < sizeof(value)) return false;

		auto& top = *(m_Data.rbegin() + (m_Used += sizeof(value)) - 1);
		reinterpret_cast<T&>(top) = value;
		return true;
	}
	template<typename T>
	std::optional<T> Stack::Pop() noexcept {
		T* result = GetTop<T>();
		if (!result) return std::nullopt;

		m_Used -= sizeof(T);
		return *result;
	}
	template<typename T>
	const T* Stack::Get(std::size_t offset) const noexcept {
		if (m_Used < offset - sizeof(T)) return nullptr;

		const auto& top = *(m_Data.rbegin() + offset - 1);
		return reinterpret_cast<const T*>(&top);
	}
	template<typename T>
	T* Stack::Get(std::size_t offset) noexcept {
		if (m_Used < offset - sizeof(T)) return nullptr;

		auto& top = *(m_Data.rbegin() + offset - 1);
		return reinterpret_cast<T*>(&top);
	}
	template<typename T>
	const T* Stack::GetTop() const noexcept {
		return Get<T>(m_Used);
	}
	template<typename T>
	T* Stack::GetTop() noexcept {
		return Get<T>(m_Used);
	}
}