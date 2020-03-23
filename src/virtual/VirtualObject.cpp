#include <svm/virtual/VirtualObject.hpp>

#include <svm/Loader.hpp>
#include <svm/Structure.hpp>

#include <cassert>
#include <cmath>

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
			[this](std::monostate) noexcept {
				m_Object = std::monostate();
			},
			[this](const auto& object) noexcept {
				m_Object = GetObject(object);
			}
		}, object.m_Object);
		return *this;
	}
	bool VirtualObject::operator==(const VirtualObject& object) const noexcept {
		return Compare(*this, object) == 0;
	}
	bool VirtualObject::operator!=(const VirtualObject& object) const noexcept {
		return Compare(*this, object) != 0;
	}
	bool VirtualObject::operator>(const VirtualObject& object) const noexcept {
		return Compare(*this, object) == 1;
	}
	bool VirtualObject::operator>=(const VirtualObject& object) const noexcept {
		return Compare(*this, object) != -1;
	}
	bool VirtualObject::operator<(const VirtualObject& object) const noexcept {
		return Compare(*this, object) == -1;
	}
	bool VirtualObject::operator<=(const VirtualObject& object) const noexcept {
		return Compare(*this, object) != 1;
	}
	VirtualObject VirtualObject::operator+(const VirtualObject& rhs) const noexcept {
		return ArithmeticOperation(rhs, [](auto&& lhs, auto&& rhs) {
			return lhs + rhs;
		});
	}
	VirtualObject VirtualObject::operator-(const VirtualObject& rhs) const noexcept {
		return ArithmeticOperation(rhs, [](auto&& lhs, auto&& rhs) {
			return lhs - rhs;
		});
	}
	VirtualObject VirtualObject::operator*(const VirtualObject& rhs) const noexcept {
		return ArithmeticOperation(rhs, [](auto&& lhs, auto&& rhs) {
			return lhs * rhs;
		});
	}
	VirtualObject VirtualObject::operator/(const VirtualObject& rhs) const noexcept {
		return ArithmeticOperation(rhs, [](auto&& lhs, auto&& rhs) {
			return lhs / rhs;
		});
	}
	VirtualObject VirtualObject::operator%(const VirtualObject& rhs) const noexcept {
		return ArithmeticOperation(rhs, detail::Overload{
			[](double lhs, double rhs) {
				return std::fmod(lhs, rhs);
			},
			[](auto&& lhs, auto&& rhs) {
				return lhs % rhs;
			}
		});
	}
	VirtualObject VirtualObject::operator+() const noexcept {
		return VirtualObject(*this);
	}
	VirtualObject VirtualObject::operator-() const noexcept {
		if (IsEmpty()) return VNULL;

		return std::visit(detail::Overload{
			[](const PointerObject& object) noexcept -> VirtualObject { return object; },
			[](const GCPointerObject& object) noexcept -> VirtualObject { return object; },
			[](const auto& object) noexcept -> VirtualObject {
				return -static_cast<detail::MakeSignedType<decltype(object.Value)>>(object.Value);
			}
		}, std::visit(detail::Overload{
			[](std::monostate) noexcept -> ObjectVariant {
				return IntObject();
			},
			[](const auto& object) noexcept -> ObjectVariant {
				return GetObject(object);
			}
		}, m_Object));
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
	VirtualObject VirtualObject::operator[](std::uint64_t index) const noexcept {
		ArrayObject* const array = IsArray();
		if (!array) return VNULL;
		else if (index >= array->Count) return VNULL;
		
		Object* const firstElement = static_cast<Object*>(array + 1);
		return reinterpret_cast<Object*>(reinterpret_cast<std::uint8_t*>(firstElement) + index * firstElement->GetType()->Size);
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
	ArrayObject* VirtualObject::IsArray() const noexcept {
		if (!IsPointer() && !IsGCPointer()) return nullptr;

		Object* const array = ToPointer();
		if (array->GetType().IsArray()) return static_cast<ArrayObject*>(array);
		else return nullptr;
	}
	StructureObject* VirtualObject::IsStructure() const noexcept {
		if (!IsPointer() && !IsGCPointer()) return nullptr;

		Object* const structure = ToPointer();
		if (structure->GetType().IsStructure()) return static_cast<StructureObject*>(structure);
		else return nullptr;
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

	VirtualObject VirtualObject::Field(const Loader& loader, std::uint32_t index) const noexcept {
		StructureObject* const structure = IsStructure();
		if (!structure) return VNULL;

		// TODO
		return VNULL;
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

	bool VirtualObject::MakeSameType(ObjectVariant& lhs, ObjectVariant& rhs) noexcept {
		if (lhs.index() == rhs.index()) return true;
		else if (lhs.index() >= 3 && rhs.index() < 3 ||
				 lhs.index() < 3 && rhs.index() >= 3) return false;

		if (lhs.index() < 3) {
			ObjectVariant& target = (lhs.index() > rhs.index() ? rhs : lhs);
			ObjectVariant& other = (lhs.index() > rhs.index() ? lhs : rhs);
			
			while (target.index() != other.index()) {
				ArithmeticPromotion(target);
			}
		} else {
			ObjectVariant& gcPtr = (lhs.index() == 4 ? lhs : rhs);
			gcPtr = PointerObject(static_cast<ManagedHeapInfo*>(std::get<PointerObject>(gcPtr).Value) + 1);
		}

		return true;
	}
	void VirtualObject::ArithmeticPromotion(ObjectVariant& target) noexcept {
		switch (target.index()) {
		case 0: target = LongObject(std::get<IntObject>(target).Value); break;
		case 1: target = DoubleObject(static_cast<double>(std::get<LongObject>(target).Value)); break;
		}
	}
	int VirtualObject::Compare(const VirtualObject& lhs, const VirtualObject& rhs) noexcept {
		if (lhs.IsEmpty() && rhs.IsEmpty()) return 0;
		else if (lhs.IsEmpty() || rhs.IsEmpty()) return 2;

		ObjectVariant lhsObject = std::visit(detail::Overload{
			[](std::monostate) noexcept -> ObjectVariant {
				return IntObject();
			},
			[](const auto& object) noexcept -> ObjectVariant {
				return GetObject(object);
			}
		}, lhs.m_Object);
		ObjectVariant rhsObject = std::visit(detail::Overload{
			[](std::monostate) noexcept -> ObjectVariant {
				return IntObject();
			},
			[](const auto& object) noexcept -> ObjectVariant {
				return GetObject(object);
			}
		}, rhs.m_Object);
		if (!MakeSameType(lhsObject, rhsObject)) return -2;
		else return Compare(lhsObject, rhsObject);
	}
	int VirtualObject::Compare(ObjectVariant lhs, ObjectVariant rhs) noexcept {
		switch (lhs.index()) {
		case 0: return Compare<std::uint32_t>(std::get<IntObject>(lhs).Value, std::get<IntObject>(rhs).Value);
		case 1: return Compare<std::uint64_t>(std::get<LongObject>(lhs).Value, std::get<LongObject>(rhs).Value);
		case 2: return Compare<double>(std::get<DoubleObject>(lhs).Value, std::get<DoubleObject>(rhs).Value);
		case 3: return Compare<Object*>(static_cast<Object*>(std::get<PointerObject>(lhs).Value), static_cast<Object*>(std::get<PointerObject>(rhs).Value));
		case 4: return Compare<ManagedHeapInfo*>(static_cast<ManagedHeapInfo*>(std::get<GCPointerObject>(lhs).Value), static_cast<ManagedHeapInfo*>(std::get<GCPointerObject>(rhs).Value));
		}
	}
}