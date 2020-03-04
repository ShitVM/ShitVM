#pragma once

#include <cstdint>

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

namespace svm {
	class Object {
	private:
		Type m_Type;

	public:
		Object() noexcept = default;
		~Object() = default;

	protected:
		Object(Type type) noexcept;
		Object(const Object& object) noexcept;

	protected:
		Object& operator=(const Object& object) noexcept;

	public:
		Type GetType() const noexcept;
		bool IsInt() const noexcept;
		bool IsLong() const noexcept;
		bool IsDouble() const noexcept;
		bool IsPointer() const noexcept;
	};

	class IntObject final : public Object {
	public:
		std::uint32_t Value = 0;

	public:
		IntObject() noexcept;
		IntObject(std::uint32_t value) noexcept;
		IntObject(const IntObject& object) noexcept;
		~IntObject() = default;

	public:
		IntObject& operator=(const IntObject& object) noexcept;
	};

	class LongObject final : public Object {
	public:
		std::uint64_t Value = 0;

	public:
		LongObject() noexcept;
		LongObject(std::uint64_t value) noexcept;
		LongObject(const LongObject& object) noexcept;
		~LongObject() = default;

	public:
		LongObject& operator=(const LongObject& object) noexcept;
	};

	class DoubleObject final : public Object {
	public:
		double Value = 0.0;

	public:
		DoubleObject() noexcept;
		DoubleObject(double value) noexcept;
		DoubleObject(const DoubleObject& object) noexcept;
		~DoubleObject() = default;

	public:
		DoubleObject& operator=(const DoubleObject& object) noexcept;
	};

	class PointerObject final : public Object {
	public:
		void* Value = nullptr;

	public:
		PointerObject() noexcept;
		PointerObject(void* value) noexcept;
		PointerObject(const PointerObject& object) noexcept;
		~PointerObject() = default;

	public:
		PointerObject& operator=(const PointerObject& object) noexcept;
	};
}