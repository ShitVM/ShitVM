#include <svm/Exception.hpp>

#include <svm/detail/InterpreterExceptionCode.hpp>

namespace svm {
	std::string_view GetInterpreterExceptionMessage(std::uint32_t code) noexcept {
		switch (code) {
		case SVM_IEC_TYPE_OUTOFRANGE: return "Type does not exist";

		case SVM_IEC_STACK_OVERFLOW: return "Stack overflow";
		case SVM_IEC_STACK_EMPTY: return "The stack is empty";
		case SVM_IEC_STACK_DIFFERENTTYPE: return "The two operands have different types";

		case SVM_IEC_CONSTANTPOOL_OUTOFRANGE: return "Constant does not exist";

		case SVM_IEC_ARITHMETIC_DIVIDEBYZERO: return "Can't divide by zero";

		case SVM_IEC_LOCALVARIABLE_OUTOFRANGE: return "Local variable does not exist";
		case SVM_IEC_LOCALVARIABLE_INVALIDINDEX: return "Invalid local variable index";

		case SVM_IEC_LABEL_OUTOFRANGE: return "Label does not exist";

		case SVM_IEC_FUNCTION_OUTOFRANGE: return "Function does not exist";
		case SVM_IEC_FUNCTION_NORETINSTRUCTION: return "Function exited without 'ret' instruction";

		case SVM_IEC_POINTER_NULLPOINTER: return "Can't dereference null pointer";
		case SVM_IEC_POINTER_NOTPOINTER: return "Not a pointer";
		case SVM_IEC_POINTER_INVALIDFORPOINTER: return "Can't operate on pointers";
		case SVM_IEC_POINTER_UNKNOWNADDRESS: return "Address is not registered in the table";

		case SVM_IEC_STRUCTURE_FIELD_OUTOFRANGE: return "Field does not exist";
		case SVM_IEC_STRUCTURE_NOTSTRUCTURE: return "Not a structure";
		case SVM_IEC_STRUCTURE_INVALIDFORSTRUCTURE: return "Can't operate on structures";

		default: return "";
		}
	}
}