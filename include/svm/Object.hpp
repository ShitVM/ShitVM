#pragma once

namespace svm {
	enum class TypeCode {
		None,

		Int,
		Long,
		Double,
		Reference,
	};

	class Type final {
	public:
		TypeCode Code = TypeCode::None;
		const char* Name = nullptr;

	public:
		Type() noexcept = default;
		Type(TypeCode code, const char* name) noexcept;
		Type(const Type&) = delete;
		~Type() = default;

	public:
		Type& operator=(const Type&) = delete;
		bool operator==(const Type&) = delete;
		bool operator!=(const Type&) = delete;
	};

	class Object {
	private:
		const Type* m_Type = nullptr;

	public:
		Object() noexcept = default;
		Object(const Type* type) noexcept;
		Object(const Object& object) noexcept;
		~Object() = default;

	public:
		Object& operator=(const Object& object) noexcept;

	public:
		const Type* GetType() const noexcept;
		TypeCode GetTypeCode() const noexcept;
		bool IsInt() const noexcept;
		bool IsLong() const noexcept;
		bool IsDouble() const noexcept;
		bool IsReference() const noexcept;
	};
}