#include <svm/Exception.hpp>

#include <svm/detail/InterpreterExceptionCode.hpp>

namespace svm {
	std::string_view GetInterpreterExceptionMessage(std::uint32_t code) noexcept {
		using namespace std::string_view_literals;

		switch (code) {
		case SVM_IEC_TYPE_OUTOFRANGE: return "Type does not exist"sv;

		case SVM_IEC_STACK_OVERFLOW: return "Stack overflow"sv;
		case SVM_IEC_STACK_EMPTY: return "The stack is empty"sv;
		case SVM_IEC_STACK_DIFFERENTTYPE: return "The two operands have different types"sv;

		case SVM_IEC_CONSTANTPOOL_OUTOFRANGE: return "Constant does not exist"sv;

		case SVM_IEC_ARITHMETIC_DIVIDEBYZERO: return "Can't divide by zero"sv;

		case SVM_IEC_LOCALVARIABLE_OUTOFRANGE: return "Local variable does not exist"sv;
		case SVM_IEC_LOCALVARIABLE_INVALIDINDEX: return "Invalid local variable index"sv;

		case SVM_IEC_LABEL_OUTOFRANGE: return "Label does not exist"sv;

		case SVM_IEC_FUNCTION_OUTOFRANGE: return "Function does not exist"sv;
		case SVM_IEC_FUNCTION_NORETINSTRUCTION: return "Function exited without 'ret' instruction"sv;

		case SVM_IEC_POINTER_NULLPOINTER: return "Can't dereference null pointer"sv;
		case SVM_IEC_POINTER_NOTPOINTER: return "Not a pointer"sv;
		case SVM_IEC_POINTER_INVALIDFORPOINTER: return "Can't operate on pointers"sv;
		case SVM_IEC_POINTER_UNKNOWNADDRESS: return "Address is not registered in the table"sv;

		case SVM_IEC_STRUCTURE_FIELD_OUTOFRANGE: return "Field does not exist"sv;
		case SVM_IEC_STRUCTURE_NOTSTRUCTURE: return "Not a structure"sv;
		case SVM_IEC_STRUCTURE_INVALIDFORSTRUCTURE: return "Can't operate on structures"sv;

		default: return ""sv;
		}
	}
}