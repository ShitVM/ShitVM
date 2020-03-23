#pragma once

#include <svm/GarbageCollector.hpp>
#include <svm/Object.hpp>
#include <svm/Type.hpp>

#include <cstdint>
#include <type_traits>
#include <variant>

namespace svm::detail {
	template<typename T, typename = void>
	struct MakeSigned;

	template<typename T>
	struct MakeSigned<T, std::enable_if_t<std::is_integral_v<T>>> final : std::make_signed<T> {};
	template<typename T>
	struct MakeSigned<T, std::enable_if_t<std::is_floating_point_v<T>>> final {
		using type = T;
	};

	template<typename T>
	using MakeSignedType = typename MakeSigned<T>::type;
}

namespace svm {
	namespace detail {
		struct VirtualNullObject final {};
	}

	extern const detail::VirtualNullObject VNull;

#define VNULL svm::VNull
}

namespace svm {
	class Loader;
	class VirtualStack;

	class VirtualObject final {
		friend class VirtualStack;

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
		ArrayObject* IsArray() const noexcept;
		StructureObject* IsStructure() const noexcept;
		std::uint32_t ToInt() const noexcept;
		std::uint64_t ToLong() const noexcept;
		double ToDouble() const noexcept;
		Object* ToPointer() const noexcept;
		ManagedHeapInfo* ToGCPointer() const noexcept;

		VirtualObject Field(const Loader& loader, std::uint32_t index) const noexcept;

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

		static bool MakeSameType(ObjectVariant& lhs, ObjectVariant& rhs) noexcept;
		static void ArithmeticPromotion(ObjectVariant& target) noexcept;
		static int Compare(const VirtualObject& lhs, const VirtualObject& rhs) noexcept;
		static int Compare(ObjectVariant lhs, ObjectVariant rhs) noexcept;
		template<typename T>
		static int Compare(T lhs, T rhs) noexcept;
		template<typename F>
		VirtualObject ArithmeticOperation(const VirtualObject& rhs, F&& function) const noexcept;
	};
}

#include "detail/impl/VirtualObject.hpp"