#include <svm/Instruction.hpp>

#include <svm/IO.hpp>
#include <svm/Memory.hpp>

#include <iomanip>
#include <ios>
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

	std::ostream& operator<<(std::ostream& stream, const Instruction& instruction) {
		if (instruction.HasOffset()) {
			stream << std::hex << std::uppercase << std::setw(16) << std::setfill('0') << instruction.Offset << ": "
				   << std::dec << std::nouppercase;
		}
		stream << Mnemonics[static_cast<int>(instruction.OpCode)];
		if (instruction.HasOperand()) {
			stream << std::hex << std::uppercase << " 0x" << instruction.Operand << std::dec << std::nouppercase;
		}
		return stream;
	}
}

namespace svm {
	Instructions::Instructions(std::vector<std::uint64_t> labels, std::vector<Instruction> instructions) noexcept
		: m_Labels(std::move(labels)), m_Instructions(std::move(instructions)) {}
	Instructions::Instructions(Instructions&& instructions) noexcept
		: m_Labels(std::move(instructions.m_Labels)), m_Instructions(std::move(instructions.m_Instructions)) {}

	Instructions& Instructions::operator=(Instructions&& instructions) noexcept {
		m_Labels = std::move(instructions.m_Labels);
		m_Instructions = std::move(instructions.m_Instructions);
		return *this;
	}
	const Instruction& Instructions::operator[](std::uint64_t offset) const noexcept {
		return m_Instructions[static_cast<std::size_t>(offset)];
	}

	void Instructions::Clear() noexcept {
		m_Labels.clear();
		m_Instructions.clear();
	}
	bool Instructions::IsEmpty() const noexcept {
		return m_Labels.empty() && m_Instructions.empty();
	}

	std::uint64_t Instructions::GetLabel(std::uint32_t index) const noexcept {
		return m_Labels[index];
	}
	const Instruction& Instructions::GetInstruction(std::uint64_t offset) const noexcept {
		return m_Instructions[static_cast<std::size_t>(offset)];
	}
	const std::vector<std::uint64_t>& Instructions::GetLabels() const noexcept {
		return m_Labels;
	}
	const std::vector<Instruction>& Instructions::GetInstructions() const noexcept {
		return m_Instructions;
	}
	std::uint32_t Instructions::GetLabelCount() const noexcept {
		return static_cast<std::uint32_t>(m_Labels.size());
	}
	std::uint64_t Instructions::GetInstructionCount() const noexcept {
		return m_Instructions.size();
	}

	std::ostream& operator<<(std::ostream& stream, const Instructions& instructions) {
		const std::string defIndent = detail::MakeTabs(stream);
		stream << defIndent << "Instructions: " << instructions.GetInstructionCount() << '\n'
			   << defIndent << "\tLabels: " << instructions.GetLabelCount();
		for (std::uint32_t i = 0; i < instructions.GetLabelCount(); ++i) {
			stream << '\n' << defIndent << "\t\t[" << i << "]: " << instructions.GetLabel(i)
				   << '(' << std::hex << std::uppercase << std::setw(16) << std::setfill('0') << instructions.GetInstruction(instructions.GetLabel(i)).Offset << ')'
				   << std::dec << std::nouppercase;
		}
		for (std::uint64_t i = 0; i < instructions.GetInstructionCount(); ++i) {
			stream << '\n' << defIndent << '\t' << instructions.GetInstruction(i);
		}

		return stream;
	}
}