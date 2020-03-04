#pragma once

#include <svm/Object.hpp>

#include <cstdint>
#include <ostream>
#include <type_traits>
#include <vector>

namespace svm {
	class ConstantPool final {
	private:
		std::vector<IntObject> m_IntPool;
		std::vector<LongObject> m_LongPool;
		std::vector<DoubleObject> m_DoublePool;

	public:
		ConstantPool() noexcept = default;
		ConstantPool(std::vector<IntObject> intPool, std::vector<LongObject> longPool, std::vector<DoubleObject> doublePool) noexcept;
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

			if constexpr (std::is_same_v<IntObject, T>) {
				return m_IntPool[index - GetOffset<T>()];
			} else if constexpr (std::is_same_v<LongObject, T>) {
				return m_LongPool[index - GetOffset<T>()];
			} else if constexpr (std::is_same_v<DoubleObject, T>) {
				return m_DoublePool[index - GetOffset<T>()];
			}
		}
		Type GetConstantType(std::uint32_t index) const noexcept;
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
		std::uint32_t GetAllCount() const noexcept;
	};

	std::ostream& operator<<(std::ostream& stream, const ConstantPool& constantPool);
}