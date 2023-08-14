#include <svm/virtual/VirtualContext.hpp>

#include <svm/Heap.hpp>
#include <svm/Interpreter.hpp>
#include <svm/Object.hpp>

#include <cassert>
#include <cstddef>
#include <cstring>

namespace svm {
	VirtualContext::VirtualContext(Interpreter& interpreter, VirtualStack& stack, Heap& heap) noexcept
		: m_Interpreter(interpreter), m_Stack(stack), m_Heap(heap) {}

	Structure VirtualContext::GetStructure(VirtualModule::StructureIndex structure) {
		return m_Interpreter.GetStructure(static_cast<TypeCode>(
			static_cast<std::uint32_t>(structure)
			+ static_cast<std::uint32_t>(TypeCode::Structure)));
	}
	Structure VirtualContext::GetStructure(VirtualModule::MappedStructureIndex structure) {
		return m_Interpreter.GetStructure(static_cast<TypeCode>(
			static_cast<std::uint32_t>(structure)
			+ m_Interpreter.GetStructureCount()
			+ static_cast<std::uint32_t>(TypeCode::Structure)));
	}

	VirtualObject VirtualContext::GetField(const VirtualObject& structure, std::uint32_t index) {
		const Type type = structure.IsStructure();
		assert(type != nullptr);

		return reinterpret_cast<Object*>(
			reinterpret_cast<std::uint8_t*>(structure.GetObjectPtr())
			+ m_Interpreter.GetStructure(type->Code)->Fields[index].Offset);
	}
	VirtualObject VirtualContext::GetElement(const VirtualObject& array, std::uint64_t index) {
		const Type type = array.IsArray();
		assert(type != nullptr);

		return reinterpret_cast<Object*>(
			reinterpret_cast<std::uint8_t*>(static_cast<ArrayObject*>(array.GetObjectPtr()) + 1)
			+ index * type->Size);
	}

	VirtualObject VirtualContext::GetParameter(std::uint16_t index) {
		return m_Stack.GetParameter(index);
	}

	VirtualObject VirtualContext::PushFundamental(const Object& object, std::uint64_t count) {
		const Type type = object.GetType();
		assert(type.IsFundamentalType());

		if (count == 0) {
			if (!m_Stack.Expand(type->Size)) return VNULL;

			VirtualObject top = m_Stack.GetTop();
			InitFundamental(top.GetObjectPtr(), object, type);
			return top;
		} else {
			const auto size = m_Interpreter.CalcArraySize(type, count);
			if (!m_Stack.Expand(size)) return VNULL;

			VirtualObject top = m_Stack.GetTop();
			InitFundamental(top.GetObjectPtr(), type, count);
			return top;
		}
	}
	VirtualObject VirtualContext::PushStructure(Structure structure, std::uint64_t count) {
		assert(structure != nullptr);

		if (count == 0) {
			if (!m_Stack.Expand(structure->Type.Size)) return VNULL;

			VirtualObject top = m_Stack.GetTop();
			InitStructure(top.GetObjectPtr(), structure);
			return top;
		} else {
			const auto size = m_Interpreter.CalcArraySize(structure->Type, count);
			if (!m_Stack.Expand(size)) return VNULL;

			VirtualObject top = m_Stack.GetTop();
			InitStructure(top.GetObjectPtr(), structure, count);
			return top;
		}
	}
	bool VirtualContext::Pop() {
		return m_Stack.Pop();
	}

	VirtualObject::PointerTarget VirtualContext::GetPointer(const VirtualObject& object) {
		return static_cast<VirtualObject::PointerTarget>(reinterpret_cast<std::uintptr_t>(object.GetObjectPtr()));
	}
	VirtualObject VirtualContext::GetObject(VirtualObject::PointerTarget pointer) {
		return reinterpret_cast<Object*>(static_cast<std::uintptr_t>(pointer));
	}
	VirtualObject VirtualContext::GetObject(VirtualObject::GCPointerTarget pointer) {
		return reinterpret_cast<ManagedHeapInfo*>(static_cast<std::uintptr_t>(pointer));
	}

	VirtualObject VirtualContext::NewFundamental(const Object& object, std::uint64_t count) {
		const Type type = object.GetType();
		assert(type.IsFundamentalType());

		if (count == 0) {
			if (void* const addr = m_Heap.AllocateUnmanagedHeap(type->Size); addr) {
				InitFundamental(addr, object, type);
				return static_cast<Object*>(addr);
			} else {
				return VNULL;
			}
		} else {
			const auto size = m_Interpreter.CalcArraySize(type, count);
			if (void* const addr = m_Heap.AllocateUnmanagedHeap(size); addr) {
				InitFundamental(addr, type, count);
				return static_cast<Object*>(addr);
			} else {
				return VNULL;
			}
		}
	}
	VirtualObject VirtualContext::GCNewFundamental(const Object& object, std::uint64_t count) {
		const Type type = object.GetType();
		assert(type.IsFundamentalType());

		if (count == 0) {
			if (const auto addr = static_cast<ManagedHeapInfo*>(m_Heap.AllocateManagedHeap(m_Interpreter, type->Size)); addr) {
				InitFundamental(addr + 1, object, type);
				return addr;
			} else {
				return VNULL;
			}
		} else {
			const auto size = m_Interpreter.CalcArraySize(type, count);
			if (const auto addr = static_cast<ManagedHeapInfo*>(m_Heap.AllocateManagedHeap(m_Interpreter, size)); addr) {
				InitFundamental(addr + 1, type, count);
				return addr;
			} else {
				return VNULL;
			}
		}
	}
	VirtualObject VirtualContext::NewStructure(Structure structure, std::uint64_t count) {
		assert(structure != nullptr);

		if (count == 0) {
			if (void* const addr = m_Heap.AllocateUnmanagedHeap(structure->Type.Size); addr) {
				InitStructure(addr, structure);
				return static_cast<Object*>(addr);
			} else {
				return VNULL;
			}
		} else {
			const auto size = m_Interpreter.CalcArraySize(structure->Type, count);
			if (void* const addr = m_Heap.AllocateUnmanagedHeap(size); addr) {
				InitStructure(addr, structure, count);
				return static_cast<Object*>(addr);
			} else {
				return VNULL;
			}
		}
	}
	VirtualObject VirtualContext::GCNewStructure(Structure structure, std::uint64_t count) {
		assert(structure != nullptr);

		if (count == 0) {
			if (const auto addr = static_cast<ManagedHeapInfo*>(
				m_Heap.AllocateManagedHeap(m_Interpreter, structure->Type.Size)); addr) {
				InitStructure(addr + 1, structure);
				return addr;
			} else {
				return VNULL;
			}
		} else {
			const auto size = m_Interpreter.CalcArraySize(structure->Type, count);
			if (const auto addr = static_cast<ManagedHeapInfo*>(
				m_Heap.AllocateManagedHeap(m_Interpreter, size)); addr) {
				InitStructure(addr + 1, structure, count);
				return addr;
			} else {
				return VNULL;
			}
		}
	}
	void VirtualContext::DeleteObject(const VirtualObject& object) {
		assert(object.IsPointer());

		m_Heap.DeallocateUnmanagedHeap(
			reinterpret_cast<void*>(static_cast<std::uintptr_t>(object.ToPointer())));
	}

	void VirtualContext::CopyObject(const VirtualObject& dest, const VirtualObject& src) {
		if (src.GetType().IsFundamentalType()) {
			assert(dest.GetType() == src.GetType());

			std::memcpy(dest.GetObjectPtr(), src.GetObjectPtr(), src.GetType()->Size);
		} else if (const Type structure = src.IsStructure(); structure != nullptr) {
			assert(dest.GetType() == structure);

			std::memcpy(dest.GetObjectPtr(), src.GetObjectPtr(), structure->Size);
		} else if (const Type array = src.IsArray(); array != nullptr) {
			assert(dest.IsArray() == array);

			std::memcpy(dest.GetObjectPtr(), src.GetObjectPtr(), std::min(
				m_Interpreter.CalcArraySize(array, dest.GetCount()),
				m_Interpreter.CalcArraySize(array, src.GetCount())));
		}
	}
	void VirtualContext::CopyObjectUnsafe(VirtualObject::PointerTarget dest, VirtualObject::PointerTarget src, std::uint64_t count) {
		std::memcpy(
			reinterpret_cast<void*>(static_cast<std::uintptr_t>(dest)),
			reinterpret_cast<void*>(static_cast<std::uintptr_t>(src)),
			static_cast<std::size_t>(reinterpret_cast<Object*>(static_cast<std::uintptr_t>(src))->GetType()->Size * count));
	}

	void VirtualContext::InitFundamental(void* target, const Object& object, const Type& type) {
		if (type == IntType) {
			*static_cast<IntObject*>(target) = static_cast<const IntObject&>(object);
		} else if (type == LongType) {
			*static_cast<LongObject*>(target) = static_cast<const LongObject&>(object);
		} else if (type == PointerType) {
			*static_cast<PointerObject*>(target) = static_cast<const PointerObject&>(object);
		} else if (type == GCPointerType) {
			*static_cast<GCPointerObject*>(target) = static_cast<const GCPointerObject&>(object);
		}
	}
	void VirtualContext::InitFundamental(void* target, const Type& type, std::uint64_t count) {
		m_Interpreter.InitArray(static_cast<Object*>(target), type, count);
	}
	void VirtualContext::InitStructure(void* target, Structure structure) {
		m_Interpreter.InitStructure(static_cast<Object*>(target), structure);
	}
	void VirtualContext::InitStructure(void* target, Structure structure, std::uint64_t count) {
		m_Interpreter.InitArray(static_cast<Object*>(target), structure->Type, count);
	}
}