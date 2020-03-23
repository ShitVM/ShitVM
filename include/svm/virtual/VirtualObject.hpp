#pragma once

#include <svm/GarbageCollector.hpp>
#include <svm/Object.hpp>
#include <svm/Type.hpp>

#include <cstdint>
#include <variant>

namespace svm {
	namespace detail {
		struct VirtualNullObject final {};
	}

	extern const detail::VirtualNullObject VNull;

#define VNULL svm::VNull
}

namespace svm {
	class VirtualObject final {
	private:
		using ObjectVariant = std::variant<
			IntObject, LongObject,
			DoubleObject,
			PointerObject, GCPointerObject>;
		using Variant = std::variant<std::monostate,
			ObjectVariant,
			Object*, ManagedHeapInfo*>;

	private:
		Variant m_Object;

	public:
		VirtualObject() noexcept = default;
		VirtualObject(detail::VirtualNullObject) noexcept;
		VirtualObject(std::int32_t value) noexcept;
		VirtualObject(std::uint32_t value) noexcept;
		VirtualObject(std::int64_t value) noexcept;
		VirtualObject(std::uint64_t value) noexcept;
		VirtualObject(double value) noexcept;
		VirtualObject(const VirtualObject& object) noexcept;
		~VirtualObject() = default;

	private:
		VirtualObject(PointerObject pointer) noexcept;
		VirtualObject(GCPointerObject pointer) noexcept;
		VirtualObject(Object* reference) noexcept;
		VirtualObject(ManagedHeapInfo* reference) noexcept;

	public:
		VirtualObject& operator=(detail::VirtualNullObject) noexcept;
		VirtualObject& operator=(const VirtualObject& object) noexcept;
		bool operator==(const VirtualObject& object) const noexcept;
		bool operator!=(const VirtualObject& object) const noexcept;
		bool operator>(const VirtualObject& object) const noexcept;
		bool operator>=(const VirtualObject& object) const noexcept;
		bool operator<(const VirtualObject& object) const noexcept;
		bool operator<=(const VirtualObject& object) const noexcept;
		VirtualObject operator+(const VirtualObject& rhs) const noexcept;
		VirtualObject operator-(const VirtualObject& rhs) const noexcept;
		VirtualObject operator*(const VirtualObject& rhs) const noexcept;
		VirtualObject operator/(const VirtualObject& rhs) const noexcept;
		VirtualObject operator%(const VirtualObject& rhs) const noexcept;
		VirtualObject operator+() const noexcept;
		VirtualObject operator-() const noexcept;
		VirtualObject operator&() const noexcept;
		VirtualObject operator*() const noexcept;
		VirtualObject operator[](std::uint64_t index) const noexcept;

	public:
		Type GetType() const noexcept;
		bool IsEmpty() const noexcept;
		bool IsInt() const noexcept;
		bool IsLong() const noexcept;
		bool IsDouble() const noexcept;
		bool IsPointer() const noexcept;
		bool IsGCPointer() const noexcept;
		bool IsArray() const noexcept;
		bool IsStructure() const noexcept;
		std::uint32_t ToInt() const noexcept;
		std::uint64_t ToLong() const noexcept;
		double ToDouble() const noexcept;
		Object* ToPointer() const noexcept;
		ManagedHeapInfo* ToGCPointer() const noexcept;

	private:
		template<typename T, typename F>
		T Native(F&& function) const noexcept;
		template<typename T>
		T NativeArithmetic() const noexcept;

	private:
		static const ObjectVariant& GetObject(const ObjectVariant& object) noexcept;
		static ObjectVariant& GetObject(ObjectVariant& object) noexcept;
		static ObjectVariant GetObject(Object* reference) noexcept;
		static ObjectVariant GetObject(ManagedHeapInfo* reference) noexcept;
	};
}

#include "detail/impl/VirtualObject.hpp"