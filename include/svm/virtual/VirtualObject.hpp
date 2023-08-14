#pragma once

#include <svm/GarbageCollector.hpp>
#include <svm/Object.hpp>
#include <svm/Type.hpp>

#include <cstdint>
#include <utility>
#include <variant>

namespace svm {
	namespace detail {
		struct VirtualNullObject final {};
	}

	extern const detail::VirtualNullObject VNull;

#define VNULL ::svm::VNull
}

namespace svm {
	class VirtualContext;
	class VirtualStack;

	class VirtualObject final {
		friend class VirtualContext;
		friend class VirtualStack;

	public:
		enum class PointerTarget : std::uintptr_t {
			Null,
		};
		enum class GCPointerTarget : std::uintptr_t {
			Null,
		};

#define VPNULL ::svm::VirtualObject::PointerTarget::Null
#define VGPNULL ::svm::VirtualObject::GCPointerTarget::Null

	private:
		std::variant<std::monostate, Object*, ManagedHeapInfo*> m_Object;

	public:
		VirtualObject() noexcept = default;
		VirtualObject(detail::VirtualNullObject) noexcept;
		VirtualObject(const VirtualObject& object) noexcept;
		~VirtualObject() = default;

	private:
		VirtualObject(Object* reference) noexcept;
		VirtualObject(ManagedHeapInfo* reference) noexcept;

	public:
		VirtualObject& operator=(detail::VirtualNullObject) noexcept;
		VirtualObject& operator=(const VirtualObject& object) noexcept;
		bool operator==(const VirtualObject& object) const noexcept;
		bool operator!=(const VirtualObject& object) const noexcept;

	public:
		Type GetType() const noexcept;
		bool IsEmpty() const noexcept;
		bool IsInt() const noexcept;
		bool IsLong() const noexcept;
		bool IsDouble() const noexcept;
		bool IsPointer() const noexcept;
		bool IsGCPointer() const noexcept;
		Type IsArray() const noexcept;
		Type IsStructure() const noexcept;

		std::uint32_t ToInt() const noexcept;
		std::uint64_t ToLong() const noexcept;
		double ToDouble() const noexcept;
		PointerTarget ToPointer() const noexcept;
		GCPointerTarget ToGCPointer() const noexcept;

		void SetInt(std::uint32_t value) noexcept;
		void SetLong(std::uint64_t value) noexcept;
		void SetDouble(double value) noexcept;
		void SetPointer(PointerTarget value) noexcept;
		void SetPointer(GCPointerTarget value) noexcept;
		void SetGCPointer(GCPointerTarget value) noexcept;

		std::uint64_t GetCount() const noexcept;

	private:
		Object* GetObjectPtr() const noexcept;
		template<typename T>
		decltype(std::declval<T>().Value)& GetValue() const noexcept;
	};
}

#include "detail/impl/VirtualObject.hpp"