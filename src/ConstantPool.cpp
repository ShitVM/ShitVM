#include <svm/ConstantPool.hpp>

#include <svm/IO.hpp>
#include <svm/Memory.hpp>

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

	void ConstantPool::Clear() noexcept {
		m_Pool.reset();
		m_IntCount = 0;
		m_LongCount = 0;
		m_DoubleCount = 0;
	}
	bool ConstantPool::IsEmpty() const noexcept {
		return m_Pool == nullptr;
	}

	std::uint32_t ConstantPool::GetIntOffset() const noexcept {
		return 0;
	}
	std::uint32_t ConstantPool::GetLongOffset() const noexcept {
		return GetIntCount() * sizeof(IntObject);
	}
	std::uint32_t ConstantPool::GetDoubleOffset() const noexcept {
		return GetLongOffset() + GetLongCount() * sizeof(LongObject);
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

	std::ostream& operator<<(std::ostream& stream, const ConstantPool& constantPool) {
		if (stream.iword(detail::ByteModeIndex()) == 1) {
			std::uint8_t bytes[12];
			if (GetEndian() == Endian::Little) {
				*reinterpret_cast<std::uint32_t*>(bytes) = constantPool.GetIntCount();
				*reinterpret_cast<std::uint32_t*>(bytes + 4) = constantPool.GetLongCount();
				*reinterpret_cast<std::uint32_t*>(bytes + 8) = constantPool.GetDoubleCount();
			} else {
				*reinterpret_cast<std::uint32_t*>(bytes) = ReverseEndian(constantPool.GetIntCount());
				*reinterpret_cast<std::uint32_t*>(bytes + 4) = ReverseEndian(constantPool.GetLongCount());
				*reinterpret_cast<std::uint32_t*>(bytes + 8) = ReverseEndian(constantPool.GetDoubleCount());
			}

			for (std::uint32_t i = 0; i < constantPool.GetIntCount(); ++i) {
				if (GetEndian() == Endian::Little) {
					*reinterpret_cast<std::uint32_t*>(bytes) = constantPool.GetConstant<IntObject>(i + constantPool.GetIntOffset()).Value;
				} else {
					*reinterpret_cast<std::uint32_t*>(bytes) = ReverseEndian(constantPool.GetConstant<IntObject>(i).Value);
				}
				stream.write(reinterpret_cast<const char*>(bytes), 4);
			}
			for (std::uint32_t i = 0; i < constantPool.GetLongCount(); ++i) {
				if (GetEndian() == Endian::Little) {
					*reinterpret_cast<std::uint64_t*>(bytes) = constantPool.GetConstant<LongObject>(i + constantPool.GetLongOffset()).Value;
				} else {
					*reinterpret_cast<std::uint64_t*>(bytes) = ReverseEndian(constantPool.GetConstant<LongObject>(i).Value);
				}
				stream.write(reinterpret_cast<const char*>(bytes), 8);
			}
			for (std::uint32_t i = 0; i < constantPool.GetDoubleCount(); ++i) {
				if (GetEndian() == Endian::Little) {
					*reinterpret_cast<double*>(bytes) = constantPool.GetConstant<DoubleObject>(i).Value;
				} else {
					*reinterpret_cast<double*>(bytes) = ReverseEndian(constantPool.GetConstant<DoubleObject>(i + constantPool.GetDoubleOffset()).Value);
				}
				stream.write(reinterpret_cast<const char*>(bytes), 8);
			}
		} else {
			bool isFirst = true;
			for (std::uint32_t i = 0; i < constantPool.GetIntCount(); ++i) {
				if (isFirst) {
					isFirst = false;
				} else {
					stream << '\n';
				}
				stream << '[' << i << "]: int(" << constantPool.GetConstant<IntObject>(i + constantPool.GetIntOffset()).Value << ')';
			}
			for (std::uint32_t i = 0; i < constantPool.GetLongCount(); ++i) {
				if (isFirst) {
					isFirst = false;
				} else {
					stream << '\n';
				}
				stream << '[' << i + constantPool.GetIntCount() << "]: long(" << constantPool.GetConstant<LongObject>(i + constantPool.GetLongOffset()).Value << ')';
			}
			for (std::uint32_t i = 0; i < constantPool.GetDoubleCount(); ++i) {
				if (isFirst) {
					isFirst = false;
				} else {
					stream << '\n';
				}
				stream << '[' << i + constantPool.GetIntCount() + constantPool.GetLongCount() << "]: double(" << constantPool.GetConstant<DoubleObject>(i + constantPool.GetDoubleOffset()).Value << ')';
			}
		}

		return stream;
	}
}