#pragma once

#include <svm/detail/ReferenceWrapper.hpp>

#include <cstdint>

namespace svm {
	enum class TypeCode : std::uint32_t {
		None,

		Byte,				// Not supported
		Short,				// Not supported
		Int,
		Long,
		Float,				// Not supported
		Double,
		Pointer,
		GCPointer,			// Not supported
		Array,				// Not supported
		Structure,
	};

	class TypeInfo final {
	public:
		TypeCode Code = TypeCode::None;
		const char* Name = nullptr;
		unsigned Size = 0;
		unsigned DataSize = 0;

	public:
		TypeInfo() noexcept = default;
		TypeInfo(TypeCode code, const char* name, unsigned size, unsigned dataSize) noexcept;
		TypeInfo(const TypeInfo&) = delete;
		~TypeInfo() = default;

	public:
		TypeInfo& operator=(const TypeInfo&) = delete;
		bool operator==(const TypeInfo&) = delete;
		bool operator!=(const TypeInfo&) = delete;
	};
}

namespace svm {
	class Type final : public detail::ReferenceWrapper<TypeInfo> {
	public:
		using detail::ReferenceWrapper<TypeInfo>::ReferenceWrapper;

	public:
		bool IsFundamentalType() const noexcept;
		bool IsStructure() const noexcept;
	};

	extern const Type NoneType;
	extern const Type IntType;
	extern const Type LongType;
	extern const Type DoubleType;
	extern const Type PointerType;
	extern const Type StructureType;

	Type GetTypeFromTypeCode(TypeCode code) noexcept;
}