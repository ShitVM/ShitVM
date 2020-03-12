#pragma once

#include <svm/detail/ReferenceWrapper.hpp>

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

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
		std::string Name;
		std::size_t Size = 0;

	public:
		TypeInfo() noexcept = default;
		TypeInfo(TypeCode code, std::string name, std::size_t size) noexcept;
		TypeInfo(const TypeInfo&) = delete;
		~TypeInfo() = default;

	public:
		TypeInfo& operator=(const TypeInfo&) = delete;
		bool operator==(const TypeInfo&) = delete;
		bool operator!=(const TypeInfo&) = delete;
	};
}

namespace svm {
	class StructureInfo;
	class Structures;

	class Type final : public detail::ReferenceWrapper<TypeInfo> {
	public:
		using detail::ReferenceWrapper<TypeInfo>::ReferenceWrapper;

	public:
		bool IsFundamentalType() const noexcept;
		bool IsStructure() const noexcept;
		bool IsValidType() const noexcept;
	};

	extern const Type NoneType;
	extern const Type IntType;
	extern const Type LongType;
	extern const Type DoubleType;
	extern const Type PointerType;

	Type GetTypeFromTypeCode(const std::vector<StructureInfo>& structures, TypeCode code) noexcept;
	Type GetTypeFromTypeCode(const Structures& structures, TypeCode code) noexcept;
}