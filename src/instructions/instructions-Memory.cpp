#include <svm/Interpreter.hpp>

#include <svm/detail/InterpreterExceptionCode.hpp>

namespace svm {
	void Interpreter::InterpretNull() noexcept {
		if (!m_Stack.Push<PointerObject>(nullptr)) {
			OccurException(SVM_IEC_STACK_OVERFLOW);
		}
	}
	void Interpreter::InterpretNew(std::uint32_t operand) noexcept {
		const Structures& structures = m_ByteFile.GetStructures();

		if (operand >= structures.GetStructureCount() + 10) {
			OccurException(SVM_IEC_TYPE_OUTOFRANGE);
			return;
		}

		const Type type = GetTypeFromTypeCode(structures, static_cast<TypeCode>(operand));
		void* address = m_Heap.AllocateUnmanagedHeap(type->Size);

		if (!m_Stack.Push<PointerObject>(address)) {
			m_Heap.DeallocateLastUnmanagedHeap();
			OccurException(SVM_IEC_STACK_OVERFLOW);
		}

		if (!address) return;
		else if (type.IsFundamentalType()) {
			*static_cast<Type*>(address) = type;
		} else if (type.IsStructure()) {
			InitStructure(structures, structures[operand - 10], static_cast<Type*>(address));
		}
	}
	void Interpreter::InterpretDelete() noexcept {
		if (IsLocalVariable()) {
			OccurException(SVM_IEC_STACK_EMPTY);
			return;
		}

		const Type* const typePtr = m_Stack.GetTopType();
		if (!typePtr) {
			OccurException(SVM_IEC_STACK_EMPTY);
			return;
		} else if (*typePtr != PointerType) {
			OccurException(SVM_IEC_POINTER_NOTPOINTER);
			return;
		}

		if (!m_Heap.DeallocateUnmanagedHeap(reinterpret_cast<const PointerObject*>(typePtr)->Value)) {
			OccurException(SVM_IEC_POINTER_UNKNOWNADDRESS);
		}

		m_Stack.Reduce(sizeof(PointerObject));
	}
}