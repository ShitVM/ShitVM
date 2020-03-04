#pragma once

#include <svm/detail/ReferenceWrapper.hpp>

namespace svm {
	enum class TypeCode {
		None,

		Int,
		Long,
		Double,
		Pointer,
	};

	class TypeData final {
	public:
		TypeCode Code = TypeCode::None;
		const char* Name = nullptr;

	public:
		TypeData() noexcept = default;
		TypeData(TypeCode code, const char* name) noexcept;
		TypeData(const TypeData&) = delete;
		~TypeData() = default;

	public:
		TypeData& operator=(const TypeData&) = delete;
		bool operator==(const TypeData&) = delete;
		bool operator!=(const TypeData&) = delete;
	};

	class Type final : public detail::ReferenceWrapper<TypeData> {
	public:
		using detail::ReferenceWrapper<TypeData>::ReferenceWrapper;

	public:
		bool IsValidType() const noexcept;
	};

	extern const Type IntType;
	extern const Type LongType;
	extern const Type DoubleType;
	extern const Type PointerType;
}