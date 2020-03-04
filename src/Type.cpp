#include <svm/Type.hpp>

namespace svm {
	TypeData::TypeData(TypeCode code, const char* name) noexcept
		: Code(code), Name(name) {}
}

namespace svm {
	Type::Type(std::nullptr_t) noexcept {}
	Type::Type(const TypeData& data) noexcept
		: m_Data(&data) {}
	Type::Type(const Type& type) noexcept
		: m_Data(type.m_Data) {}

	Type& Type::operator=(const Type& type) noexcept {
		m_Data = type.m_Data;
		return *this;
	}

	bool Type::operator==(const Type& type) const noexcept {
		return m_Data == type.m_Data;
	}
	bool Type::operator!=(const Type& type) const noexcept {
		return m_Data != type.m_Data;
	}
	const TypeData& Type::operator*() const noexcept {
		return *m_Data;
	}
	const TypeData* Type::operator->() const noexcept {
		return m_Data;
	}

	bool Type::IsValidType() const noexcept {
		return m_Data == IntType.m_Data ||
			m_Data == LongType.m_Data ||
			m_Data == DoubleType.m_Data ||
			m_Data == PointerType.m_Data;
	}

	namespace {
		static const TypeData s_IntType(TypeCode::Int, "int");
		static const TypeData s_LongType(TypeCode::Long, "long");
		static const TypeData s_DoubleType(TypeCode::Double, "double");
		static const TypeData s_PointerType(TypeCode::Pointer, "pointer");
	}

	const Type IntType = s_IntType;
	const Type LongType = s_LongType;
	const Type DoubleType = s_DoubleType;
	const Type PointerType = s_PointerType;
}