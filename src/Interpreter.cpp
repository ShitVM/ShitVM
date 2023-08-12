#include <svm/Interpreter.hpp>

#include <svm/Object.hpp>
#include <svm/core/ByteFile.hpp>
#include <svm/detail/InterpreterExceptionCode.hpp>

#include <utility>

namespace svm {
	Interpreter::Interpreter(Loader&& loader, Module program) noexcept
		: m_Loader(std::move(loader)), m_Program(&static_cast<const ByteFile&>(std::get<core::ByteFile>(program->Module))) {
		m_StackFrame.Instructions = &m_Program->GetEntrypoint();
	}
	Interpreter::Interpreter(Interpreter&& interpreter) noexcept
		: m_Loader(std::move(interpreter.m_Loader)), m_Program(interpreter.m_Program), m_Exception(std::move(interpreter.m_Exception)),
		m_Stack(std::move(interpreter.m_Stack)), m_StackFrame(interpreter.m_StackFrame), m_Depth(interpreter.m_Depth),
		m_StructureCodeOffset(interpreter.m_StructureCodeOffset), m_LocalVariables(std::move(interpreter.m_LocalVariables)),
		m_Heap(std::move(interpreter.m_Heap)) {}

	Interpreter& Interpreter::operator=(Interpreter&& interpreter) noexcept {
		m_Loader = std::move(interpreter.m_Loader);
		m_Program = interpreter.m_Program;
		m_Exception = std::move(interpreter.m_Exception);

		m_Stack = std::move(interpreter.m_Stack);
		m_StackFrame = interpreter.m_StackFrame;
		m_Depth = interpreter.m_Depth;

		m_StructureCodeOffset = interpreter.m_StructureCodeOffset;
		m_LocalVariables = std::move(interpreter.m_LocalVariables);

		m_Heap = std::move(interpreter.m_Heap);
		return *this;
	}

	void Interpreter::Clear() noexcept {
		m_Loader.Clear();
		m_Exception.reset();

		m_Stack.Deallocate();
		m_StackFrame = {};
		m_Depth = 0;

		m_StructureCodeOffset = 0;
		m_LocalVariables.clear();

		m_Heap.Deallocate();
	}
	void Interpreter::Load(Loader&& loader, Module program) noexcept {
		m_Loader = std::move(loader);
		m_Program = &static_cast<const ByteFile&>(std::get<core::ByteFile>(program->Module));
		m_StackFrame.Instructions = &m_Program->GetEntrypoint();
	}

	void Interpreter::AllocateStack(std::size_t size) {
		m_Stack.Allocate(size);
	}
	void Interpreter::ReallocateStack(std::size_t newSize) {
		m_Stack.Reallocate(newSize);
	}
	void Interpreter::SetGarbageCollector(std::unique_ptr<GarbageCollector>&& gc) noexcept {
		m_Heap.SetGarbageCollector(std::move(gc));
	}

	bool Interpreter::Interpret() {
		for (; m_StackFrame.Caller < m_StackFrame.Instructions->GetInstructionCount(); ++m_StackFrame.Caller) {
			const Instruction& inst = m_StackFrame.Instructions->GetInstruction(m_StackFrame.Caller);
			switch (inst.OpCode) {
			case OpCode::Push: InterpretPush(inst.Operand); break;
			case OpCode::Pop: InterpretPop(); break;
			case OpCode::Load: InterpretLoad(inst.Operand); break;
			case OpCode::Store: InterpretStore(inst.Operand); break;
			case OpCode::Lea: InterpretLea(inst.Operand); break;
			case OpCode::FLea: InterpretFLea(inst.Operand); break;
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

			case OpCode::Null: InterpretNull(); break;
			case OpCode::New: InterpretNew(inst.Operand); break;
			case OpCode::Delete: InterpretDelete(); break;
			case OpCode::GCNull: InterpretGCNull(); break;
			case OpCode::GCNew: InterpretGCNew(inst.Operand); break;

			case OpCode::APush: InterpretAPush(inst.Operand); break;
			case OpCode::ANew: InterpretANew(inst.Operand); break;
			case OpCode::AGCNew: InterpretAGCNew(inst.Operand); break;
			case OpCode::ALea: InterpretALea(); break;
			case OpCode::Count: InterpretCount(); break;
			}

			if (m_Exception.has_value()) return false;
		}

		if (m_Depth != 0) {
			OccurException(SVM_IEC_FUNCTION_NORETINSTRUCTION);
			return false;
		} else return true;
	}
	bool Interpreter::HasResult() const noexcept {
		return m_Stack.GetUsedSize();
	}
	const Object* Interpreter::GetResult() const noexcept {
		return m_Stack.GetTop<Object>();
	}
	void Interpreter::PrintObject(std::ostream& stream, const Object& object) const {
		PrintObject(stream, object, false);
	}
	void Interpreter::PrintObject(std::ostream& stream, const Object& object, bool printPointerTarget) const {
		const Type type = object.GetType();
		if (type.IsFundamentalType()) {
			if (type == IntType) {
				stream << static_cast<const IntObject&>(object).Value;
			} else if (type == LongType) {
				stream << static_cast<const LongObject&>(object).Value;
			} else if (type == DoubleType) {
				stream << static_cast<const DoubleObject&>(object).Value;
			} else if (type == PointerType || type == GCPointerType) {
				stream << static_cast<const PointerObject&>(object).Value;
				if (printPointerTarget) {
					PrintPointerTaget(stream, object);
				}
			}
			return;
		} else if (type.IsStructure()) {
			const Structure structure = GetStructure(type);
			const std::uint32_t fieldCount = static_cast<std::uint32_t>(structure->Fields.size());

			stream << type->Name << '(';

			for (std::uint32_t i = 0; i < fieldCount; ++i) {
				const Field& field = structure->Fields[i];

				if (i != 0) {
					stream << ", ";
				}
				PrintObject(stream, reinterpret_cast<const Object*>(reinterpret_cast<const std::uint8_t*>(&object) + field.Offset), printPointerTarget);
			}

			stream << ')';
		} else if (type.IsArray()) {
			const ArrayObject& array = static_cast<const ArrayObject&>(object);
			const Type* const elementType = reinterpret_cast<const Type*>(&array + 1);
			stream << elementType->GetReference().Name << '[' << array.Count << "]{";

			for (std::uint64_t i = 0; i < array.Count; ++i) {
				if (i != 0) {
					stream << ", ";
				}
				PrintObject(stream, reinterpret_cast<const Object*>(reinterpret_cast<const std::uint8_t*>(elementType) + i * elementType->GetReference().Size));
			}

			stream << '}';
		}
	}
	void Interpreter::PrintObject(std::ostream& stream, const Object* object) const {
		PrintObject(stream, *object, false);
	}
	void Interpreter::PrintObject(std::ostream& stream, const Object* object, bool printPointerTarget) const {
		PrintObject(stream, *object, printPointerTarget);
	}

	bool Interpreter::HasException() const noexcept {
		return m_Exception.has_value();
	}
	const InterpreterException& Interpreter::GetException() const noexcept {
		return *m_Exception;
	}
	std::vector<StackFrame> Interpreter::GetCallStacks() const {
		std::vector<StackFrame> result(m_Depth + 1);
		result[0] = m_StackFrame;

		const StackFrame* frame = &m_StackFrame;
		std::size_t stackOffset;
		for (std::size_t i = 0; i < m_Depth; ++i) {
			stackOffset = frame->StackBegin;
			while (true) {
				const Type type = *m_Stack.Get<Type>(stackOffset);
				if (!type.IsValidType()) break;
				stackOffset -= type->Size;
			}
			frame = m_Stack.Get<StackFrame>(stackOffset);
			result[i + 1] = *frame;
		}

		if (HasException()) {
			result[0].Caller = static_cast<std::size_t>(m_Exception->InstructionIndex);
		}
		return result;
	}

	Structure Interpreter::GetStructure(Type type) const noexcept {
		return m_Loader.GetModule(type->Module)->GetStructure(static_cast<std::uint32_t>(type->Code) - static_cast<std::uint32_t>(TypeCode::Structure));
	}
	Structure Interpreter::GetStructure(TypeCode code) const noexcept {
		std::uint32_t index = static_cast<std::uint32_t>(code) - static_cast<std::uint32_t>(TypeCode::Structure) - m_StructureCodeOffset;

		const Structures& structures = m_Program->GetStructures();
		const std::uint32_t structCount = static_cast<std::uint32_t>(structures.size());
		if (index < structCount) return structures[index];

		index -= structCount;
		const Mappings& mappings = m_Program->GetMappings();
		if (index >= mappings.GetStructureMappingCount()) return nullptr;

		const Mapping& mapping = mappings.GetStructureMapping(index);
		return m_Loader.GetModule(mapping.Module)->GetStructure(mapping.Name);
	}
	std::uint32_t Interpreter::GetStructureCount() const noexcept {
		return static_cast<std::uint32_t>(m_Program->GetStructures().size()) + m_Program->GetMappings().GetStructureMappingCount();
	}
	std::variant<std::monostate, Function, VirtualFunction> Interpreter::GetFunction(std::uint32_t index) const noexcept {
		const Functions& functions = m_Program->GetFunctions();
		const std::uint32_t funcCount = static_cast<std::uint32_t>(functions.size());
		if (index < funcCount) return functions[index];

		index -= funcCount;
		const Mappings& mappings = m_Program->GetMappings();
		if (index >= mappings.GetFunctionMappingCount()) return std::monostate();

		const Mapping& mapping = mappings.GetFunctionMapping(index);
		const auto result = m_Loader.GetModule(mapping.Module + 1)->GetFunction(mapping.Name);

		if (std::holds_alternative<Function>(result)) return std::get<Function>(result);
		else return std::get<VirtualFunction>(result);
	}
	std::uint32_t Interpreter::GetFunctionCount() const noexcept {
		return static_cast<std::uint32_t>(m_Program->GetFunctions().size()) + m_Program->GetMappings().GetFunctionMappingCount();
	}
	const Type* Interpreter::GetLocalVariable(std::uint32_t index) const noexcept {
		return m_Stack.Get<Type>(m_LocalVariables[index]);
	}
	Type* Interpreter::GetLocalVariable(std::uint32_t index) noexcept {
		return m_Stack.Get<Type>(m_LocalVariables[index]);
	}
	std::uint32_t Interpreter::GetLocalVariableCount() const noexcept {
		return static_cast<std::uint32_t>(m_LocalVariables.size());
	}

	void Interpreter::PrintPointerTaget(std::ostream& stream, const Object& object) const {
		if (object.GetType() == PointerType) {
			const PointerObject& pointer = static_cast<const PointerObject&>(object);
			if (pointer.Value) {
				stream << '(';
				PrintObject(stream, static_cast<const Object*>(pointer.Value), true);
				stream << ')';
			}
		} else if (object.GetType() == GCPointerType) {
			const GCPointerObject& pointer = static_cast<const GCPointerObject&>(object);
			if (pointer.Value) {
				stream << '(';
				PrintObject(stream, reinterpret_cast<const Object*>(static_cast<const ManagedHeapInfo*>(pointer.Value) + 1), true);
				stream << ')';
			}
		}
	}

	void Interpreter::OccurException(std::uint32_t code) noexcept {
		InterpreterException& e = m_Exception.emplace();
		e.Function = m_StackFrame.Function;
		e.Instructions = m_StackFrame.Instructions;
		e.InstructionIndex = m_StackFrame.Caller;

		e.Code = code;
	}

	bool Interpreter::IsLocalVariable(std::size_t delta) const noexcept {
		return !m_LocalVariables.empty() && m_LocalVariables.back() == m_Stack.GetUsedSize() - delta;
	}
}