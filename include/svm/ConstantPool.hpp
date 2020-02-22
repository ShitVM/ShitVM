#pragma once

#include <svm/Object.hpp>

#include <cstdint>
#include <memory>
#include <type_traits>

namespace svm {
	class ConstantPool final {
	private:
		std::unique_ptr<std::uint8_t[]> m_Pool;
		std::uint32_t m_IntCount = 0, m_LongCount = 0, m_DoubleCount = 0;

	public:
		ConstantPool() noexcept = default;
		ConstantPool(std::unique_ptr<std::uint8_t[]>&& pool, std::uint32_t intCount, std::uint32_t longCount, std::uint32_t doubleCount) noexcept;
		ConstantPool(ConstantPool&& pool) noexcept;
		~ConstantPool() = default;

	public:
		ConstantPool& operator=(ConstantPool&& pool) noexcept;
		bool operator==(const ConstantPool&) = delete;
		bool operator!=(const ConstantPool&) = delete;

	public:
		void Clear() noexcept;
		bool IsEmpty() const noexcept;

		template<typename T>
		const T& GetConstant(std::uint32_t index) const noexcept {
			static_assert(std::is_base_of_v<Object, T>);

			std::uint32_t newIndex = index;
			if (index >= GetDoubleOffset()) {
				newIndex -= GetDoubleOffset();
			} else if (index >= GetLongOffset()) {
				newIndex -= GetLongOffset();
			}

			return *reinterpret_cast<const T*>(m_Pool.get() + GetOffset<T>() + index);
		}
		template<typename T>
		std::uint32_t GetOffset() const noexcept {
			static_assert(std::is_base_of_v<Object, T>);

			if constexpr (std::is_same_v<IntObject, T>) return GetIntOffset();
			else if constexpr (std::is_same_v<LongObject, T>) return GetLongOffset();
			else if constexpr (std::is_same_v<DoubleObject, T>) return GetDoubleOffset();
		}
		std::uint32_t GetIntOffset() const noexcept;
		std::uint32_t GetLongOffset() const noexcept;
		std::uint32_t GetDoubleOffset() const noexcept;
		template<typename T>
		std::uint32_t GetCount() const noexcept {
			static_assert(std::is_base_of_v<Object, T>);

			if constexpr (std::is_same_v<IntObject, T>) return GetIntCount();
			else if constexpr (std::is_same_v<LongObject, T>) return GetLongCount();
			else if constexpr (std::is_same_v<DoubleObject, T>) return GetDoubleCount();
		}
		std::uint32_t GetIntCount() const noexcept;
		std::uint32_t GetLongCount() const noexcept;
		std::uint32_t GetDoubleCount() const noexcept;
	};
}