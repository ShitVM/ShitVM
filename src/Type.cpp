#include <svm/Type.hpp>

#include <svm/Object.hpp>

namespace svm {
	TypeData::TypeData(TypeCode code, const char* name, unsigned size, unsigned dataSize) noexcept
		: Code(code), Name(name), Size(size), DataSize(dataSize) {}
}

namespace svm {
	bool Type::IsFundamentalType() const noexcept {
		return GetPointer() == IntType.GetPointer() ||
			GetPointer() == LongType.GetPointer() ||
			GetPointer() == DoubleType.GetPointer() ||
			GetPointer() == PointerType.GetPointer();
	}
	bool Type::IsStructure() const noexcept {
		return GetReference().Code >= TypeCode::Structure;
	}

	namespace {
		static const TypeData s_NoneType(TypeCode::None, "none", 0, 0);
		static const TypeData s_IntType(TypeCode::Int, "int", sizeof(IntObject), sizeof(std::uint32_t));
		static const TypeData s_LongType(TypeCode::Long, "long", sizeof(LongObject), sizeof(std::uint64_t));
		static const TypeData s_DoubleType(TypeCode::Double, "double", sizeof(DoubleObject), sizeof(double));
		static const TypeData s_PointerType(TypeCode::Pointer, "pointer", sizeof(PointerObject), sizeof(void*));
		static const TypeData s_StructureType(TypeCode::Structure, "structure", 0, 0);
	}

	const Type NoneType = s_NoneType;
	const Type IntType = s_IntType;
	const Type LongType = s_LongType;
	const Type DoubleType = s_DoubleType;
	const Type PointerType = s_PointerType;
	const Type StructureType = s_StructureType;

	Type GetTypeFromTypeCode(TypeCode code) noexcept {
		switch (code) {
		case TypeCode::Int: return IntType;
		case TypeCode::Long: return LongType;
		case TypeCode::Double: return DoubleType;
		case TypeCode::Pointer: return PointerType;

		default:
			if (code >= TypeCode::Structure) return StructureType;
			else return NoneType;
		}
	}
}