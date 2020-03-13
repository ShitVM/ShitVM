#include <svm/ConstantPool.hpp>

#include <svm/IO.hpp>

#include <cassert>
#include <utility>

namespace svm {
	ConstantPool::ConstantPool(std::vector<IntObject> intPool, std::vector<LongObject> longPool, std::vector<DoubleObject> doublePool) noexcept
		: m_IntPool(std::move(intPool)), m_LongPool(std::move(longPool)), m_DoublePool(std::move(doublePool)) {}
	ConstantPool::ConstantPool(ConstantPool&& pool) noexcept
		: m_IntPool(std::move(pool.m_IntPool)), m_LongPool(std::move(pool.m_LongPool)), m_DoublePool(std::move(pool.m_DoublePool)) {}

	ConstantPool& ConstantPool::operator=(ConstantPool&& pool) noexcept {
		m_IntPool = std::move(pool.m_IntPool);
		m_LongPool = std::move(pool.m_LongPool);
		m_DoublePool = std::move(pool.m_DoublePool);
		return *this;
	}

	void ConstantPool::Clear() noexcept {
		m_IntPool.clear();
		m_LongPool.clear();
		m_DoublePool.clear();
	}
	bool ConstantPool::IsEmpty() const noexcept {
		return m_IntPool.empty() && m_LongPool.empty() && m_DoublePool.empty();
	}

	Type ConstantPool::GetConstantType(std::uint32_t index) const noexcept {
		assert(index < GetAllCount());

		if (index >= GetDoubleOffset()) return DoubleType;
		else if (index >= GetLongOffset()) return LongType;
		else return IntType;
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
		return static_cast<std::uint32_t>(m_IntPool.size());
	}
	std::uint32_t ConstantPool::GetLongCount() const noexcept {
		return static_cast<std::uint32_t>(m_LongPool.size());
	}
	std::uint32_t ConstantPool::GetDoubleCount() const noexcept {
		return static_cast<std::uint32_t>(m_DoublePool.size());
	}
	std::uint32_t ConstantPool::GetAllCount() const noexcept {
		return GetIntCount() + GetLongCount() + GetDoubleCount();
	}

	const std::vector<IntObject>& ConstantPool::GetIntPool() const noexcept {
		return m_IntPool;
	}
	void ConstantPool::SetIntPool(std::vector<IntObject> newIntPool) noexcept {
		m_IntPool = std::move(newIntPool);
	}
	const std::vector<LongObject>& ConstantPool::GetLongPool() const noexcept {
		return m_LongPool;
	}
	void ConstantPool::SetLongPool(std::vector<LongObject> newLongPool) noexcept {
		m_LongPool = std::move(newLongPool);
	}
	const std::vector<DoubleObject>& ConstantPool::GetDoublePool() const noexcept {
		return m_DoublePool;
	}
	void ConstantPool::SetDoublePool(std::vector<DoubleObject> newDoublePool) noexcept {
		m_DoublePool = std::move(newDoublePool);
	}

	namespace {
		template<typename T>
		void PrintConstants(std::ostream& stream, const ConstantPool& constantPool, const std::string& defIndent, std::uint32_t i) {
			const auto& constant = constantPool.GetConstant<T>(i);
			stream << '\n' << defIndent << "\t[" << i << "]: " << constant.GetType()->Name << '(' << constant.Value << ')';
		}
	}

	std::ostream& operator<<(std::ostream& stream, const ConstantPool& constantPool) {
		const std::string defIndent = detail::MakeTabs(stream);

		stream << defIndent << "ConstantPool: " << constantPool.GetAllCount();

		static constexpr std::uint32_t((ConstantPool::*types[])() const noexcept) = {
			&ConstantPool::GetIntCount,
			&ConstantPool::GetLongCount,
			&ConstantPool::GetDoubleCount,
		};

		std::uint32_t i = 0;
		for (auto type : types) {
			const std::uint32_t count = (constantPool.*type)();
			const std::uint32_t end = i + count;

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