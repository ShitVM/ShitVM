#include <svm/Object.hpp>

namespace svm {
	Type::Type(TypeCode code, const char* name) noexcept
		: Code(code), Name(name) {}

	namespace {
		static const Type s_IntType(TypeCode::Int, "int");
		static const Type s_LongType(TypeCode::Long, "long");
		static const Type s_DoubleType(TypeCode::Double, "double");
		static const Type s_ReferenceType(TypeCode::Reference, "reference");
	}

	const Type* IntType = &s_IntType;
	const Type* LongType = &s_LongType;
	const Type* DoubleType = &s_DoubleType;
	const Type* ReferenceType = &s_ReferenceType;
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

namespace svm {
	IntObject::IntObject() noexcept
		: Object(IntType) {}
	IntObject::IntObject(std::uint32_t value) noexcept
		: Object(IntType), Value(value) {}
	IntObject::IntObject(const IntObject& object) noexcept
		: Object(object), Value(object.Value) {}

	IntObject& IntObject::operator=(const IntObject& object) noexcept {
		Object::operator=(object);

		Value = object.Value;
		return *this;
	}
}

namespace svm {
	LongObject::LongObject() noexcept
		: Object(LongType) {}
	LongObject::LongObject(std::uint64_t value) noexcept
		: Object(LongType), Value(value) {}
	LongObject::LongObject(const LongObject& object) noexcept
		: Object(object), Value(object.Value) {}

	LongObject& LongObject::operator=(const LongObject& object) noexcept {
		Object::operator=(object);

		Value = object.Value;
		return *this;
	}
}

namespace svm {
	DoubleObject::DoubleObject() noexcept
		: Object(LongType) {}
	DoubleObject::DoubleObject(double value) noexcept
		: Object(LongType), Value(value) {}
	DoubleObject::DoubleObject(const DoubleObject& object) noexcept
		: Object(object), Value(object.Value) {}

	DoubleObject& DoubleObject::operator=(const DoubleObject& object) noexcept {
		Object::operator=(object);

		Value = object.Value;
		return *this;
	}
}

namespace svm {
	ReferenceObject::ReferenceObject() noexcept
		: Object(LongType) {}
	ReferenceObject::ReferenceObject(Object* value) noexcept
		: Object(LongType), Value(value) {}
	ReferenceObject::ReferenceObject(const ReferenceObject& object) noexcept
		: Object(object), Value(object.Value) {}

	ReferenceObject& ReferenceObject::operator=(const ReferenceObject& object) noexcept {
		Object::operator=(object);

		Value = object.Value;
		return *this;
	}
}