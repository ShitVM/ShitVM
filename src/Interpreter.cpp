#include <svm/Interpreter.hpp>

#include <svm/Object.hpp>
#include <svm/detail/InterpreterExceptionCode.hpp>

#include <cmath>
#include <type_traits>
#include <variant>

namespace svm {
	Stack::Stack(std::size_t size) {
		Allocate(size);
	}
	Stack::Stack(Stack&& stack) noexcept
		: m_Data(std::move(stack.m_Data)), m_Used(stack.m_Used) {}

	Stack& Stack::operator=(Stack&& stack) noexcept {
		m_Data = std::move(stack.m_Data);
		m_Used = stack.m_Used;
		return *this;
	}

	void Stack::Allocate(std::size_t size) {
		m_Data.resize(size);
		m_Used = 0;
	}
	void Stack::Reallocate(std::size_t newSize) {
		if (m_Used > newSize) throw std::runtime_error("Failed to resize the stack. Smaller size than the used size.");

		std::vector<std::uint8_t> newData(newSize);
		std::copy(m_Data.rbegin(), m_Data.rbegin() + m_Used, newData.rbegin());

		m_Data = std::move(newData);
	}
	void Stack::Deallocate() noexcept {
		m_Data.clear();
		m_Used = 0;
	}

	const Type** Stack::GetTopType() noexcept {
		return Get<const Type*>(m_Used);
	}
	std::size_t Stack::GetSize() const noexcept {
		return m_Data.size();
	}
	std::size_t Stack::GetUsedSize() const noexcept {
		return m_Used;
	}
	std::size_t Stack::GetFreeSize() const noexcept {
		return GetSize() - m_Used;
	}
	void Stack::RemoveTo(std::size_t newSize) noexcept {
		m_Used = newSize;
	}
}

namespace svm {
	Interpreter::Interpreter(ByteFile&& byteFile) noexcept
		: m_ByteFile(std::move(byteFile)) {
		m_StackFrame.Instructions = &m_ByteFile.GetEntryPoint();
	}
	Interpreter::Interpreter(Interpreter&& interpreter) noexcept
		: m_ByteFile(std::move(interpreter.m_ByteFile)),
		m_Stack(std::move(interpreter.m_Stack)), m_StackFrame(interpreter.m_StackFrame), m_InstructionIndex(interpreter.m_InstructionIndex),
		m_LocalVariables(std::move(interpreter.m_LocalVariables)),
		m_Exception(std::move(interpreter.m_Exception)) {
		interpreter.m_InstructionIndex = 0;
	}

	Interpreter& Interpreter::operator=(Interpreter&& interpreter) noexcept {
		m_ByteFile = std::move(interpreter.m_ByteFile);

		m_Stack = std::move(interpreter.m_Stack);
		m_StackFrame = interpreter.m_StackFrame;
		m_InstructionIndex = interpreter.m_InstructionIndex;

		m_LocalVariables = std::move(interpreter.m_LocalVariables);

		m_Exception = std::move(interpreter.m_Exception);

		interpreter.m_StackFrame = {};
		interpreter.m_InstructionIndex = 0;
		return *this;
	}

	void Interpreter::Clear() noexcept {
		m_ByteFile.Clear();

		m_Stack.Deallocate();
		m_StackFrame = {};
		m_InstructionIndex = 0;

		m_LocalVariables.clear();

		m_Exception.reset();
	}
	void Interpreter::Load(ByteFile&& byteFile) noexcept {
		m_ByteFile = std::move(byteFile);
		m_StackFrame.Instructions = &m_ByteFile.GetEntryPoint();
	}
	void Interpreter::AllocateStack(std::size_t size) {
		m_Stack.Allocate(size);
	}
	void Interpreter::ReallocateStack(std::size_t newSize) {
		m_Stack.Reallocate(newSize);
	}

	bool Interpreter::Interpret() {
		for (; m_InstructionIndex < m_StackFrame.Instructions->GetInstructionCount(); ++m_InstructionIndex) {
			const Instruction& inst = m_StackFrame.Instructions->GetInstruction(m_InstructionIndex);
			switch (inst.OpCode) {
			case OpCode::Push: InterpretPush(inst.Operand); break;
			case OpCode::Pop: InterpretPop(); break;
			case OpCode::Load: InterpretLoad(inst.Operand); break;
			case OpCode::Store: InterpretStore(inst.Operand); break;
			case OpCode::Copy: InterpretCopy(); break;
			case OpCode::Swap: InterpretSwap(); break;

			case OpCode::Add: InterpretAdd(); break;
			case OpCode::Sub: InterpretSub(); break;
			case OpCode::Mul: InterpretMul(); break;
			case OpCode::IMul: InterpretIMul(); break;
			case OpCode::Div: InterpretDiv(); break;
			case OpCode::IDiv: InterpretIDiv(); break;
			case OpCode::Mod: InterpretMod(); break;
			case OpCode::IMod: InterpretIMod(); break;
			case OpCode::Neg: InterpretNeg(); break;
			case OpCode::Inc: InterpretIncDec(1); break;
			case OpCode::Dec: InterpretIncDec(-1); break;

			case OpCode::And: InterpretAnd(); break;
			case OpCode::Or: InterpretOr(); break;
			case OpCode::Xor: InterpretXor(); break;
			case OpCode::Not: InterpretNot(); break;
			case OpCode::Shl: InterpretShl(); break;
			case OpCode::Sal: InterpretSal(); break;
			case OpCode::Shr: InterpretShr(); break;
			case OpCode::Sar: InterpretSar(); break;

			case OpCode::Cmp: InterpretCmp(); break;
			case OpCode::ICmp: InterpretICmp(); break;
			case OpCode::Jmp: InterpretJmp(inst.Operand); break;
			case OpCode::Je: InterpretJe(inst.Operand); break;
			case OpCode::Jne: InterpretJne(inst.Operand); break;
			case OpCode::Ja: InterpretJa(inst.Operand); break;
			case OpCode::Jae: InterpretJae(inst.Operand); break;
			case OpCode::Jb: InterpretJb(inst.Operand); break;
			case OpCode::Jbe: InterpretJbe(inst.Operand); break;
			case OpCode::Call: InterpretCall(inst.Operand); break;
			case OpCode::Ret: InterpretRet(); break;

			case OpCode::ToI: InterpretToI(); break;
			case OpCode::ToL: InterpretToL(); break;
			case OpCode::ToD: InterpretToD(); break;
			}

			if (m_Exception.has_value()) return false;
		}

		return true;
	}
	const InterpreterException& Interpreter::GetException() const noexcept {
		return *m_Exception;
	}

	void Interpreter::OccurException(std::uint32_t code) noexcept {
		InterpreterException& e = m_Exception.emplace();
		e.Function = m_StackFrame.Function;
		e.Instructions = m_StackFrame.Instructions;
		e.InstructionIndex = m_InstructionIndex;

		e.Code = code;
	}

	template<typename T>
	void Interpreter::PopTwoSameType(const Type*& rhsType, T& lhs, T& rhs) noexcept {
		if constexpr (std::is_same_v<IntObject, T>) {
			const Type** const lhsTypePtr = m_Stack.Get<const Type*>(m_Stack.GetUsedSize() - sizeof(IntObject));
			if (!lhsTypePtr) {
				OccurException(SVM_IEC_STACK_EMPTY);
				return;
			} else if (rhsType != *lhsTypePtr) {
				OccurException(SVM_IEC_STACK_DIFFERENTTYPE);
				return;
			}

			lhs = *reinterpret_cast<IntObject*>(lhsTypePtr);
			rhs = reinterpret_cast<IntObject&>(rhsType);
		} else if constexpr (std::is_same_v<IntObject, T>) {
			const Type** const lhsTypePtr = m_Stack.Get<const Type*>(m_Stack.GetUsedSize() - sizeof(LongObject));
			if (!lhsTypePtr) {
				OccurException(SVM_IEC_STACK_EMPTY);
				return;
			} else if (rhsType != *lhsTypePtr) {
				OccurException(SVM_IEC_STACK_DIFFERENTTYPE);
				return;
			}

			lhs = *reinterpret_cast<LongObject*>(lhsTypePtr);
			rhs = reinterpret_cast<LongObject&>(rhsType);
		} else if constexpr (std::is_same_v<IntObject, T>) {
			const Type** const lhsTypePtr = m_Stack.Get<const Type*>(m_Stack.GetUsedSize() - sizeof(DoubleObject));
			if (!lhsTypePtr) {
				OccurException(SVM_IEC_STACK_EMPTY);
				return;
			} else if (rhsType != *lhsTypePtr) {
				OccurException(SVM_IEC_STACK_DIFFERENTTYPE);
				return;
			}

			lhs = *reinterpret_cast<DoubleObject*>(lhsTypePtr);
			rhs = reinterpret_cast<DoubleObject&>(rhsType);
		}

		m_Stack.Pop<T>();
		m_Stack.Pop<T>();
	}
	template<typename T>
	IntObject Interpreter::CompareTwoSameType(T lhs, T rhs) noexcept {
		if (lhs > rhs) {
			return 1;
		} else if (lhs == rhs) {
			return 0;
		} else {
			return static_cast<std::uint32_t>(-1);
		}
	}
	template<typename T>
	void Interpreter::JumpCondition(std::uint32_t operand) {
		if (operand >= m_StackFrame.Instructions->GetLabelCount()) {
			OccurException(SVM_IEC_LABEL_OUTOFRANGE);
			return;
		}

		const Type** const typePtr = m_Stack.GetTopType();
		if (!typePtr) {
			OccurException(SVM_IEC_STACK_EMPTY);
			return;
		}

		const Type* const type = *typePtr;
		if (type == IntType) {
			const IntObject& value = *m_Stack.GetTop<IntObject>();
			if (T::Compare(value.Value)) {
				m_InstructionIndex = m_StackFrame.Instructions->GetLabel(operand) - 1;
				m_Stack.Pop<IntObject>();
			}
		} else if (type == LongType) {
			const LongObject& value = *m_Stack.GetTop<LongObject>();
			if (T::Compare(value.Value)) {
				m_InstructionIndex = m_StackFrame.Instructions->GetLabel(operand) - 1;
				m_Stack.Pop<LongObject>();
			}
		} else if (type == DoubleType) {
			const DoubleObject& value = *m_Stack.GetTop<DoubleObject>();
			if (T::Compare(value.Value)) {
				m_InstructionIndex = m_StackFrame.Instructions->GetLabel(operand) - 1;
				m_Stack.Pop<DoubleObject>();
			}
		}
	}

	SVM_NOINLINE_FOR_PROFILING void Interpreter::InterpretPush(std::uint32_t operand) {
#define ConstantPool m_ByteFile.GetConstantPool()
		if (operand >= ConstantPool.GetAllCount()) {
			OccurException(SVM_IEC_CONSTANTPOOL_OUTOFRANGE);
			return;
		}

		const Type* const constType = ConstantPool.GetConstantType(operand);
		bool isSuccess = false;
		if (constType == IntType) {
			isSuccess = m_Stack.Push(ConstantPool.GetConstant<IntObject>(operand));
		} else if (constType == LongType) {
			isSuccess = m_Stack.Push(ConstantPool.GetConstant<LongObject>(operand));
		} else if (constType == DoubleType) {
			isSuccess = m_Stack.Push(ConstantPool.GetConstant<DoubleObject>(operand));
		}

		if (!isSuccess) {
			OccurException(SVM_IEC_STACK_OVERFLOW);
		}
#undef ConstantPool
	}
	SVM_NOINLINE_FOR_PROFILING void Interpreter::InterpretPop() {
		if (!m_LocalVariables.empty() && m_Stack.GetUsedSize() == m_LocalVariables.back()) {
			OccurException(SVM_IEC_STACK_EMPTY);
			return;
		}

		const Type** const typePtr = m_Stack.GetTopType();
		if (!typePtr) {
			OccurException(SVM_IEC_STACK_EMPTY);
			return;
		}

		const Type* const type = *typePtr;
		if (type == IntType) {
			m_Stack.Pop<IntObject>();
		} else if (type == LongType) {
			m_Stack.Pop<LongObject>();
		} else if (type == DoubleType) {
			m_Stack.Pop<DoubleObject>();
		} else {
			OccurException(SVM_IEC_STACK_EMPTY);
		}
	}
	SVM_NOINLINE_FOR_PROFILING void Interpreter::InterpretLoad(std::uint32_t operand) {
		operand += static_cast<std::uint32_t>(m_StackFrame.VariableBegin);
		if (operand >= m_LocalVariables.size()) {
			OccurException(SVM_IEC_LOCALVARIABLE_OUTOFRANGE);
			return;
		}

		const Type* const& typePtr = *m_Stack.Get<const Type*>(m_LocalVariables[operand]);
		bool isSuccess = false;
		if (typePtr == IntType) {
			isSuccess = m_Stack.Push(*reinterpret_cast<const IntObject*>(&typePtr));
		} else if (typePtr == LongType) {
			isSuccess = m_Stack.Push(*reinterpret_cast<const LongObject*>(&typePtr));
		} else if (typePtr == DoubleType) {
			isSuccess = m_Stack.Push(*reinterpret_cast<const DoubleObject*>(&typePtr));
		} else {
			OccurException(SVM_IEC_STACK_EMPTY);
		}

		if (!isSuccess) {
			OccurException(SVM_IEC_STACK_OVERFLOW);
		}
	}
	SVM_NOINLINE_FOR_PROFILING void Interpreter::InterpretStore(std::uint32_t operand) {
		operand += static_cast<std::uint32_t>(m_StackFrame.VariableBegin);
		if (operand > m_LocalVariables.size()) {
			OccurException(SVM_IEC_LOCALVARIABLE_INVALIDINDEX);
			return;
		} else if (operand == m_LocalVariables.size()) {
			const Type** const typePtr = m_Stack.GetTopType();
			if (!typePtr) {
				OccurException(SVM_IEC_STACK_EMPTY);
				return;
			}

			const Type* const type = *typePtr;
			if (type != IntType && type != LongType && type != DoubleType) {
				OccurException(SVM_IEC_STACK_EMPTY);
				return;
			}

			m_LocalVariables.push_back(m_Stack.GetUsedSize());
			return;
		}

		const Type** const typePtr = m_Stack.GetTopType();
		if (!typePtr) {
			OccurException(SVM_IEC_STACK_EMPTY);
			return;
		}

		const Type* const type = *typePtr;
		const Type*& varType = *m_Stack.Get<const Type*>(m_LocalVariables[operand]);
		if (type != varType) {
			OccurException(SVM_IEC_STACK_DIFFERENTTYPE);
			return;
		}

		if (type == IntType) {
			reinterpret_cast<IntObject&>(varType) = m_Stack.Pop<IntObject>().value();
		} else if (type == LongType) {
			reinterpret_cast<LongObject&>(varType) = m_Stack.Pop<LongObject>().value();
		} else if (type == DoubleType) {
			reinterpret_cast<DoubleObject&>(varType) = m_Stack.Pop<DoubleObject>().value();
		} else {
			OccurException(SVM_IEC_STACK_EMPTY);
		}
	}
	SVM_NOINLINE_FOR_PROFILING void Interpreter::InterpretCopy() {
		const Type** const typePtr = m_Stack.GetTopType();
		if (!typePtr) {
			OccurException(SVM_IEC_STACK_EMPTY);
			return;
		}

		const Type* const& type = *typePtr;
		bool isSuccess = false;
		if (type == IntType) {
			isSuccess = m_Stack.Push(reinterpret_cast<const IntObject&>(type));
		} else if (type == LongType) {
			isSuccess = m_Stack.Push(reinterpret_cast<const LongObject&>(type));
		} else if (type == DoubleType) {
			isSuccess = m_Stack.Push(reinterpret_cast<const DoubleObject&>(type));
		} else {
			OccurException(SVM_IEC_STACK_EMPTY);
		}

		if (!isSuccess) {
			OccurException(SVM_IEC_STACK_OVERFLOW);
		}
	}
	SVM_NOINLINE_FOR_PROFILING void Interpreter::InterpretSwap() {
		const Type** const firstTypePtr = m_Stack.GetTopType();
		if (!firstTypePtr) {
			OccurException(SVM_IEC_STACK_EMPTY);
			return;
		}

		const Type*& firstType = *firstTypePtr;
		if (firstType == IntType) {
			const Type** secondTypePtr = m_Stack.Get<const Type*>(m_Stack.GetUsedSize() - sizeof(IntObject));
			if (!secondTypePtr) {
				OccurException(SVM_IEC_STACK_EMPTY);
				return;
			}

			const Type*& secondType = *secondTypePtr;
			if (firstType != secondType) {
				OccurException(SVM_IEC_STACK_DIFFERENTTYPE);
				return;
			}

			std::swap(reinterpret_cast<IntObject&>(firstType), reinterpret_cast<IntObject&>(secondType));
		} else if (firstType == LongType) {
			const Type** secondTypePtr = m_Stack.Get<const Type*>(m_Stack.GetUsedSize() - sizeof(LongObject));
			if (!secondTypePtr) {
				OccurException(SVM_IEC_STACK_EMPTY);
				return;
			}

			const Type*& secondType = *secondTypePtr;
			if (firstType != secondType) {
				OccurException(SVM_IEC_STACK_DIFFERENTTYPE);
				return;
			}

			std::swap(reinterpret_cast<LongObject&>(firstType), reinterpret_cast<LongObject&>(secondType));
		} else if (firstType == DoubleType) {
			const Type** secondTypePtr = m_Stack.Get<const Type*>(m_Stack.GetUsedSize() - sizeof(DoubleObject));
			if (!secondTypePtr) {
				OccurException(SVM_IEC_STACK_EMPTY);
				return;
			}

			const Type*& secondType = *secondTypePtr;
			if (firstType != secondType) {
				OccurException(SVM_IEC_STACK_DIFFERENTTYPE);
				return;
			}

			std::swap(reinterpret_cast<DoubleObject&>(firstType), reinterpret_cast<DoubleObject&>(secondType));
		} else {
			OccurException(SVM_IEC_STACK_EMPTY);
		}
	}

	SVM_NOINLINE_FOR_PROFILING void Interpreter::InterpretAdd() {
		const Type** const rhsTypePtr = m_Stack.GetTopType();
		if (!rhsTypePtr) {
			OccurException(SVM_IEC_STACK_EMPTY);
			return;
		}

		const Type*& rhsType = *rhsTypePtr;
		if (rhsType == IntType) {
			IntObject lhs, rhs;
			PopTwoSameType(rhsType, lhs, rhs);
			m_Stack.Push<IntObject>(lhs.Value + rhs.Value);
		} else if (rhsType == LongType) {
			LongObject lhs, rhs;
			PopTwoSameType(rhsType, lhs, rhs);
			m_Stack.Push<LongObject>(lhs.Value + rhs.Value);
		} else if (rhsType == DoubleType) {
			DoubleObject lhs, rhs;
			PopTwoSameType(rhsType, lhs, rhs);
			m_Stack.Push<DoubleObject>(lhs.Value + rhs.Value);
		} else {
			OccurException(SVM_IEC_STACK_EMPTY);
		}
	}
	SVM_NOINLINE_FOR_PROFILING void Interpreter::InterpretSub() {
		const Type** const rhsTypePtr = m_Stack.GetTopType();
		if (!rhsTypePtr) {
			OccurException(SVM_IEC_STACK_EMPTY);
			return;
		}

		const Type*& rhsType = *rhsTypePtr;
		if (rhsType == IntType) {
			IntObject lhs, rhs;
			PopTwoSameType(rhsType, lhs, rhs);
			m_Stack.Push<IntObject>(lhs.Value - rhs.Value);
		} else if (rhsType == LongType) {
			LongObject lhs, rhs;
			PopTwoSameType(rhsType, lhs, rhs);
			m_Stack.Push<LongObject>(lhs.Value - rhs.Value);
		} else if (rhsType == DoubleType) {
			DoubleObject lhs, rhs;
			PopTwoSameType(rhsType, lhs, rhs);
			m_Stack.Push<DoubleObject>(lhs.Value - rhs.Value);
		} else {
			OccurException(SVM_IEC_STACK_EMPTY);
		}
	}
	SVM_NOINLINE_FOR_PROFILING void Interpreter::InterpretMul() {
		const Type** const rhsTypePtr = m_Stack.GetTopType();
		if (!rhsTypePtr) {
			OccurException(SVM_IEC_STACK_EMPTY);
			return;
		}

		const Type*& rhsType = *rhsTypePtr;
		if (rhsType == IntType) {
			IntObject lhs, rhs;
			PopTwoSameType(rhsType, lhs, rhs);
			m_Stack.Push<IntObject>(lhs.Value * rhs.Value);
		} else if (rhsType == LongType) {
			LongObject lhs, rhs;
			PopTwoSameType(rhsType, lhs, rhs);
			m_Stack.Push<LongObject>(lhs.Value * rhs.Value);
		} else if (rhsType == DoubleType) {
			DoubleObject lhs, rhs;
			PopTwoSameType(rhsType, lhs, rhs);
			m_Stack.Push<DoubleObject>(lhs.Value * rhs.Value);
		} else {
			OccurException(SVM_IEC_STACK_EMPTY);
		}
	}
	SVM_NOINLINE_FOR_PROFILING void Interpreter::InterpretIMul() {
		const Type** const rhsTypePtr = m_Stack.GetTopType();
		if (!rhsTypePtr) {
			OccurException(SVM_IEC_STACK_EMPTY);
			return;
		}

		const Type*& rhsType = *rhsTypePtr;
		if (rhsType == IntType) {
			IntObject lhs, rhs;
			PopTwoSameType(rhsType, lhs, rhs);
			m_Stack.Push<IntObject>(static_cast<std::int32_t>(lhs.Value) * static_cast<std::int32_t>(rhs.Value));
		} else if (rhsType == LongType) {
			LongObject lhs, rhs;
			PopTwoSameType(rhsType, lhs, rhs);
			m_Stack.Push<LongObject>(static_cast<std::int64_t>(lhs.Value) * static_cast<std::int64_t>(rhs.Value));
		} else if (rhsType == DoubleType) {
			DoubleObject lhs, rhs;
			PopTwoSameType(rhsType, lhs, rhs);
			m_Stack.Push<DoubleObject>(lhs.Value * rhs.Value);
		} else {
			OccurException(SVM_IEC_STACK_EMPTY);
		}
	}
	SVM_NOINLINE_FOR_PROFILING void Interpreter::InterpretDiv() {
		const Type** const rhsTypePtr = m_Stack.GetTopType();
		if (!rhsTypePtr) {
			OccurException(SVM_IEC_STACK_EMPTY);
			return;
		}

		const Type*& rhsType = *rhsTypePtr;
		if (rhsType == IntType) {
			IntObject lhs, rhs;
			PopTwoSameType(rhsType, lhs, rhs);
			if (rhs.Value == 0) {
				OccurException(SVM_IEC_ARITHMETIC_DIVIDEBYZERO);
				return;
			}
			m_Stack.Push<IntObject>(lhs.Value / rhs.Value);
		} else if (rhsType == LongType) {
			LongObject lhs, rhs;
			PopTwoSameType(rhsType, lhs, rhs);
			if (rhs.Value == 0) {
				OccurException(SVM_IEC_ARITHMETIC_DIVIDEBYZERO);
				return;
			}
			m_Stack.Push<LongObject>(lhs.Value / rhs.Value);
		} else if (rhsType == DoubleType) {
			DoubleObject lhs, rhs;
			PopTwoSameType(rhsType, lhs, rhs);
			if (rhs.Value == 0) {
				OccurException(SVM_IEC_ARITHMETIC_DIVIDEBYZERO);
				return;
			}
			m_Stack.Push<DoubleObject>(lhs.Value / rhs.Value);
		} else {
			OccurException(SVM_IEC_STACK_EMPTY);
		}
	}
	SVM_NOINLINE_FOR_PROFILING void Interpreter::InterpretIDiv() {
		const Type** const rhsTypePtr = m_Stack.GetTopType();
		if (!rhsTypePtr) {
			OccurException(SVM_IEC_STACK_EMPTY);
			return;
		}

		const Type*& rhsType = *rhsTypePtr;
		if (rhsType == IntType) {
			IntObject lhs, rhs;
			PopTwoSameType(rhsType, lhs, rhs);
			if (rhs.Value == 0) {
				OccurException(SVM_IEC_ARITHMETIC_DIVIDEBYZERO);
				return;
			}
			m_Stack.Push<IntObject>(static_cast<std::int32_t>(lhs.Value) / static_cast<std::int32_t>(rhs.Value));
		} else if (rhsType == LongType) {
			LongObject lhs, rhs;
			PopTwoSameType(rhsType, lhs, rhs);
			if (rhs.Value == 0) {
				OccurException(SVM_IEC_ARITHMETIC_DIVIDEBYZERO);
				return;
			}
			m_Stack.Push<LongObject>(static_cast<std::int64_t>(lhs.Value) / static_cast<std::int64_t>(rhs.Value));
		} else if (rhsType == DoubleType) {
			DoubleObject lhs, rhs;
			PopTwoSameType(rhsType, lhs, rhs);
			if (rhs.Value == 0) {
				OccurException(SVM_IEC_ARITHMETIC_DIVIDEBYZERO);
				return;
			}
			m_Stack.Push<DoubleObject>(lhs.Value / rhs.Value);
		} else {
			OccurException(SVM_IEC_STACK_EMPTY);
		}
	}
	SVM_NOINLINE_FOR_PROFILING void Interpreter::InterpretMod() {
		const Type** const rhsTypePtr = m_Stack.GetTopType();
		if (!rhsTypePtr) {
			OccurException(SVM_IEC_STACK_EMPTY);
			return;
		}

		const Type*& rhsType = *rhsTypePtr;
		if (rhsType == IntType) {
			IntObject lhs, rhs;
			PopTwoSameType(rhsType, lhs, rhs);
			if (rhs.Value == 0) {
				OccurException(SVM_IEC_ARITHMETIC_DIVIDEBYZERO);
				return;
			}
			m_Stack.Push<IntObject>(lhs.Value % rhs.Value);
		} else if (rhsType == LongType) {
			LongObject lhs, rhs;
			PopTwoSameType(rhsType, lhs, rhs);
			if (rhs.Value == 0) {
				OccurException(SVM_IEC_ARITHMETIC_DIVIDEBYZERO);
				return;
			}
			m_Stack.Push<LongObject>(lhs.Value % rhs.Value);
		} else if (rhsType == DoubleType) {
			DoubleObject lhs, rhs;
			PopTwoSameType(rhsType, lhs, rhs);
			if (rhs.Value == 0) {
				OccurException(SVM_IEC_ARITHMETIC_DIVIDEBYZERO);
				return;
			}
			m_Stack.Push<DoubleObject>(std::fmod(lhs.Value, rhs.Value));
		} else {
			OccurException(SVM_IEC_STACK_EMPTY);
		}
	}
	SVM_NOINLINE_FOR_PROFILING void Interpreter::InterpretIMod() {
		const Type** const rhsTypePtr = m_Stack.GetTopType();
		if (!rhsTypePtr) {
			OccurException(SVM_IEC_STACK_EMPTY);
			return;
		}

		const Type*& rhsType = *rhsTypePtr;
		if (rhsType == IntType) {
			IntObject lhs, rhs;
			PopTwoSameType(rhsType, lhs, rhs);
			if (rhs.Value == 0) {
				OccurException(SVM_IEC_ARITHMETIC_DIVIDEBYZERO);
				return;
			}
			m_Stack.Push<IntObject>(static_cast<std::int32_t>(lhs.Value) % static_cast<std::int32_t>(rhs.Value));
		} else if (rhsType == LongType) {
			LongObject lhs, rhs;
			PopTwoSameType(rhsType, lhs, rhs);
			if (rhs.Value == 0) {
				OccurException(SVM_IEC_ARITHMETIC_DIVIDEBYZERO);
				return;
			}
			m_Stack.Push<LongObject>(static_cast<std::int64_t>(lhs.Value) % static_cast<std::int64_t>(rhs.Value));
		} else if (rhsType == DoubleType) {
			DoubleObject lhs, rhs;
			PopTwoSameType(rhsType, lhs, rhs);
			if (rhs.Value == 0) {
				OccurException(SVM_IEC_ARITHMETIC_DIVIDEBYZERO);
				return;
			}
			m_Stack.Push<DoubleObject>(std::fmod(lhs.Value, rhs.Value));
		} else {
			OccurException(SVM_IEC_STACK_EMPTY);
		}
	}
	SVM_NOINLINE_FOR_PROFILING void Interpreter::InterpretNeg() {
		const Type** const typePtr = m_Stack.GetTopType();
		if (!typePtr) {
			OccurException(SVM_IEC_STACK_EMPTY);
			return;
		}

		const Type*& type = *typePtr;
		if (type == IntType) {
			IntObject& top = reinterpret_cast<IntObject&>(type);
			top.Value = -static_cast<std::int32_t>(top.Value);
		} else if (type == LongType) {
			LongObject& top = reinterpret_cast<LongObject&>(type);
			top.Value = -static_cast<std::int64_t>(top.Value);
		} else if (type == DoubleType) {
			DoubleObject& top = reinterpret_cast<DoubleObject&>(type);
			top.Value = -top.Value;
		} else {
			OccurException(SVM_IEC_STACK_EMPTY);
		}
	}
	SVM_NOINLINE_FOR_PROFILING void Interpreter::InterpretIncDec(int delta) {
		const Type** const typePtr = m_Stack.GetTopType();
		if (!typePtr) {
			OccurException(SVM_IEC_STACK_EMPTY);
			return;
		}

		const Type*& type = *typePtr;
		if (type == IntType) {
			reinterpret_cast<IntObject&>(type).Value += delta;
		} else if (type == LongType) {
			reinterpret_cast<LongObject&>(type).Value += delta;
		} else if (type == DoubleType) {
			reinterpret_cast<DoubleObject&>(type).Value += delta;
		} else {
			OccurException(SVM_IEC_STACK_EMPTY);
		}
	}

	SVM_NOINLINE_FOR_PROFILING void Interpreter::InterpretAnd() {
		const Type** const rhsTypePtr = m_Stack.GetTopType();
		if (!rhsTypePtr) {
			OccurException(SVM_IEC_STACK_EMPTY);
			return;
		}

		const Type*& rhsType = *rhsTypePtr;
		if (rhsType == IntType) {
			IntObject lhs, rhs;
			PopTwoSameType(rhsType, lhs, rhs);
			m_Stack.Push<IntObject>(lhs.Value & rhs.Value);
		} else if (rhsType == LongType || rhsType == DoubleType) {
			LongObject lhs, rhs;
			PopTwoSameType(rhsType, lhs, rhs);
			m_Stack.Push<LongObject>(lhs.Value & rhs.Value);
			if (rhsType == DoubleType) {
				*m_Stack.GetTopType() = DoubleType;
			}
		} else {
			OccurException(SVM_IEC_STACK_EMPTY);
		}
	}
	SVM_NOINLINE_FOR_PROFILING void Interpreter::InterpretOr() {
		const Type** const rhsTypePtr = m_Stack.GetTopType();
		if (!rhsTypePtr) {
			OccurException(SVM_IEC_STACK_EMPTY);
			return;
		}

		const Type*& rhsType = *rhsTypePtr;
		if (rhsType == IntType) {
			IntObject lhs, rhs;
			PopTwoSameType(rhsType, lhs, rhs);
			m_Stack.Push<IntObject>(lhs.Value | rhs.Value);
		} else if (rhsType == LongType || rhsType == DoubleType) {
			LongObject lhs, rhs;
			PopTwoSameType(rhsType, lhs, rhs);
			m_Stack.Push<LongObject>(lhs.Value | rhs.Value);
			if (rhsType == DoubleType) {
				*m_Stack.GetTopType() = DoubleType;
			}
		} else {
			OccurException(SVM_IEC_STACK_EMPTY);
		}
	}
	SVM_NOINLINE_FOR_PROFILING void Interpreter::InterpretXor() {
		const Type** const rhsTypePtr = m_Stack.GetTopType();
		if (!rhsTypePtr) {
			OccurException(SVM_IEC_STACK_EMPTY);
			return;
		}

		const Type*& rhsType = *rhsTypePtr;
		if (rhsType == IntType) {
			IntObject lhs, rhs;
			PopTwoSameType(rhsType, lhs, rhs);
			m_Stack.Push<IntObject>(lhs.Value ^ rhs.Value);
		} else if (rhsType == LongType || rhsType == DoubleType) {
			LongObject lhs, rhs;
			PopTwoSameType(rhsType, lhs, rhs);
			m_Stack.Push<LongObject>(lhs.Value ^ rhs.Value);
			if (rhsType == DoubleType) {
				*m_Stack.GetTopType() = DoubleType;
			}
		} else {
			OccurException(SVM_IEC_STACK_EMPTY);
		}
	}
	SVM_NOINLINE_FOR_PROFILING void Interpreter::InterpretNot() {
		const Type** const typePtr = m_Stack.GetTopType();
		if (!typePtr) {
			OccurException(SVM_IEC_STACK_EMPTY);
			return;
		}

		const Type*& type = *typePtr;
		if (type == IntType) {
			IntObject& top = reinterpret_cast<IntObject&>(type);
			top.Value = ~top.Value;
		} else if (type == LongType || type == DoubleType) {
			LongObject& top = reinterpret_cast<LongObject&>(type);
			top.Value = ~top.Value;
		} else {
			OccurException(SVM_IEC_STACK_EMPTY);
		}
	}
	SVM_NOINLINE_FOR_PROFILING void Interpreter::InterpretShl() {
		const Type** const rhsTypePtr = m_Stack.GetTopType();
		if (!rhsTypePtr) {
			OccurException(SVM_IEC_STACK_EMPTY);
			return;
		}

		const Type*& rhsType = *rhsTypePtr;
		if (rhsType == IntType) {
			IntObject lhs, rhs;
			PopTwoSameType(rhsType, lhs, rhs);
			m_Stack.Push<IntObject>(lhs.Value << rhs.Value);
		} else if (rhsType == LongType || rhsType == DoubleType) {
			LongObject lhs, rhs;
			PopTwoSameType(rhsType, lhs, rhs);
			m_Stack.Push<LongObject>(lhs.Value << rhs.Value);
			if (rhsType == DoubleType) {
				*m_Stack.GetTopType() = DoubleType;
			}
		} else {
			OccurException(SVM_IEC_STACK_EMPTY);
		}
	}
	SVM_NOINLINE_FOR_PROFILING void Interpreter::InterpretSal() {
		const Type** const rhsTypePtr = m_Stack.GetTopType();
		if (!rhsTypePtr) {
			OccurException(SVM_IEC_STACK_EMPTY);
			return;
		}

		const Type*& rhsType = *rhsTypePtr;
		if (rhsType == IntType) {
			IntObject lhs, rhs;
			PopTwoSameType(rhsType, lhs, rhs);
			m_Stack.Push<IntObject>(lhs.Value << rhs.Value);
		} else if (rhsType == LongType || rhsType == DoubleType) {
			LongObject lhs, rhs;
			PopTwoSameType(rhsType, lhs, rhs);
			m_Stack.Push<LongObject>(lhs.Value << rhs.Value);
			if (rhsType == DoubleType) {
				*m_Stack.GetTopType() = DoubleType;
			}
		} else {
			OccurException(SVM_IEC_STACK_EMPTY);
		}
	}
	SVM_NOINLINE_FOR_PROFILING void Interpreter::InterpretShr() {
		const Type** const rhsTypePtr = m_Stack.GetTopType();
		if (!rhsTypePtr) {
			OccurException(SVM_IEC_STACK_EMPTY);
			return;
		}

		const Type*& rhsType = *rhsTypePtr;
		if (rhsType == IntType) {
			IntObject lhs, rhs;
			PopTwoSameType(rhsType, lhs, rhs);
			m_Stack.Push<IntObject>(lhs.Value >> rhs.Value);
		} else if (rhsType == LongType || rhsType == DoubleType) {
			LongObject lhs, rhs;
			PopTwoSameType(rhsType, lhs, rhs);
			m_Stack.Push<LongObject>(lhs.Value >> rhs.Value);
			if (rhsType == DoubleType) {
				*m_Stack.GetTopType() = DoubleType;
			}
		} else {
			OccurException(SVM_IEC_STACK_EMPTY);
		}
	}
	SVM_NOINLINE_FOR_PROFILING void Interpreter::InterpretSar() {
		const Type** const rhsTypePtr = m_Stack.GetTopType();
		if (!rhsTypePtr) {
			OccurException(SVM_IEC_STACK_EMPTY);
			return;
		}

		const Type*& rhsType = *rhsTypePtr;
		if (rhsType == IntType) {
			IntObject lhs, rhs;
			PopTwoSameType(rhsType, lhs, rhs);
			m_Stack.Push<IntObject>(static_cast<std::int32_t>(lhs.Value) >> static_cast<std::int32_t>(rhs.Value));
		} else if (rhsType == LongType || rhsType == DoubleType) {
			LongObject lhs, rhs;
			PopTwoSameType(rhsType, lhs, rhs);
			m_Stack.Push<LongObject>(static_cast<std::int64_t>(lhs.Value) >> static_cast<std::int64_t>(rhs.Value));
			if (rhsType == DoubleType) {
				*m_Stack.GetTopType() = DoubleType;
			}
		} else {
			OccurException(SVM_IEC_STACK_EMPTY);
		}
	}

	namespace {
#define CompareClass(n, o)								\
struct n final {										\
	template<typename T>								\
	static constexpr bool Compare(T value) noexcept	{	\
		return value o;									\
	}													\
};

		CompareClass(EqualZero, == 0);
		CompareClass(NotEqualZero, != 0);
		CompareClass(EqualOne, == 1);
		CompareClass(NotEqualOne, != 0);
		CompareClass(EqualMinusOne, == -1);
		CompareClass(NotEqualMinusOne, != -1);

#undef CompareClass
	}

	SVM_NOINLINE_FOR_PROFILING void Interpreter::InterpretCmp() {
		const Type** const rhsTypePtr = m_Stack.GetTopType();
		if (!rhsTypePtr) {
			OccurException(SVM_IEC_STACK_EMPTY);
			return;
		}

		const Type*& rhsType = *rhsTypePtr;
		if (rhsType == IntType) {
			IntObject lhs, rhs;
			PopTwoSameType(rhsType, lhs, rhs);
			m_Stack.Push(CompareTwoSameType(lhs.Value, rhs.Value));
		} else if (rhsType == LongType) {
			LongObject lhs, rhs;
			PopTwoSameType(rhsType, lhs, rhs);
			m_Stack.Push(CompareTwoSameType(lhs.Value, rhs.Value));
		} else if (rhsType == DoubleType) {
			DoubleObject lhs, rhs;
			PopTwoSameType(rhsType, lhs, rhs);
			m_Stack.Push(CompareTwoSameType(lhs.Value, rhs.Value));
		} else {
			OccurException(SVM_IEC_STACK_EMPTY);
		}
	}
	SVM_NOINLINE_FOR_PROFILING void Interpreter::InterpretICmp() {
		const Type** const rhsTypePtr = m_Stack.GetTopType();
		if (!rhsTypePtr) {
			OccurException(SVM_IEC_STACK_EMPTY);
			return;
		}

		const Type*& rhsType = *rhsTypePtr;
		if (rhsType == IntType) {
			IntObject lhs, rhs;
			PopTwoSameType(rhsType, lhs, rhs);
			m_Stack.Push(CompareTwoSameType<std::int32_t>(lhs.Value, rhs.Value));
		} else if (rhsType == LongType) {
			LongObject lhs, rhs;
			PopTwoSameType(rhsType, lhs, rhs);
			m_Stack.Push(CompareTwoSameType<std::int64_t>(lhs.Value, rhs.Value));
		} else if (rhsType == DoubleType) {
			DoubleObject lhs, rhs;
			PopTwoSameType(rhsType, lhs, rhs);
			m_Stack.Push(CompareTwoSameType(lhs.Value, rhs.Value));
		} else {
			OccurException(SVM_IEC_STACK_EMPTY);
		}
	}
	SVM_NOINLINE_FOR_PROFILING void Interpreter::InterpretJmp(std::uint32_t operand) {
		if (operand >= m_StackFrame.Instructions->GetLabelCount()) {
			OccurException(SVM_IEC_LABEL_OUTOFRANGE);
			return;
		}

		m_InstructionIndex = m_StackFrame.Instructions->GetLabel(operand) - 1;
	}
	SVM_NOINLINE_FOR_PROFILING void Interpreter::InterpretJe(std::uint32_t operand) {
		JumpCondition<EqualZero>(operand);
	}
	SVM_NOINLINE_FOR_PROFILING void Interpreter::InterpretJne(std::uint32_t operand) {
		JumpCondition<NotEqualZero>(operand);
	}
	SVM_NOINLINE_FOR_PROFILING void Interpreter::InterpretJa(std::uint32_t operand) {
		JumpCondition<EqualOne>(operand);
	}
	SVM_NOINLINE_FOR_PROFILING void Interpreter::InterpretJae(std::uint32_t operand) {
		JumpCondition<NotEqualMinusOne>(operand);
	}
	SVM_NOINLINE_FOR_PROFILING void Interpreter::InterpretJb(std::uint32_t operand) {
		JumpCondition<EqualMinusOne>(operand);
	}
	SVM_NOINLINE_FOR_PROFILING void Interpreter::InterpretJbe(std::uint32_t operand) {
		JumpCondition<NotEqualOne>(operand);
	}
	SVM_NOINLINE_FOR_PROFILING void Interpreter::InterpretCall(std::uint32_t operand) {
		m_StackFrame.Caller = static_cast<std::size_t>(m_InstructionIndex);
		m_Stack.Push(m_StackFrame);

		m_StackFrame = { m_Stack.GetUsedSize(), m_LocalVariables.size() };
		m_StackFrame.Function = &m_ByteFile.GetFunctions()[operand];
		m_StackFrame.Instructions = &m_StackFrame.Function->GetInstructions();

		std::size_t stackOffset = m_Stack.GetUsedSize() - sizeof(m_StackFrame);
		for (std::uint16_t j = 0; j < m_StackFrame.Function->GetArity(); ++j) {
			const Type** typePtr = m_Stack.Get<const Type*>(stackOffset);
			if (!typePtr) {
				OccurException(SVM_IEC_STACK_EMPTY);
				m_StackFrame = m_Stack.Pop<StackFrame>().value();
				return;
			}
			
			m_LocalVariables.push_back(stackOffset);

			const Type* const type = *typePtr;
			if (type == IntType) {
				stackOffset -= sizeof(IntObject);
			} else if (type == LongType) {
				stackOffset -= sizeof(LongObject);
			} else if (type == DoubleType) {
				stackOffset -= sizeof(DoubleObject);
			} else {
				OccurException(SVM_IEC_STACK_EMPTY);
				m_StackFrame = m_Stack.Pop<StackFrame>().value();
				m_LocalVariables.erase(m_LocalVariables.end() - 1);
				return;
			}
		}

		m_InstructionIndex = static_cast<std::uint64_t>(-1);
		++m_Depth;
	}
	SVM_NOINLINE_FOR_PROFILING void Interpreter::InterpretRet() {
		std::variant<std::monostate, IntObject, LongObject, DoubleObject> result;
		if (m_StackFrame.Function->HasResult()) {
			const Type** typePtr = m_Stack.GetTopType();
			if (!typePtr) {
				OccurException(SVM_IEC_STACK_EMPTY);
				return;
			}

			const Type* const type = *typePtr;
			if (type == IntType) {
				result = m_Stack.Pop<IntObject>().value();
			} else if (type == LongType) {
				result = m_Stack.Pop<LongObject>().value();
			} else if (type == DoubleType) {
				result = m_Stack.Pop<DoubleObject>().value();
			} else {
				OccurException(SVM_IEC_STACK_EMPTY);
				return;
			}
		}

		m_LocalVariables.erase(m_LocalVariables.begin() + m_StackFrame.VariableBegin, m_LocalVariables.end());

		const std::uint16_t arity = m_StackFrame.Function->GetArity();
		m_Stack.RemoveTo(m_StackFrame.StackBegin);
		m_StackFrame = m_Stack.Pop<StackFrame>().value();

		m_InstructionIndex = m_StackFrame.Caller;

		for (std::uint16_t j = 0; j < arity; ++j) {
			const Type* const type = *m_Stack.GetTopType();
			if (type == IntType) {
				m_Stack.Pop<IntObject>();
			} else if (type == LongType) {
				m_Stack.Pop<LongObject>();
			} else if (type == DoubleType) {
				m_Stack.Pop<DoubleObject>();
			}
		}

		if (std::holds_alternative<std::monostate>(result)) {
			return;
		} else if (std::holds_alternative<IntObject>(result)) {
			m_Stack.Push(std::get<IntObject>(result));
		} else if (std::holds_alternative<LongObject>(result)) {
			m_Stack.Push(std::get<LongObject>(result));
		} else if (std::holds_alternative<DoubleObject>(result)) {
			m_Stack.Push(std::get<DoubleObject>(result));
		}
	}

	SVM_NOINLINE_FOR_PROFILING void Interpreter::InterpretToI() {
		const Type** const typePtr = m_Stack.GetTopType();
		if (!typePtr) {
			OccurException(SVM_IEC_STACK_EMPTY);
			return;
		}

		const Type* const type = *typePtr;
		if (type == IntType) {
			return;
		} else if (type == LongType) {
			const LongObject value = *m_Stack.Pop<LongObject>();
			m_Stack.Push<IntObject>(static_cast<std::uint32_t>(value.Value));
		} else if (type == DoubleType) {
			const DoubleObject value = *m_Stack.Pop<DoubleObject>();
			m_Stack.Push<IntObject>(static_cast<std::uint32_t>(value.Value));
		} else {
			OccurException(SVM_IEC_STACK_EMPTY);
		}
	}
	SVM_NOINLINE_FOR_PROFILING void Interpreter::InterpretToL() {
		const Type** const typePtr = m_Stack.GetTopType();
		if (!typePtr) {
			OccurException(SVM_IEC_STACK_EMPTY);
			return;
		}

		const Type* const type = *typePtr;
		if (type == IntType) {
			const IntObject value = *m_Stack.Pop<IntObject>();
			m_Stack.Push<LongObject>(value.Value);
		} else if (type == LongType) {
			return;
		} else if (type == DoubleType) {
			const DoubleObject value = *m_Stack.Pop<DoubleObject>();
			m_Stack.Push<LongObject>(static_cast<std::uint64_t>(value.Value));
		} else {
			OccurException(SVM_IEC_STACK_EMPTY);
		}
	}
	SVM_NOINLINE_FOR_PROFILING void Interpreter::InterpretToD() {
		const Type** const typePtr = m_Stack.GetTopType();
		if (!typePtr) {
			OccurException(SVM_IEC_STACK_EMPTY);
			return;
		}

		const Type* const type = *typePtr;
		if (type == IntType) {
			const IntObject value = *m_Stack.Pop<IntObject>();
			m_Stack.Push<DoubleObject>(value.Value);
		} else if (type == LongType) {
			const LongObject value = *m_Stack.Pop<LongObject>();
			m_Stack.Push<DoubleObject>(static_cast<double>(value.Value));
		} else if (type == DoubleType) {
			return;
		} else {
			OccurException(SVM_IEC_STACK_EMPTY);
		}
	}
}