#include <svm/Interpreter.hpp>

#include <svm/detail/InterpreterExceptionCode.hpp>

#include <cstdlib>

namespace svm {
	void Interpreter::InterpretNull() noexcept {
		if (!m_Stack.Push<PointerObject>(nullptr)) {
			OccurException(SVM_IEC_STACK_OVERFLOW);
		}
	}
	void Interpreter::InterpretNew(std::uint32_t operand) noexcept {
		if (operand >= m_ByteFile.GetStructures().GetCount()) {
			OccurException(SVM_IEC_TYPE_OUTOFRANGE);
			return;
		}

		const Type type = GetTypeFromTypeCode(m_ByteFile.GetStructures(), static_cast<TypeCode>(operand));
		void* address = std::calloc(1, type->Size);

		if (!m_Stack.Push<PointerObject>(address)) {
			std::free(address);
			OccurException(SVM_IEC_STACK_OVERFLOW);
		}
	}
	void Interpreter::InterpretDelete() noexcept {
		const Type* const typePtr = m_Stack.GetTopType();
		if (!typePtr) {
			OccurException(SVM_IEC_STACK_EMPTY);
			return;
		} else if (*typePtr != PointerType) {
			OccurException(SVM_IEC_POINTER_NOTPOINTER);
			return;
		}

		m_Stack.Remove(sizeof(PointerObject));
		std::free(reinterpret_cast<const PointerObject*>(typePtr)->Value);
	}
}