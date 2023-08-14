#include <svm/virtual/VirtualObject.hpp>

#include <cassert>

namespace svm {
	const detail::VirtualNullObject VNull;
}

namespace svm {
	VirtualObject::VirtualObject(detail::VirtualNullObject) noexcept {}
	VirtualObject::VirtualObject(const VirtualObject& object) noexcept
		: m_Object(object.m_Object) {}

	VirtualObject::VirtualObject(Object* reference) noexcept
		: m_Object(reference) {}
	VirtualObject::VirtualObject(ManagedHeapInfo* reference) noexcept
		: m_Object(reference) {}

	VirtualObject& VirtualObject::operator=(detail::VirtualNullObject) noexcept {
		return m_Object = std::monostate(), *this;
	}
	VirtualObject& VirtualObject::operator=(const VirtualObject& object) noexcept {
		return m_Object = object.m_Object, *this;
	}
	bool VirtualObject::operator==(const VirtualObject& object) const noexcept {
		return m_Object == object.m_Object;
	}
	bool VirtualObject::operator!=(const VirtualObject& object) const noexcept {
		return m_Object != object.m_Object;
	}

	Type VirtualObject::GetType() const noexcept {
		if (std::holds_alternative<std::monostate>(m_Object)) return NoneType;
		else return GetObjectPtr()->GetType();
	}
	bool VirtualObject::IsEmpty() const noexcept {
		return GetType() == NoneType;
	}
	bool VirtualObject::IsInt() const noexcept {
		return GetType() == IntType;
	}
	bool VirtualObject::IsLong() const noexcept {
		return GetType() == LongType;
	}
	bool VirtualObject::IsDouble() const noexcept {
		return GetType() == DoubleType;
	}
	bool VirtualObject::IsPointer() const noexcept {
		return GetType() == PointerType;
	}
	bool VirtualObject::IsGCPointer() const noexcept {
		return GetType() == GCPointerType;
	}
	Type VirtualObject::IsArray() const noexcept {
		if (!GetType().IsArray()) return nullptr;

		return (static_cast<ArrayObject*>(GetObjectPtr()) + 1)->GetType();
	}
	Type VirtualObject::IsStructure() const noexcept {
		if (const Type type = GetType(); type.IsStructure()) return type;
		else return nullptr;
	}

	std::uint32_t VirtualObject::ToInt() const noexcept {
		assert(IsInt());

		return GetValue<IntObject>();
	}
	std::uint64_t VirtualObject::ToLong() const noexcept {
		assert(IsLong());

		return GetValue<LongObject>();
	}
	double VirtualObject::ToDouble() const noexcept {
		assert(IsDouble());

		return GetValue<DoubleObject>();
	}
	VirtualObject::PointerTarget VirtualObject::ToPointer() const noexcept {
		assert(IsPointer());

		return static_cast<PointerTarget>(reinterpret_cast<std::uintptr_t>(GetValue<PointerObject>()));
	}
	VirtualObject::GCPointerTarget VirtualObject::ToGCPointer() const noexcept {
		assert(IsGCPointer());

		return static_cast<GCPointerTarget>(reinterpret_cast<std::uintptr_t>(GetValue<GCPointerObject>()));
	}

	void VirtualObject::SetInt(std::uint32_t value) noexcept {
		assert(IsInt());

		GetValue<IntObject>() = value;
	}
	void VirtualObject::SetLong(std::uint64_t value) noexcept {
		assert(IsLong());

		GetValue<LongObject>() = value;
	}
	void VirtualObject::SetDouble(double value) noexcept {
		assert(IsDouble());

		GetValue<DoubleObject>() = value;
	}
	void VirtualObject::SetPointer(PointerTarget value) noexcept {
		assert(IsPointer());

		GetValue<PointerObject>() = reinterpret_cast<Object*>(static_cast<std::uintptr_t>(value));
	}
	void VirtualObject::SetPointer(GCPointerTarget value) noexcept {
		assert(IsPointer());

		GetValue<PointerObject>() = reinterpret_cast<ManagedHeapInfo*>(static_cast<std::uintptr_t>(value)) + 1;
	}
	void VirtualObject::SetGCPointer(GCPointerTarget value) noexcept {
		assert(IsGCPointer());

		GetValue<GCPointerObject>() = reinterpret_cast<ManagedHeapInfo*>(static_cast<std::uintptr_t>(value));
	}

	std::uint64_t VirtualObject::GetCount() const noexcept {
		assert(IsArray() != nullptr);

		return static_cast<std::uint64_t>(static_cast<ArrayObject*>(GetObjectPtr())->Count);
	}

	Object* VirtualObject::GetObjectPtr() const noexcept {
		assert(!std::holds_alternative<std::monostate>(m_Object));

		if (std::holds_alternative<Object*>(m_Object)) return std::get<Object*>(m_Object);
		else if (std::holds_alternative<ManagedHeapInfo*>(m_Object)) return reinterpret_cast<Object*>(std::get<ManagedHeapInfo*>(m_Object) + 1);
	}
}