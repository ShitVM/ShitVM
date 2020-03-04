#include <svm/Object.hpp>

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

namespace svm {
	Object::Object(Type type) noexcept
		: m_Type(type) {}
	Object::Object(const Object& object) noexcept
		: m_Type(object.m_Type) {}

	Object& Object::operator=(const Object& object) noexcept {
		m_Type = object.m_Type;
		return *this;
	}

	Type Object::GetType() const noexcept {
		return m_Type;
	}
	bool Object::IsInt() const noexcept {
		return m_Type == IntType;
	}
	bool Object::IsLong() const noexcept {
		return m_Type == LongType;
	}
	bool Object::IsDouble() const noexcept {
		return m_Type == DoubleType;
	}
	bool Object::IsPointer() const noexcept {
		return m_Type == PointerType;
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
		: Object(DoubleType) {}
	DoubleObject::DoubleObject(double value) noexcept
		: Object(DoubleType), Value(value) {}
	DoubleObject::DoubleObject(const DoubleObject& object) noexcept
		: Object(object), Value(object.Value) {}

	DoubleObject& DoubleObject::operator=(const DoubleObject& object) noexcept {
		Object::operator=(object);

		Value = object.Value;
		return *this;
	}
}

namespace svm {
	PointerObject::PointerObject() noexcept
		: Object(PointerType) {}
	PointerObject::PointerObject(void* value) noexcept
		: Object(PointerType), Value(value) {}
	PointerObject::PointerObject(const PointerObject& object) noexcept
		: Object(object), Value(object.Value) {}

	PointerObject& PointerObject::operator=(const PointerObject& object) noexcept {
		Object::operator=(object);

		Value = object.Value;
		return *this;
	}
}