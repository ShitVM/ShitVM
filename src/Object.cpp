#include <svm/Object.hpp>

namespace svm {
	Type::Type(TypeCode code, const char* name) noexcept
		: Code(code), Name(name) {}
}

namespace svm {
	Object::Object(const Type* type) noexcept
		: m_Type(type) {}
	Object::Object(const Object& object) noexcept
		: m_Type(object.m_Type) {}

	Object& Object::operator=(const Object& object) noexcept {
		m_Type = object.m_Type;
		return *this;
	}

	const Type* Object::GetType() const noexcept {
		return m_Type;
	}
	TypeCode Object::GetTypeCode() const noexcept {
		return m_Type ? m_Type->Code : TypeCode::None;
	}
	bool Object::IsInt() const noexcept {
		return GetTypeCode() == TypeCode::Int;
	}
	bool Object::IsLong() const noexcept {
		return GetTypeCode() == TypeCode::Long;
	}
	bool Object::IsDouble() const noexcept {
		return GetTypeCode() == TypeCode::Double;
	}
	bool Object::IsReference() const noexcept {
		return GetTypeCode() == TypeCode::Reference;
	}
}