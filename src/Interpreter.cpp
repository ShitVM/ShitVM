#include <svm/Interpreter.hpp>

#include <svm/Object.hpp>

#include <cmath>
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

	const Type*& Stack::GetTopType() {
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
		: m_ByteFile(std::move(byteFile)) {}
	Interpreter::Interpreter(Interpreter&& interpreter) noexcept
		: m_ByteFile(std::move(interpreter.m_ByteFile)),
		m_Stack(std::move(interpreter.m_Stack)), m_StackFrame(interpreter.m_StackFrame), m_CallStack(std::move(interpreter.m_CallStack)),
		m_LocalVariables(std::move(interpreter.m_LocalVariables)) {}

	Interpreter& Interpreter::operator=(Interpreter&& interpreter) noexcept {
		m_ByteFile = std::move(interpreter.m_ByteFile);

		m_Stack = std::move(interpreter.m_Stack);
		m_StackFrame = interpreter.m_StackFrame;
		m_CallStack = std::move(interpreter.m_CallStack);

		m_LocalVariables = std::move(interpreter.m_LocalVariables);
		return *this;
	}

	void Interpreter::Clear() noexcept {
		m_ByteFile.Clear();

		m_Stack.Deallocate();
		m_StackFrame = {};
		m_CallStack.clear();

		m_LocalVariables.clear();
	}
	void Interpreter::Load(ByteFile&& byteFile) noexcept {
		m_ByteFile = std::move(byteFile);
	}
	void Interpreter::AllocateStack(std::size_t size) {
		m_Stack.Allocate(size);
	}
	void Interpreter::ReallocateStack(std::size_t newSize) {
		m_Stack.Reallocate(newSize);
	}

#define ConstantPool m_ByteFile.GetConstantPool()

	void Interpreter::Interpret() {
		m_StackFrame.Instructions = &m_ByteFile.GetEntryPoint();
		m_CallStack.reserve(128);

		for (std::uint64_t i = 0; i < m_StackFrame.Instructions->GetInstructionCount(); ++i) {
			const Instruction& inst = m_StackFrame.Instructions->GetInstruction(i);
			switch (inst.OpCode) {
			case OpCode::Push: InterpretPush(inst.Operand); break;
			case OpCode::Pop: InterpretPop(); break;
			case OpCode::Load: InterpretLoad(inst.Operand); break;
			case OpCode::Store: InterpretStore(inst.Operand); break;

			case OpCode::Add: InterpretAdd(); break;
			case OpCode::Sub: InterpretSub(); break;
			case OpCode::Mul: InterpretMul(); break;
			case OpCode::IMul: InterpretIMul(); break;
			case OpCode::Div: InterpretDiv(); break;
			case OpCode::IDiv: InterpretIDiv(); break;
			case OpCode::Mod: InterpretMod(); break;
			case OpCode::IMod: InterpretIMod(); break;
			case OpCode::Neg: InterpretNeg(); break;

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
			case OpCode::Jmp: InterpretJmp(i, inst.Operand); break;
			case OpCode::Je: InterpretJe(i, inst.Operand); break;
			case OpCode::Jne: InterpretJne(i, inst.Operand); break;
			case OpCode::Ja: InterpretJa(i, inst.Operand); break;
			case OpCode::Jae: InterpretJae(i, inst.Operand); break;
			case OpCode::Jb: InterpretJb(i, inst.Operand); break;
			case OpCode::Jbe: InterpretJbe(i, inst.Operand); break;
			case OpCode::Call: InterpretCall(i, inst.Operand); break;
			case OpCode::Ret: InterpretRet(i); break;

			case OpCode::I2L: InterpretI2L(); break;
			case OpCode::I2D: InterpretI2D(); break;
			case OpCode::L2I: InterpretL2I(); break;
			case OpCode::L2D: InterpretL2D(); break;
			case OpCode::D2I: InterpretD2I(); break;
			case OpCode::D2L: InterpretD2L(); break;
			}
		}
	}

	void Interpreter::InterpretPush(std::uint32_t operand) {
		const Type* const constType = ConstantPool.GetConstantType(operand);
		if (constType == IntType) {
			m_Stack.Push(ConstantPool.GetConstant<IntObject>(operand));
		} else if (constType == LongType) {
			m_Stack.Push(ConstantPool.GetConstant<LongObject>(operand));
		} else if (constType == DoubleType) {
			m_Stack.Push(ConstantPool.GetConstant<DoubleObject>(operand));
		}
	}
	void Interpreter::InterpretPop() {
		const Type* const type = m_Stack.GetTopType();
		if (type == IntType) {
			m_Stack.Pop<IntObject>();
		} else if (type == LongType) {
			m_Stack.Pop<LongObject>();
		} else if (type == DoubleType) {
			m_Stack.Pop<DoubleObject>();
		}
	}
	void Interpreter::InterpretLoad(std::uint32_t operand) {
		operand += static_cast<std::uint32_t>(m_StackFrame.VariableBegin);

		const Type*& varType = m_Stack.Get<const Type*>(m_LocalVariables[operand]);
		if (varType == IntType) {
			m_Stack.Push(*reinterpret_cast<const IntObject*>(&varType));
		} else if (varType == LongType) {
			m_Stack.Push(*reinterpret_cast<const LongObject*>(&varType));
		} else if (varType == DoubleType) {
			m_Stack.Push(*reinterpret_cast<const DoubleObject*>(&varType));
		}
	}
	void Interpreter::InterpretStore(std::uint32_t operand) {
		operand += static_cast<std::uint32_t>(m_StackFrame.VariableBegin);

		const Type* const type = m_Stack.GetTopType();
		if (type == IntType) {
			const IntObject& object = m_Stack.Get<IntObject>(m_Stack.GetUsedSize() + sizeof(IntObject));
			if (m_LocalVariables.size() <= operand) {
				m_LocalVariables.resize(operand + 1, std::numeric_limits<std::size_t>::max());
			}
			if (m_LocalVariables[operand] == std::numeric_limits<std::size_t>::max()) {
				m_Stack.Push<IntObject>({});
				m_LocalVariables[operand] = m_Stack.GetUsedSize();
			}
			m_Stack.Get<IntObject>(m_LocalVariables[operand]) = object;
		} else if (type == LongType) {
			const LongObject& object = m_Stack.Get<LongObject>(m_Stack.GetUsedSize() + sizeof(LongObject));
			if (m_LocalVariables.size() <= operand) {
				m_LocalVariables.resize(operand + 1, std::numeric_limits<std::size_t>::max());
			}
			if (m_LocalVariables[operand] == std::numeric_limits<std::size_t>::max()) {
				m_Stack.Push<LongObject>({});
				m_LocalVariables[operand] = m_Stack.GetUsedSize();
			}
			m_Stack.Get<LongObject>(m_LocalVariables[operand]) = object;
		} else if (type == DoubleType) {
			const DoubleObject& object = m_Stack.Get<DoubleObject>(m_Stack.GetUsedSize() + sizeof(DoubleObject));
			if (m_LocalVariables.size() <= operand) {
				m_LocalVariables.resize(operand + 1, std::numeric_limits<std::size_t>::max());
			}
			if (m_LocalVariables[operand] == std::numeric_limits<std::size_t>::max()) {
				m_Stack.Push<DoubleObject>({});
				m_LocalVariables[operand] = m_Stack.GetUsedSize();
			}
			m_Stack.Get<DoubleObject>(m_LocalVariables[operand]) = object;
		}
	}

	void Interpreter::InterpretAdd() {
		const Type* const rhsType = m_Stack.GetTopType();
		if (rhsType == IntType) {
			const IntObject rhs = m_Stack.Pop<IntObject>();
			const IntObject lhs = m_Stack.Pop<IntObject>();
			const IntObject result = lhs.Value + rhs.Value;
			m_Stack.Push(result);
		} else if (rhsType == LongType) {
			const LongObject rhs = m_Stack.Pop<LongObject>();
			const LongObject lhs = m_Stack.Pop<LongObject>();
			const LongObject result = lhs.Value + rhs.Value;
			m_Stack.Push(result);
		} else if (rhsType == DoubleType) {
			const DoubleObject rhs = m_Stack.Pop<DoubleObject>();
			const DoubleObject lhs = m_Stack.Pop<DoubleObject>();
			const DoubleObject result = lhs.Value + rhs.Value;
			m_Stack.Push(result);
		}
	}
	void Interpreter::InterpretSub() {
		const Type* const rhsType = m_Stack.GetTopType();
		if (rhsType == IntType) {
			const IntObject rhs = m_Stack.Pop<IntObject>();
			const IntObject lhs = m_Stack.Pop<IntObject>();
			const IntObject result = lhs.Value - rhs.Value;
			m_Stack.Push(result);
		} else if (rhsType == LongType) {
			const LongObject rhs = m_Stack.Pop<LongObject>();
			const LongObject lhs = m_Stack.Pop<LongObject>();
			const LongObject result = lhs.Value - rhs.Value;
			m_Stack.Push(result);
		} else if (rhsType == DoubleType) {
			const DoubleObject rhs = m_Stack.Pop<DoubleObject>();
			const DoubleObject lhs = m_Stack.Pop<DoubleObject>();
			const DoubleObject result = lhs.Value - rhs.Value;
			m_Stack.Push(result);
		}
	}
	void Interpreter::InterpretMul() {
		const Type* const rhsType = m_Stack.GetTopType();
		if (rhsType == IntType) {
			const IntObject rhs = m_Stack.Pop<IntObject>();
			const IntObject lhs = m_Stack.Pop<IntObject>();
			const IntObject result = lhs.Value * rhs.Value;
			m_Stack.Push(result);
		} else if (rhsType == LongType) {
			const LongObject rhs = m_Stack.Pop<LongObject>();
			const LongObject lhs = m_Stack.Pop<LongObject>();
			const LongObject result = lhs.Value * rhs.Value;
			m_Stack.Push(result);
		} else if (rhsType == DoubleType) {
			const DoubleObject rhs = m_Stack.Pop<DoubleObject>();
			const DoubleObject lhs = m_Stack.Pop<DoubleObject>();
			const DoubleObject result = lhs.Value * rhs.Value;
			m_Stack.Push(result);
		}
	}
	void Interpreter::InterpretIMul() {
		const Type* const rhsType = m_Stack.GetTopType();
		if (rhsType == IntType) {
			const std::int32_t rhs = m_Stack.Pop<IntObject>().Value;
			const std::int32_t lhs = m_Stack.Pop<IntObject>().Value;
			const IntObject result = lhs + rhs;
			m_Stack.Push(result);
		} else if (rhsType == LongType) {
			const std::int64_t rhs = m_Stack.Pop<LongObject>().Value;
			const std::int64_t lhs = m_Stack.Pop<LongObject>().Value;
			const LongObject result = lhs + rhs;
			m_Stack.Push(result);
		} else if (rhsType == DoubleType) {
			const DoubleObject rhs = m_Stack.Pop<DoubleObject>();
			const DoubleObject lhs = m_Stack.Pop<DoubleObject>();
			const DoubleObject result = lhs.Value + rhs.Value;
			m_Stack.Push(result);
		}
	}
	void Interpreter::InterpretDiv() {
		const Type* const rhsType = m_Stack.GetTopType();
		if (rhsType == IntType) {
			const IntObject rhs = m_Stack.Pop<IntObject>();
			const IntObject lhs = m_Stack.Pop<IntObject>();
			const IntObject result = lhs.Value / rhs.Value;
			m_Stack.Push(result);
		} else if (rhsType == LongType) {
			const LongObject rhs = m_Stack.Pop<LongObject>();
			const LongObject lhs = m_Stack.Pop<LongObject>();
			const LongObject result = lhs.Value / rhs.Value;
			m_Stack.Push(result);
		} else if (rhsType == DoubleType) {
			const DoubleObject rhs = m_Stack.Pop<DoubleObject>();
			const DoubleObject lhs = m_Stack.Pop<DoubleObject>();
			const DoubleObject result = lhs.Value / rhs.Value;
			m_Stack.Push(result);
		}
	}
	void Interpreter::InterpretIDiv() {
		const Type* const rhsType = m_Stack.GetTopType();
		if (rhsType == IntType) {
			const std::int32_t rhs = m_Stack.Pop<IntObject>().Value;
			const std::int32_t lhs = m_Stack.Pop<IntObject>().Value;
			const IntObject result = lhs / rhs;
			m_Stack.Push(result);
		} else if (rhsType == LongType) {
			const std::int64_t rhs = m_Stack.Pop<LongObject>().Value;
			const std::int64_t lhs = m_Stack.Pop<LongObject>().Value;
			const LongObject result = lhs / rhs;
			m_Stack.Push(result);
		} else if (rhsType == DoubleType) {
			const DoubleObject rhs = m_Stack.Pop<DoubleObject>();
			const DoubleObject lhs = m_Stack.Pop<DoubleObject>();
			const DoubleObject result = lhs.Value + rhs.Value;
			m_Stack.Push(result);
		}
	}
	void Interpreter::InterpretMod() {
		const Type* const rhsType = m_Stack.GetTopType();
		if (rhsType == IntType) {
			const IntObject rhs = m_Stack.Pop<IntObject>();
			const IntObject lhs = m_Stack.Pop<IntObject>();
			const IntObject result = lhs.Value % rhs.Value;
			m_Stack.Push(result);
		} else if (rhsType == LongType) {
			const LongObject rhs = m_Stack.Pop<LongObject>();
			const LongObject lhs = m_Stack.Pop<LongObject>();
			const LongObject result = lhs.Value % rhs.Value;
			m_Stack.Push(result);
		} else if (rhsType == DoubleType) {
			const DoubleObject rhs = m_Stack.Pop<DoubleObject>();
			const DoubleObject lhs = m_Stack.Pop<DoubleObject>();
			const DoubleObject result = std::fmod(lhs.Value, rhs.Value);
			m_Stack.Push(result);
		}
	}
	void Interpreter::InterpretIMod() {
		const Type* const rhsType = m_Stack.GetTopType();
		if (rhsType == IntType) {
			const std::int32_t rhs = m_Stack.Pop<IntObject>().Value;
			const std::int32_t lhs = m_Stack.Pop<IntObject>().Value;
			const IntObject result = lhs % rhs;
			m_Stack.Push(result);
		} else if (rhsType == LongType) {
			const std::int64_t rhs = m_Stack.Pop<LongObject>().Value;
			const std::int64_t lhs = m_Stack.Pop<LongObject>().Value;
			const LongObject result = lhs % rhs;
			m_Stack.Push(result);
		} else if (rhsType == DoubleType) {
			const DoubleObject rhs = m_Stack.Pop<DoubleObject>();
			const DoubleObject lhs = m_Stack.Pop<DoubleObject>();
			const DoubleObject result = std::fmod(lhs.Value, rhs.Value);
			m_Stack.Push(result);
		}
	}
	void Interpreter::InterpretNeg() {
		const Type* const type = m_Stack.GetTopType();
		if (type == IntType) {
			IntObject& top = m_Stack.GetTop<IntObject>();
			top.Value = -static_cast<std::int32_t>(top.Value);
		} else if (type == LongType) {
			LongObject& top = m_Stack.GetTop<LongObject>();
			top.Value = -static_cast<std::int64_t>(top.Value);
		} else if (type == DoubleType) {
			DoubleObject& top = m_Stack.GetTop<DoubleObject>();
			top.Value = -top.Value;
		}
	}

	void Interpreter::InterpretAnd() {
		const Type* const rhsType = m_Stack.GetTopType();
		if (rhsType == IntType) {
			const IntObject rhs = m_Stack.Pop<IntObject>();
			const IntObject lhs = m_Stack.Pop<IntObject>();
			const IntObject result = lhs.Value & rhs.Value;
			m_Stack.Push(result);
		} else if (rhsType == LongType || rhsType == DoubleType) {
			const LongObject rhs = m_Stack.Pop<LongObject>();
			const LongObject lhs = m_Stack.Pop<LongObject>();
			const LongObject result = lhs.Value & rhs.Value;
			m_Stack.Push(result);
		}
	}
	void Interpreter::InterpretOr() {
		const Type* const rhsType = m_Stack.GetTopType();
		if (rhsType == IntType) {
			const IntObject rhs = m_Stack.Pop<IntObject>();
			const IntObject lhs = m_Stack.Pop<IntObject>();
			const IntObject result = lhs.Value | rhs.Value;
			m_Stack.Push(result);
		} else if (rhsType == LongType || rhsType == DoubleType) {
			const LongObject rhs = m_Stack.Pop<LongObject>();
			const LongObject lhs = m_Stack.Pop<LongObject>();
			const LongObject result = lhs.Value | rhs.Value;
			m_Stack.Push(result);
		}
	}
	void Interpreter::InterpretXor() {
		const Type* const rhsType = m_Stack.GetTopType();
		if (rhsType == IntType) {
			const IntObject rhs = m_Stack.Pop<IntObject>();
			const IntObject lhs = m_Stack.Pop<IntObject>();
			const IntObject result = lhs.Value ^ rhs.Value;
			m_Stack.Push(result);
		} else if (rhsType == LongType || rhsType == DoubleType) {
			const LongObject rhs = m_Stack.Pop<LongObject>();
			const LongObject lhs = m_Stack.Pop<LongObject>();
			const LongObject result = lhs.Value ^ rhs.Value;
			m_Stack.Push(result);
		}
	}
	void Interpreter::InterpretNot() {
		const Type* const type = m_Stack.GetTopType();
		if (type == IntType) {
			IntObject& top = m_Stack.GetTop<IntObject>();
			top.Value = ~top.Value;
		} else if (type == LongType || type == DoubleType) {
			LongObject& top = m_Stack.GetTop<LongObject>();
			top.Value = ~top.Value;
		}
	}
	void Interpreter::InterpretShl() {
		const Type* const rhsType = m_Stack.GetTopType();
		if (rhsType == IntType) {
			const IntObject rhs = m_Stack.Pop<IntObject>();
			const IntObject lhs = m_Stack.Pop<IntObject>();
			const IntObject result = lhs.Value << rhs.Value;
			m_Stack.Push(result);
		} else if (rhsType == LongType || rhsType == DoubleType) {
			const LongObject rhs = m_Stack.Pop<LongObject>();
			const LongObject lhs = m_Stack.Pop<LongObject>();
			const LongObject result = lhs.Value << rhs.Value;
			m_Stack.Push(result);
		}
	}
	void Interpreter::InterpretSal() {
		const Type* const rhsType = m_Stack.GetTopType();
		if (rhsType == IntType) {
			const std::int32_t rhs = m_Stack.Pop<IntObject>().Value;
			const std::int32_t lhs = m_Stack.Pop<IntObject>().Value;
			const IntObject result = lhs << rhs;
			m_Stack.Push(result);
		} else if (rhsType == LongType || rhsType == DoubleType) {
			const std::int64_t rhs = m_Stack.Pop<LongObject>().Value;
			const std::int64_t lhs = m_Stack.Pop<LongObject>().Value;
			const LongObject result = lhs << rhs;
			m_Stack.Push(result);
		}
	}
	void Interpreter::InterpretShr() {
		const Type* const rhsType = m_Stack.GetTopType();
		if (rhsType == IntType) {
			const IntObject rhs = m_Stack.Pop<IntObject>();
			const IntObject lhs = m_Stack.Pop<IntObject>();
			const IntObject result = lhs.Value >> rhs.Value;
			m_Stack.Push(result);
		} else if (rhsType == LongType || rhsType == DoubleType) {
			const LongObject rhs = m_Stack.Pop<LongObject>();
			const LongObject lhs = m_Stack.Pop<LongObject>();
			const LongObject result = lhs.Value >> rhs.Value;
			m_Stack.Push(result);
		}
	}
	void Interpreter::InterpretSar() {
		const Type* const rhsType = m_Stack.GetTopType();
		if (rhsType == IntType) {
			const std::int32_t rhs = m_Stack.Pop<IntObject>().Value;
			const std::int32_t lhs = m_Stack.Pop<IntObject>().Value;
			const IntObject result = lhs >> rhs;
			m_Stack.Push(result);
		} else if (rhsType == LongType || rhsType == DoubleType) {
			const std::int64_t rhs = m_Stack.Pop<LongObject>().Value;
			const std::int64_t lhs = m_Stack.Pop<LongObject>().Value;
			const LongObject result = lhs >> rhs;
			m_Stack.Push(result);
		}
	}

	void Interpreter::InterpretCmp() {
		const Type* const rhsType = m_Stack.GetTopType();
		if (rhsType == IntType) {
			const IntObject rhs = m_Stack.Pop<IntObject>();
			const IntObject lhs = m_Stack.Pop<IntObject>();
			const IntObject result = lhs.Value > rhs.Value ? 1 : (lhs.Value < rhs.Value ? -1 : 0);
			m_Stack.Push(result);
		} else if (rhsType == LongType) {
			const LongObject rhs = m_Stack.Pop<LongObject>();
			const LongObject lhs = m_Stack.Pop<LongObject>();
			const IntObject result = lhs.Value > rhs.Value ? 1 : (lhs.Value < rhs.Value ? -1 : 0);
			m_Stack.Push(result);
		} else if (rhsType == DoubleType) {
			const DoubleObject rhs = m_Stack.Pop<DoubleObject>();
			const DoubleObject lhs = m_Stack.Pop<DoubleObject>();
			const IntObject result = lhs.Value > rhs.Value ? 1 : (lhs.Value < rhs.Value ? -1 : 0);
			m_Stack.Push(result);
		}
	}
	void Interpreter::InterpretICmp() {
		const Type* const rhsType = m_Stack.GetTopType();
		if (rhsType == IntType) {
			const std::int32_t rhs = m_Stack.Pop<IntObject>().Value;
			const std::int32_t lhs = m_Stack.Pop<IntObject>().Value;
			const IntObject result = lhs > rhs ? 1 : (lhs < rhs ? -1 : 0);
			m_Stack.Push(result);
		} else if (rhsType == LongType) {
			const std::int64_t rhs = m_Stack.Pop<LongObject>().Value;
			const std::int64_t lhs = m_Stack.Pop<LongObject>().Value;
			const IntObject result = lhs > rhs ? 1 : (lhs < rhs ? -1 : 0);
			m_Stack.Push(result);
		} else if (rhsType == DoubleType) {
			const DoubleObject rhs = m_Stack.Pop<DoubleObject>();
			const DoubleObject lhs = m_Stack.Pop<DoubleObject>();
			const IntObject result = lhs.Value > rhs.Value ? 1 : (lhs.Value < rhs.Value ? -1 : 0);
			m_Stack.Push(result);
		}
	}
	void Interpreter::InterpretJmp(std::uint64_t& i, std::uint32_t operand) {
		i = m_StackFrame.Instructions->GetLabel(operand) - 1;
	}
	void Interpreter::InterpretJe(std::uint64_t& i, std::uint32_t operand) {
		const IntObject& value = m_Stack.GetTop<IntObject>();
		if (value.Value == 0) {
			i = m_StackFrame.Instructions->GetLabel(operand) - 1;
			m_Stack.Pop<IntObject>();
		}
	}
	void Interpreter::InterpretJne(std::uint64_t& i, std::uint32_t operand) {
		const IntObject& value = m_Stack.GetTop<IntObject>();
		if (value.Value != 0) {
			i = m_StackFrame.Instructions->GetLabel(operand) - 1;
			m_Stack.Pop<IntObject>();
		}
	}
	void Interpreter::InterpretJa(std::uint64_t& i, std::uint32_t operand) {
		const IntObject& value = m_Stack.GetTop<IntObject>();
		if (value.Value == 1) {
			i = m_StackFrame.Instructions->GetLabel(operand) - 1;
			m_Stack.Pop<IntObject>();
		}
	}
	void Interpreter::InterpretJae(std::uint64_t& i, std::uint32_t operand) {
		const IntObject& value = m_Stack.GetTop<IntObject>();
		if (value.Value != -1) {
			i = m_StackFrame.Instructions->GetLabel(operand) - 1;
			m_Stack.Pop<IntObject>();
		}
	}
	void Interpreter::InterpretJb(std::uint64_t& i, std::uint32_t operand) {
		const IntObject& value = m_Stack.GetTop<IntObject>();
		if (value.Value == -1) {
			i = m_StackFrame.Instructions->GetLabel(operand) - 1;
			m_Stack.Pop<IntObject>();
		}
	}
	void Interpreter::InterpretJbe(std::uint64_t& i, std::uint32_t operand) {
		const IntObject& value = m_Stack.GetTop<IntObject>();
		if (value.Value != 1) {
			i = m_StackFrame.Instructions->GetLabel(operand) - 1;
			m_Stack.Pop<IntObject>();
		}
	}
	SVM_INLINE void Interpreter::InterpretCall(std::uint64_t& i, std::uint32_t operand) {
		m_StackFrame.Caller = static_cast<std::size_t>(i);
		m_CallStack.push_back(m_StackFrame);

		m_StackFrame = { m_Stack.GetUsedSize(), m_LocalVariables.size() };
		m_StackFrame.Function = &m_ByteFile.GetFunctions()[operand];
		m_StackFrame.Instructions = &m_StackFrame.Function->GetInstructions();

		std::size_t stackOffset = m_Stack.GetUsedSize();
		for (std::uint16_t j = 0; j < m_StackFrame.Function->GetArity(); ++j) {
			const Type* type = m_Stack.Get<const Type*>(stackOffset);
			m_LocalVariables.push_back(stackOffset);
			if (type == IntType) {
				stackOffset -= sizeof(IntObject);
			} else if (type == LongType) {
				stackOffset -= sizeof(LongObject);
			} else if (type == DoubleType) {
				stackOffset -= sizeof(DoubleObject);
			}
		}

		i = static_cast<std::uint64_t>(-1);
	}
	SVM_INLINE void Interpreter::InterpretRet(std::uint64_t& i) {
		std::variant<std::monostate, IntObject, LongObject, DoubleObject> result;
		if (m_StackFrame.Function->HasResult()) {
			const Type* type = m_Stack.GetTopType();
			if (type == IntType) {
				result = m_Stack.Pop<IntObject>();
			} else if (type == LongType) {
				result = m_Stack.Pop<LongObject>();
			} else if (type == DoubleType) {
				result = m_Stack.Pop<DoubleObject>();
			}
		}

		m_LocalVariables.erase(m_LocalVariables.begin() + m_StackFrame.VariableBegin, m_LocalVariables.end());

		const std::uint16_t arity = m_StackFrame.Function->GetArity();
		m_Stack.RemoveTo(m_StackFrame.StackBegin);
		m_StackFrame = m_CallStack.back();
		m_CallStack.pop_back();

		i = m_StackFrame.Caller;

		for (std::uint16_t j = 0; j < arity; ++j) {
			const Type* type = m_Stack.GetTopType();
			if (type == IntType) {
				m_Stack.Pop<IntObject>();
			} else if (type == LongType) {
				m_Stack.Pop<LongObject>();
			} else if (type == DoubleType) {
				m_Stack.Pop<DoubleObject>();
			}
		}

		if (std::holds_alternative<IntObject>(result)) {
			m_Stack.Push(std::get<IntObject>(result));
		} else if (std::holds_alternative<LongObject>(result)) {
			m_Stack.Push(std::get<LongObject>(result));
		} else if (std::holds_alternative<DoubleObject>(result)) {
			m_Stack.Push(std::get<DoubleObject>(result));
		}
	}

	void Interpreter::InterpretI2L() {
		const IntObject value = m_Stack.Pop<IntObject>();
		m_Stack.Push<LongObject>(value.Value);
	}
	void Interpreter::InterpretI2D() {
		const IntObject value = m_Stack.Pop<IntObject>();
		m_Stack.Push<DoubleObject>(value.Value);
	}
	void Interpreter::InterpretL2I() {
		const LongObject value = m_Stack.Pop<LongObject>();
		m_Stack.Push<IntObject>(static_cast<std::uint32_t>(value.Value));
	}
	void Interpreter::InterpretL2D() {
		const LongObject value = m_Stack.Pop<LongObject>();
		m_Stack.Push<DoubleObject>(static_cast<double>(value.Value));
	}
	void Interpreter::InterpretD2I() {
		const DoubleObject value = m_Stack.Pop<DoubleObject>();
		m_Stack.Push<IntObject>(static_cast<std::uint32_t>(value.Value));
	}
	void Interpreter::InterpretD2L() {
		const DoubleObject value = m_Stack.Pop<DoubleObject>();
		m_Stack.Push<LongObject>(static_cast<std::uint64_t>(value.Value));
	}
}