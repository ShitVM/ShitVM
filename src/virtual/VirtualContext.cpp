#include <svm/virtual/VirtualContext.hpp>

#include <svm/Interpreter.hpp>
#include <svm/Object.hpp>

#include <cassert>

namespace svm {
	VirtualContext::VirtualContext(Interpreter& interpreter, VirtualStack& stack) noexcept
		: m_Interpreter(interpreter), m_Stack(stack) {}

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
			if (type == IntType) {
				*static_cast<IntObject*>(top.GetObjectPtr()) = static_cast<const IntObject&>(object);
			} else if (type == LongType) {
				*static_cast<LongObject*>(top.GetObjectPtr()) = static_cast<const LongObject&>(object);
			} else if (type == PointerType) {
				*static_cast<PointerObject*>(top.GetObjectPtr()) = static_cast<const PointerObject&>(object);
			} else if (type == GCPointerType) {
				*static_cast<GCPointerObject*>(top.GetObjectPtr()) = static_cast<const GCPointerObject&>(object);
			}
			return top;
		} else {
			const auto size = m_Interpreter.CalcArraySize(type, count);
			if (!m_Stack.Expand(size)) return VNULL;

			VirtualObject top = m_Stack.GetTop();
			m_Interpreter.InitArray(top.GetObjectPtr(), type, count);
			return top;
		}
	}
	VirtualObject VirtualContext::PushStructure(Structure structure, std::uint64_t count) {
		assert(structure != nullptr);

		if (count == 0) {
			if (!m_Stack.Expand(structure->Type.Size)) return VNULL;

			VirtualObject top = m_Stack.GetTop();
			m_Interpreter.InitStructure(top.GetObjectPtr(), structure);
			return top;
		} else {
			const auto size = m_Interpreter.CalcArraySize(structure->Type, count);
			if (!m_Stack.Expand(size)) return VNULL;

			VirtualObject top = m_Stack.GetTop();
			m_Interpreter.InitArray(top.GetObjectPtr(), structure->Type, count);
			return top;
		}
	}
	bool VirtualContext::Pop() {
		return m_Stack.Pop();
	}
}