#pragma once

#include <svm/Object.hpp>
#include <svm/Type.hpp>

#include <cstdint>
#include <ostream>
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
		const T& GetConstant(std::uint32_t index) const noexcept;
		Type GetConstantType(std::uint32_t index) const noexcept;
		template<typename T>
		std::uint32_t GetOffset() const noexcept;
		std::uint32_t GetIntOffset() const noexcept;
		std::uint32_t GetLongOffset() const noexcept;
		std::uint32_t GetDoubleOffset() const noexcept;
		template<typename T>
		std::uint32_t GetCount() const noexcept;
		std::uint32_t GetIntCount() const noexcept;
		std::uint32_t GetLongCount() const noexcept;
		std::uint32_t GetDoubleCount() const noexcept;
		std::uint32_t GetAllCount() const noexcept;

		const std::vector<IntObject>& GetIntPool() const noexcept;
		void SetIntPool(std::vector<IntObject> newIntPool) noexcept;
		const std::vector<LongObject>& GetLongPool() const noexcept;
		void SetLongPool(std::vector<LongObject> newLongPool) noexcept;
		const std::vector<DoubleObject>& GetDoublePool() const noexcept;
		void SetDoublePool(std::vector<DoubleObject> newDoublePool) noexcept;
	};

	std::ostream& operator<<(std::ostream& stream, const ConstantPool& constantPool);
}

#include "detail/impl/ConstantPool.hpp"