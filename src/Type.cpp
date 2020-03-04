#include <svm/Type.hpp>

namespace svm {
	TypeData::TypeData(TypeCode code, const char* name) noexcept
		: Code(code), Name(name) {}
}

namespace svm {
	bool Type::IsValidType() const noexcept {
		return GetPointer() == IntType.GetPointer() ||
			GetPointer() == LongType.GetPointer() ||
			GetPointer() == DoubleType.GetPointer() ||
			GetPointer() == PointerType.GetPointer();
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