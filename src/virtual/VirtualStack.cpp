#include <svm/virtual/VirtualStack.hpp>

#include <svm/Interpreter.hpp>
#include <svm/Object.hpp>
#include <svm/Stack.hpp>

#include <cstring>

namespace svm {
	VirtualStack::VirtualStack(Stack* stack, const StackFrame* stackFrame, const std::vector<std::size_t>* localVariables) noexcept
		: m_Stack(stack), m_StackFrame(stackFrame), m_LocalVariables(localVariables) {}

	bool VirtualStack::IsEmpty() const noexcept {
		return m_Stack->GetUsedSize() == 0;
	}
	VirtualObject VirtualStack::GetTop() noexcept {
		if (m_Stack->GetUsedSize() == m_StackFrame->StackBegin) return VNULL;
		else return reinterpret_cast<Object*>(m_Stack->GetTopType());
	}
	bool VirtualStack::Push(const VirtualObject& object) {
		const Type type = object.GetType();
		if (type == IntType) return m_Stack->Push<IntObject>(object.ToInt());
		else if (type == LongType) return m_Stack->Push<LongObject>(object.ToLong());
		else if (type == DoubleType) return m_Stack->Push<DoubleObject>(object.ToDouble());
		else if (type == PointerType) return m_Stack->Push<PointerObject>(object.ToPointer());
		else if (type == GCPointerType) return m_Stack->Push<GCPointerObject>(object.ToGCPointer());
		else if (const auto structure = object.IsStructure(); structure) {
			const std::size_t size = structure->GetType()->Size;
			if (!m_Stack->Expand(size)) return false;

			std::memcpy(m_Stack->GetTopType(), structure, size);
			return true;
		} else if (const auto array = object.IsArray(); array) {
			Object* const firstElement = static_cast<Object*>(array + 1);
			const std::size_t size = static_cast<std::size_t>(array->Count * firstElement->GetType()->Size + sizeof(ArrayObject));
			if (!m_Stack->Expand(size)) return false;

			std::memcpy(m_Stack->GetTopType(), array, size);
			return true;
		}

		return false;
	}
	bool VirtualStack::Pop() {
		if (m_Stack->GetUsedSize() == m_StackFrame->StackBegin) return false;

		Type* const typePtr = m_Stack->GetTopType();
		const Type type = *typePtr;

		if (type.IsArray()) {
			ArrayObject* const array = reinterpret_cast<ArrayObject*>(typePtr);
			Object* const firstElement = static_cast<Object*>(array + 1);
			const std::size_t size = static_cast<std::size_t>(array->Count * firstElement->GetType()->Size + sizeof(ArrayObject));
			m_Stack->Reduce(size);
		} else {
			m_Stack->Reduce(type->Size);
		}

		return true;
	}

	VirtualObject VirtualStack::Parameter(std::uint16_t index) noexcept {
		const std::uint32_t realIndex = index + m_StackFrame->VariableBegin;
		if (realIndex >= m_LocalVariables->size()) return VNULL;
		else return m_Stack->Get<Object>((*m_LocalVariables)[realIndex]);
	}
}