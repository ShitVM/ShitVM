#include <svm/Interpreter.hpp>

#include <svm/detail/InterpreterExceptionCode.hpp>

#include <type_traits>

namespace svm {
	SVM_NOINLINE_FOR_PROFILING void Interpreter::InterpretToI() noexcept {
		if (IsLocalVariable()) {
			OccurException(SVM_IEC_STACK_EMPTY);
			return;
		}

		Type* const typePtr = m_Stack.GetTopType();
		if (!typePtr) {
			OccurException(SVM_IEC_STACK_EMPTY);
			return;
		}

		const Type type = *typePtr;
		if (type == IntType) {
			TypeCast<IntObject, IntObject>(typePtr);
		} else if (type == LongType) {
			TypeCast<IntObject, LongObject>(typePtr);
		} else if (type == DoubleType) {
			TypeCast<IntObject, DoubleObject>(typePtr);
		} else if (type == PointerType) {
			TypeCast<IntObject, PointerObject>(typePtr);
		} else if (type == GCPointerType) {
			OccurException(SVM_IEC_POINTER_INVALIDFORPOINTER);
		} else if (type.IsStructure()) {
			OccurException(SVM_IEC_STRUCTURE_INVALIDFORSTRUCTURE);
		} else if (type.IsArray()) {
			OccurException(SVM_IEC_ARRAY_INVALIDFORARRAY);
		} else {
			OccurException(SVM_IEC_STACK_EMPTY);
		}
	}
	SVM_NOINLINE_FOR_PROFILING void Interpreter::InterpretToL() noexcept {
		if (IsLocalVariable()) {
			OccurException(SVM_IEC_STACK_EMPTY);
			return;
		}

		Type* const typePtr = m_Stack.GetTopType();
		if (!typePtr) {
			OccurException(SVM_IEC_STACK_EMPTY);
			return;
		}

		const Type type = *typePtr;
		if (type == IntType) {
			TypeCast<LongObject, IntObject>(typePtr);
		} else if (type == LongType) {
			TypeCast<LongObject, LongObject>(typePtr);
		} else if (type == DoubleType) {
			TypeCast<LongObject, DoubleObject>(typePtr);
		} else if (type == PointerType) {
			TypeCast<LongObject, PointerObject>(typePtr);
		} else if (type == GCPointerType) {
			OccurException(SVM_IEC_POINTER_INVALIDFORPOINTER);
		} else if (type.IsStructure()) {
			OccurException(SVM_IEC_STRUCTURE_INVALIDFORSTRUCTURE);
		} else if (type.IsArray()) {
			OccurException(SVM_IEC_ARRAY_INVALIDFORARRAY);
		} else {
			OccurException(SVM_IEC_STACK_EMPTY);
		}
	}
	SVM_NOINLINE_FOR_PROFILING void Interpreter::InterpretToD() noexcept {
		if (IsLocalVariable()) {
			OccurException(SVM_IEC_STACK_EMPTY);
			return;
		}

		Type* const typePtr = m_Stack.GetTopType();
		if (!typePtr) {
			OccurException(SVM_IEC_STACK_EMPTY);
			return;
		}

		const Type type = *typePtr;
		if (type == IntType) {
			TypeCast<DoubleObject, IntObject>(typePtr);
		} else if (type == LongType) {
			TypeCast<DoubleObject, LongObject>(typePtr);
		} else if (type == DoubleType) {
			TypeCast<DoubleObject, DoubleObject>(typePtr);
		} else if (type == PointerType) {
			TypeCast<DoubleObject, PointerObject>(typePtr);
		} else if (type == GCPointerType) {
			OccurException(SVM_IEC_POINTER_INVALIDFORPOINTER);
		} else if (type.IsStructure()) {
			OccurException(SVM_IEC_STRUCTURE_INVALIDFORSTRUCTURE);
		} else if (type.IsArray()) {
			OccurException(SVM_IEC_ARRAY_INVALIDFORARRAY);
		} else {
			OccurException(SVM_IEC_STACK_EMPTY);
		}
	}
	SVM_NOINLINE_FOR_PROFILING void Interpreter::InterpretToP() noexcept {
		if (IsLocalVariable()) {
			OccurException(SVM_IEC_STACK_EMPTY);
			return;
		}

		Type* const typePtr = m_Stack.GetTopType();
		if (!typePtr) {
			OccurException(SVM_IEC_STACK_EMPTY);
			return;
		}

		const Type type = *typePtr;
		if (type == IntType) {
			TypeCast<PointerObject, IntObject>(typePtr);
		} else if (type == LongType) {
			TypeCast<PointerObject, LongObject>(typePtr);
		} else if (type == DoubleType) {
			TypeCast<PointerObject, DoubleObject>(typePtr);
		} else if (type == PointerType) {
			TypeCast<PointerObject, PointerObject>(typePtr);
		} else if (type == GCPointerType) {
			OccurException(SVM_IEC_POINTER_INVALIDFORPOINTER);
		} else if (type.IsStructure()) {
			OccurException(SVM_IEC_STRUCTURE_INVALIDFORSTRUCTURE);
		} else if (type.IsArray()) {
			OccurException(SVM_IEC_ARRAY_INVALIDFORARRAY);
		} else {
			OccurException(SVM_IEC_STACK_EMPTY);
		}
	}
}

namespace svm {
	template<typename T, typename F>
	SVM_NOINLINE_FOR_PROFILING void Interpreter::TypeCast(Type* typePtr) noexcept {
		if constexpr (std::is_same_v<F, T>) return;
		else if constexpr (sizeof(T) > sizeof(F)) {
			if (!m_Stack.Expand(sizeof(T) - sizeof(F))) {
				OccurException(SVM_IEC_STACK_OVERFLOW);
				return;
			}
		} else if constexpr (sizeof(T) < sizeof(F)) {
			m_Stack.Reduce(sizeof(F) - sizeof(T));
		}

		*m_Stack.GetTop<T>() = reinterpret_cast<F*>(typePtr)->template Cast<T>();
	}
}