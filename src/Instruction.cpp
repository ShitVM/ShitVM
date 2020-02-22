#include <svm/Instruction.hpp>

#include <svm/Memory.hpp>

#include <iomanip>
#include <ios>
#include <sstream>
#include <utility>

namespace svm {
	Instruction::Instruction(svm::OpCode opCode) noexcept
		: OpCode(opCode) {}
	Instruction::Instruction(svm::OpCode opCode, std::uint64_t offset) noexcept
		: OpCode(opCode), Offset(offset) {}
	Instruction::Instruction(svm::OpCode opCode, std::uint32_t operand) noexcept
		: OpCode(opCode), Operand(operand) {}
	Instruction::Instruction(svm::OpCode opCode, std::uint32_t operand, std::uint64_t offset) noexcept
		: OpCode(opCode), Operand(operand), Offset(offset) {}
	Instruction::Instruction(const Instruction& instruction) noexcept
		: OpCode(instruction.OpCode), Operand(instruction.Operand), Offset(instruction.Offset) {}

	Instruction& Instruction::operator=(const Instruction& instruction) noexcept {
		OpCode = instruction.OpCode;
		Operand = instruction.Operand;

		Offset = instruction.Offset;
		return *this;
	}
	bool Instruction::operator==(const Instruction& instruction) const noexcept {
		return OpCode == instruction.OpCode && Operand == instruction.Operand;
	}
	bool Instruction::operator!=(const Instruction& instruction) const noexcept {
		return OpCode != instruction.OpCode || Operand != instruction.Operand;
	}

	bool Instruction::HasOperand() const noexcept {
		return Operand != NoOperand;
	}
	bool Instruction::HasOffset() const noexcept {
		return Offset != NoOffset;
	}
	std::string Instruction::ToString() const {
		std::ostringstream oss;
		oss << *this;
		return oss.str();
	}

	namespace {
		int ByteOrTextIndex() {
			static const int index = std::ios_base::xalloc();
			return index;
		}
	}

	std::ostream& Byte(std::ostream& stream) {
		stream.iword(ByteOrTextIndex()) = 1;
		return stream;
	}
	std::ostream& Text(std::ostream& stream) {
		stream.iword(ByteOrTextIndex()) = 0;
		return stream;
	}
	std::ostream& operator<<(std::ostream& stream, const Instruction& instruction) {
		if (stream.iword(ByteOrTextIndex()) == 0) {
			if (instruction.HasOffset()) {
				stream << std::hex << std::uppercase << std::setw(16) << std::setfill('0') << instruction.Offset << ": ";
			}
			stream << Mnemonics[static_cast<int>(instruction.OpCode)];
			if (instruction.HasOperand()) {
				stream << " 0x" << instruction.Operand << std::dec << std::nouppercase;
			}
		} else {
			std::uint8_t bytes[5];
			bytes[0] = static_cast<std::uint8_t>(instruction.OpCode);
			if (instruction.HasOperand()) {
				if (GetEndian() == Endian::Little) {
					*reinterpret_cast<std::uint32_t*>(bytes + 1) = instruction.Operand;
				} else {
					*reinterpret_cast<std::uint32_t*>(bytes + 1) = ReverseEndian(instruction.Operand);
				}
			}
			stream.write(reinterpret_cast<const char*>(bytes), instruction.HasOperand() ? 5 : 1);
		}

		return stream;
	}
}

namespace svm {
	Instructions::Instructions(std::unique_ptr<std::uint64_t[]>&& labels, std::uint32_t labelCount,
							   std::unique_ptr<Instruction[]> instructions, std::uint64_t instructionCount) noexcept
		: m_Labels(std::move(labels)), m_LabelCount(labelCount), m_Instructions(std::move(instructions)), m_InstructionCount(instructionCount) {}
	Instructions::Instructions(Instructions&& instructions) noexcept
		: m_Labels(std::move(instructions.m_Labels)), m_LabelCount(instructions.m_LabelCount),
		m_Instructions(std::move(instructions.m_Instructions)), m_InstructionCount(instructions.m_InstructionCount){}

	Instructions& Instructions::operator=(Instructions&& instructions) noexcept {
		m_Labels = std::move(instructions.m_Labels);
		m_LabelCount = instructions.m_LabelCount;
		m_Instructions = std::move(instructions.m_Instructions);
		m_InstructionCount = instructions.m_InstructionCount;
		return *this;
	}
	const Instruction& Instructions::operator[](std::uint64_t offset) const noexcept {
		return m_Instructions[static_cast<std::size_t>(offset)];
	}

	void Instructions::Clear() noexcept {
		m_Labels.reset();
		m_LabelCount = 0;
		m_Instructions.reset();
		m_InstructionCount = 0;
	}
	bool Instructions::IsEmpty() const noexcept {
		return m_Labels == nullptr && m_Instructions == nullptr;
	}

	std::uint64_t Instructions::GetLabel(std::uint32_t index) const noexcept {
		return m_Labels[index];
	}
	const Instruction& Instructions::GetInstruction(std::uint64_t offset) const noexcept {
		return m_Instructions[static_cast<std::size_t>(offset)];
	}
	const std::uint64_t* Instructions::GetLabels() const noexcept {
		return m_Labels.get();
	}
	const Instruction* Instructions::GetInstructions() const noexcept {
		return m_Instructions.get();
	}
	std::uint32_t Instructions::GetLabelCount() const noexcept {
		return m_LabelCount;
	}
	std::uint64_t Instructions::GetInstructionCount() const noexcept {
		return m_InstructionCount;
	}

	std::ostream& operator<<(std::ostream& stream, const Instructions& instructions) {
		for (std::uint64_t i = 0; i < instructions.GetInstructionCount(); ++i) {
			if (i != 0) {
				stream << '\n';
			}
			stream << instructions[i];
		}
		return stream;
	}
}