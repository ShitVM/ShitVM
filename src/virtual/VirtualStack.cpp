#include <svm/virtual/VirtualStack.hpp>

#include <svm/Interpreter.hpp>
#include <svm/Object.hpp>
#include <svm/Stack.hpp>

#include <cstddef>
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
	bool VirtualStack::Expand(std::size_t size) {
		return m_Stack->Expand(size);
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

	VirtualObject VirtualStack::GetParameter(std::uint16_t index) noexcept {
		const std::uint32_t realIndex = index + m_StackFrame->VariableBegin;
		if (realIndex >= m_LocalVariables->size()) return VNULL;
		else return m_Stack->Get<Object>((*m_LocalVariables)[realIndex]);
	}
}