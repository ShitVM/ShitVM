#include <svm/virtual/VirtualObject.hpp>

#include <cassert>

namespace svm {
	const detail::VirtualNullObject VNull;
}

namespace svm {
	VirtualObject::VirtualObject(detail::VirtualNullObject) noexcept {}
	VirtualObject::VirtualObject(std::int32_t value) noexcept
		: m_Object(IntObject(value)) {}
	VirtualObject::VirtualObject(std::uint32_t value) noexcept
		: m_Object(IntObject(value)) {}
	VirtualObject::VirtualObject(std::int64_t value) noexcept
		: m_Object(LongObject(value)) {}
	VirtualObject::VirtualObject(std::uint64_t value) noexcept
		: m_Object(LongObject(value)) {}
	VirtualObject::VirtualObject(double value) noexcept
		: m_Object(DoubleObject(value)) {}
	VirtualObject::VirtualObject(const VirtualObject& object) noexcept {
		*this = object;
	}

	VirtualObject::VirtualObject(PointerObject pointer) noexcept
		: m_Object(pointer) {}
	VirtualObject::VirtualObject(GCPointerObject pointer) noexcept
		: m_Object(pointer) {}
	VirtualObject::VirtualObject(Object* reference) noexcept
		: m_Object(reference) {}
	VirtualObject::VirtualObject(ManagedHeapInfo* reference) noexcept
		: m_Object(reference) {}

	VirtualObject& VirtualObject::operator=(detail::VirtualNullObject) noexcept {
		return m_Object = std::monostate(), *this;
	}
	VirtualObject& VirtualObject::operator=(const VirtualObject& object) noexcept {
		std::visit(detail::Overload{
			[this](std::monostate) {
				m_Object = std::monostate();
			},
			[this](const auto& object) {
				m_Object = GetObject(object);
			}
		}, object.m_Object);
		return *this;
	}

	VirtualObject VirtualObject::operator&() const noexcept {
		if (std::holds_alternative<std::monostate>(m_Object)) return VNULL;
		else if (std::holds_alternative<Object*>(m_Object)) return PointerObject(std::get<Object*>(m_Object));
		else if (std::holds_alternative<ManagedHeapInfo*>(m_Object)) return GCPointerObject(std::get<ManagedHeapInfo*>(m_Object));
		else return std::visit([](const auto& object) noexcept -> PointerObject {
			return const_cast<std::decay_t<decltype(object)>*>(&object);
		}, std::get<ObjectVariant>(m_Object));
	}
	VirtualObject VirtualObject::operator*() const noexcept {
		if (std::holds_alternative<std::monostate>(m_Object)) return VNULL;
		else return std::visit(detail::Overload{
			[](const PointerObject& object) noexcept -> VirtualObject {
				return static_cast<Object*>(object.Value);
			},
			[](const GCPointerObject& object) noexcept -> VirtualObject {
				return static_cast<ManagedHeapInfo*>(object.Value);
			},
			[](const auto&) noexcept -> VirtualObject {
				return VNULL;
			}
		}, std::visit(detail::Overload{
				[](std::monostate) noexcept -> ObjectVariant {
					return IntObject();
				},
				[](const auto& object) noexcept {
					return GetObject(object);
				}
			}, m_Object));
	}

	Type VirtualObject::GetType() const noexcept {
		if (std::holds_alternative<std::monostate>(m_Object)) return NoneType;
		else if (std::holds_alternative<Object*>(m_Object)) return std::get<Object*>(m_Object)->GetType();
		else if (std::holds_alternative<ManagedHeapInfo*>(m_Object)) return reinterpret_cast<Object*>(std::get<ManagedHeapInfo*>(m_Object) + 1)->GetType();
		else {
			const ObjectVariant& object = std::get<ObjectVariant>(m_Object);
			switch (object.index()) {
			case 0: return IntType;
			case 1: return LongType;
			case 2: return DoubleType;
			case 3: return PointerType;
			case 4: return GCPointerType;
			}
		}
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

	std::uint32_t VirtualObject::ToInt() const noexcept {
		return NativeArithmetic<std::uint32_t>();
	}
	std::uint64_t VirtualObject::ToLong() const noexcept {
		return NativeArithmetic<std::uint64_t>();
	}
	double VirtualObject::ToDouble() const noexcept {
		return NativeArithmetic<double>();
	}
	Object* VirtualObject::ToPointer() const noexcept {
		return Native<Object*>(detail::Overload{
			[](const PointerObject& pointer) noexcept {
				return static_cast<Object*>(pointer.Value);
			},
			[](const GCPointerObject& pointer) noexcept {
				return reinterpret_cast<Object*>(static_cast<ManagedHeapInfo*>(pointer.Value) + 1);
			},
			[](const auto& object) noexcept {
				return reinterpret_cast<Object*>(static_cast<std::uintptr_t>(object.Value));
			}
		});
	}
	ManagedHeapInfo* VirtualObject::ToGCPointer() const noexcept {
		return Native<ManagedHeapInfo*>(detail::Overload{
			[](const PointerObject& pointer) noexcept {
				return reinterpret_cast<ManagedHeapInfo*>(static_cast<ManagedHeapInfo*>(pointer.Value) - 1);
			},
			[](const GCPointerObject& pointer) noexcept {
				return static_cast<ManagedHeapInfo*>(pointer.Value);
			},
			[](const auto& object) noexcept {
				return reinterpret_cast<ManagedHeapInfo*>(static_cast<std::uintptr_t>(object.Value));
			}
		});
	}

	const VirtualObject::ObjectVariant& VirtualObject::GetObject(const ObjectVariant& object) noexcept {
		return object;
	}
	VirtualObject::ObjectVariant& VirtualObject::GetObject(ObjectVariant& object) noexcept {
		return object;
	}
	VirtualObject::ObjectVariant VirtualObject::GetObject(Object* reference) noexcept {
		const Type type = reference->GetType();

		if (type == IntType) return *static_cast<IntObject*>(reference);
		else if (type == LongType) return *static_cast<LongObject*>(reference);
		else if (type == DoubleType) return *static_cast<DoubleObject*>(reference);
		else if (type == PointerType) return *static_cast<PointerObject*>(reference);
		else if (type == GCPointerType) return *static_cast<GCPointerObject*>(reference);
	}
	VirtualObject::ObjectVariant VirtualObject::GetObject(ManagedHeapInfo* reference) noexcept {
		return GetObject(reinterpret_cast<Object*>(reference + 1));
	}
}