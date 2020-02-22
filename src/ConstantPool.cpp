#include <svm/ConstantPool.hpp>

#include <utility>

namespace svm {
	ConstantPool::ConstantPool(std::unique_ptr<std::uint8_t[]>&& pool, std::uint32_t intCount, std::uint32_t longCount, std::uint32_t doubleCount) noexcept
		: m_Pool(std::move(pool)), m_IntCount(intCount), m_LongCount(longCount), m_DoubleCount(doubleCount) {}
	ConstantPool::ConstantPool(ConstantPool&& pool) noexcept
		: m_Pool(std::move(pool.m_Pool)), m_IntCount(pool.m_IntCount), m_LongCount(pool.m_LongCount), m_DoubleCount(pool.m_DoubleCount) {}

	ConstantPool& ConstantPool::operator=(ConstantPool&& pool) noexcept {
		m_Pool = std::move(pool.m_Pool);
		m_IntCount = pool.m_IntCount;
		m_LongCount = pool.m_LongCount;
		m_DoubleCount = pool.m_DoubleCount;
		return *this;
	}

	std::uint32_t ConstantPool::GetIntOffset() const noexcept {
		return 0;
	}
	std::uint32_t ConstantPool::GetLongOffset() const noexcept {
		return GetIntCount();
	}
	std::uint32_t ConstantPool::GetDoubleOffset() const noexcept {
		return GetLongOffset() + GetLongCount();
	}
	std::uint32_t ConstantPool::GetIntCount() const noexcept {
		return m_IntCount;
	}
	std::uint32_t ConstantPool::GetLongCount() const noexcept {
		return m_LongCount;
	}
	std::uint32_t ConstantPool::GetDoubleCount() const noexcept {
		return m_DoubleCount;
	}
}