#include <svm/ConstantPool.hpp>

#include <svm/IO.hpp>
#include <svm/Macro.hpp>
#include <svm/Memory.hpp>

#include <utility>

namespace svm {
	ConstantPool::ConstantPool(std::vector<std::uint8_t> pool, std::uint32_t intCount, std::uint32_t longCount, std::uint32_t doubleCount) noexcept
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
		m_Pool.clear();
		m_IntCount = 0;
		m_LongCount = 0;
		m_DoubleCount = 0;
	}
	bool ConstantPool::IsEmpty() const noexcept {
		return m_Pool.empty();
	}

	const Type* ConstantPool::GetConstantType(std::uint32_t index) const noexcept {
		if (index >= GetDoubleOffset()) {
			return DoubleType;
		} else if (index >= GetLongOffset()) {
			return LongType;
		} else {
			return IntType;
		}
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

	namespace {
		template<typename T>
		SVM_INLINE void PrintConstants(std::ostream& stream, const ConstantPool& constantPool, const std::string& defIndent, std::uint32_t i) {
			const auto& constant = constantPool.GetConstant<T>(i);
			stream << '\n' << defIndent << "\t[" << i << "]: " << constant.GetType()->Name << '(' << constant.Value << ')';
		}
	}

	std::ostream& operator<<(std::ostream& stream, const ConstantPool& constantPool) {
		const std::string defIndent = detail::MakeTabs(stream);

		stream << defIndent << "ConstantPool:";

		static constexpr std::uint32_t((ConstantPool::*types[])() const noexcept) = {
			&ConstantPool::GetIntCount,
			&ConstantPool::GetLongCount,
			&ConstantPool::GetDoubleCount,
		};

		std::uint32_t i = 0;
		std::uint32_t offset = 0;
		for (auto type : types) {
			const std::uint32_t count = (constantPool.*type)();
			const std::uint32_t end = offset + count;
			for (; i < end; ++i) {
				if (type == &ConstantPool::GetIntCount) {
					PrintConstants<IntObject>(stream, constantPool, defIndent, i);
				} else if (type == &ConstantPool::GetLongCount) {
					PrintConstants<LongObject>(stream, constantPool, defIndent, i);
				} else if (type == &ConstantPool::GetDoubleCount) {
					PrintConstants<DoubleObject>(stream, constantPool, defIndent, i);
				}
			}
		}

		return stream;
	}
}