#include <svm/Type.hpp>

#include <svm/Object.hpp>
#include <svm/Structure.hpp>

#include <utility>

namespace svm {
	TypeInfo::TypeInfo(TypeCode code, std::string name, std::size_t size) noexcept
		: Code(code), Name(std::move(name)), Size(size) {}
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
	bool Type::IsValidType() const noexcept {
		return IsFundamentalType() || IsStructure();
	}

	namespace {
		static const TypeInfo s_NoneType(TypeCode::None, "none", 0);
		static const TypeInfo s_IntType(TypeCode::Int, "int", sizeof(IntObject));
		static const TypeInfo s_LongType(TypeCode::Long, "long", sizeof(LongObject));
		static const TypeInfo s_DoubleType(TypeCode::Double, "double", sizeof(DoubleObject));
		static const TypeInfo s_PointerType(TypeCode::Pointer, "pointer", sizeof(PointerObject));
	}

	const Type NoneType = s_NoneType;
	const Type IntType = s_IntType;
	const Type LongType = s_LongType;
	const Type DoubleType = s_DoubleType;
	const Type PointerType = s_PointerType;

	Type GetTypeFromTypeCode(const std::vector<StructureInfo>& structures, TypeCode code) noexcept {
		switch (code) {
		case TypeCode::Int: return IntType;
		case TypeCode::Long: return LongType;
		case TypeCode::Double: return DoubleType;
		case TypeCode::Pointer: return PointerType;

		default:
			if (code >= TypeCode::Structure) return structures[static_cast<std::uint8_t>(code) - static_cast<std::uint8_t>(TypeCode::Structure)].Type;
			else return NoneType;
		}
	}
}