#include <svm/Exception.hpp>

#include <svm/detail/InterpreterExceptionCode.hpp>

namespace svm {
	std::string_view GetInterpreterExceptionMessage(std::uint32_t code) noexcept {
		switch (code) {
		case SVM_IEC_STACK_OVERFLOW: return "Stack overflow";
		case SVM_IEC_STACK_EMPTY: return "The stack is empty";
		case SVM_IEC_STACK_DIFFERENTTYPE: return "The two operands have different types";

		case SVM_IEC_CONSTANTPOOL_OUTOFRANGE: return "Constant does not exist";

		case SVM_IEC_ARITHMETIC_DIVIDEBYZERO: return "Can't divide by zero";

		case SVM_IEC_LOCALVARIABLE_OUTOFRANGE: return "Local variable does not exist";
		case SVM_IEC_LOCALVARIABLE_INVALIDINDEX: return "Invalid local variable index";

		case SVM_IEC_LABEL_OUTOFRANGE: return "Label does not exist";

		case SVM_IEC_FUNCTION_OUTOFRANGE: return "Function does not exist";
		case SVM_IEC_FUNCTION_TOPOFCALLSTACK: return "Already at the top of the call stack";
		case SVM_IEC_FUNCTION_NORETINSTRUCTION: return "Function exited without 'ret' instruction";

		default: return "";
		}
	}
}