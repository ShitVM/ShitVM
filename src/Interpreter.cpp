#include <svm/Interpreter.hpp>

#include <svm/Object.hpp>
#include <svm/detail/InterpreterExceptionCode.hpp>

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

	const Type* const* Stack::GetTopType() const noexcept {
		return Get<const Type*>(m_Used);
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
		m_Stack(std::move(interpreter.m_Stack)), m_StackFrame(interpreter.m_StackFrame), m_Depth(interpreter.m_Depth), m_InstructionIndex(interpreter.m_InstructionIndex),
		m_LocalVariables(std::move(interpreter.m_LocalVariables)),
		m_Exception(std::move(interpreter.m_Exception)) {
		interpreter.m_Depth = 0;
		interpreter.m_InstructionIndex = 0;
	}

	Interpreter& Interpreter::operator=(Interpreter&& interpreter) noexcept {
		m_ByteFile = std::move(interpreter.m_ByteFile);

		m_Stack = std::move(interpreter.m_Stack);
		m_StackFrame = interpreter.m_StackFrame;
		m_Depth = interpreter.m_Depth;
		m_InstructionIndex = interpreter.m_InstructionIndex;

		m_LocalVariables = std::move(interpreter.m_LocalVariables);

		m_Exception = std::move(interpreter.m_Exception);

		interpreter.m_StackFrame = {};
		interpreter.m_Depth = 0;
		interpreter.m_InstructionIndex = 0;
		return *this;
	}

	void Interpreter::Clear() noexcept {
		m_ByteFile.Clear();

		m_Stack.Deallocate();
		m_StackFrame = {};
		m_Depth = 0;
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
	const ByteFile& Interpreter::GetByteFile() const noexcept {
		return m_ByteFile;
	}

	bool Interpreter::Interpret() {
		for (; m_InstructionIndex < m_StackFrame.Instructions->GetInstructionCount(); ++m_InstructionIndex) {
			const Instruction& inst = m_StackFrame.Instructions->GetInstruction(m_InstructionIndex);
			switch (inst.OpCode) {
			case OpCode::Push: InterpretPush(inst.Operand); break;
			case OpCode::Pop: InterpretPop(); break;
			case OpCode::Load: InterpretLoad(inst.Operand); break;
			case OpCode::Store: InterpretStore(inst.Operand); break;
			case OpCode::TLoad: InterpretTLoad(); break;
			case OpCode::TStore: InterpretTStore(); break;
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

		if (m_Depth != 0) {
			OccurException(SVM_IEC_FUNCTION_NORETINSTRUCTION);
			return false;
		} else return true;
	}
	const InterpreterException& Interpreter::GetException() const noexcept {
		return *m_Exception;
	}
	Interpreter::Result Interpreter::GetResult() const noexcept {
		const Type* const* const typePtr = m_Stack.GetTopType();
		if (!typePtr) return std::monostate();

		const Type* const type = *typePtr;
		if (type == IntType) {
			return m_Stack.GetTop<IntObject>()->Value;
		} else if (type == LongType) {
			return m_Stack.GetTop<LongObject>()->Value;
		} else if (type == DoubleType) {
			return m_Stack.GetTop<DoubleObject>()->Value;
		} else return std::monostate();
	}
	std::vector<StackFrame> Interpreter::GetCallStacks() const {
		std::vector<StackFrame> result(m_Depth + 1);
		result[0] = m_StackFrame;

		const StackFrame* frame = &m_StackFrame;
		std::size_t stackOffset;
		for (std::size_t i = 0; i < m_Depth; ++i) {
			stackOffset = frame->StackBegin;
			for (std::uint16_t j = 0; j < frame->Function->GetArity(); ++j) {
				const Type* const type = *m_Stack.Get<const Type*>(stackOffset);
				if (type == IntType) {
					stackOffset -= sizeof(IntObject);
				} else if (type == LongType) {
					stackOffset -= sizeof(LongObject);
				} else if (type == DoubleType) {
					stackOffset -= sizeof(DoubleObject);
				}
			}
			frame = m_Stack.Get<StackFrame>(stackOffset);
			result[i + 1] = *frame;
		}

		return result;
	}

	void Interpreter::OccurException(std::uint32_t code) noexcept {
		InterpreterException& e = m_Exception.emplace();
		e.Function = m_StackFrame.Function;
		e.Instructions = m_StackFrame.Instructions;
		e.InstructionIndex = m_InstructionIndex;

		e.Code = code;
	}
}