#pragma once

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

	class Type final {
	private:
		const TypeData* m_Data = nullptr;

	public:
		Type() noexcept = default;
		Type(std::nullptr_t) noexcept;
		Type(const TypeData& data) noexcept;
		Type(const Type& type) noexcept;
		~Type() noexcept = default;

	public:
		Type& operator=(const Type& type) noexcept;
		bool operator==(const Type& type) const noexcept;
		bool operator!=(const Type& type) const noexcept;
		const TypeData& operator*() const noexcept;
		const TypeData* operator->() const noexcept;

	public:
		bool IsValidType() const noexcept;
	};

	extern const Type IntType;
	extern const Type LongType;
	extern const Type DoubleType;
	extern const Type PointerType;
}