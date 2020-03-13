#include <svm/Instruction.hpp>

#include <svm/IO.hpp>

#include <utility>

namespace svm {
	Instruction::Instruction(svm::OpCode opCode, std::uint64_t offset) noexcept
		: OpCode(opCode), Offset(offset) {}
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
		return svm::HasOperand[static_cast<std::uint8_t>(OpCode)];
	}

	std::ostream& operator<<(std::ostream& stream, const Instruction& instruction) {
		stream << QWord(instruction.Offset) << ": " << Mnemonics[static_cast<std::uint8_t>(instruction.OpCode)];
		if (instruction.HasOperand()) {
			stream << " 0x" << Hex(instruction.Operand);
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
	std::uint32_t Instructions::GetLabelCount() const noexcept {
		return static_cast<std::uint32_t>(m_Labels.size());
	}
	std::uint64_t Instructions::GetInstructionCount() const noexcept {
		return m_Instructions.size();
	}

	const std::vector<std::uint64_t>& Instructions::GetLabels() const noexcept {
		return m_Labels;
	}
	const std::vector<Instruction>& Instructions::GetInstructions() const noexcept {
		return m_Instructions;
	}

	std::ostream& operator<<(std::ostream& stream, const Instructions& instructions) {
		const std::string defIndent = detail::MakeTabs(stream);
		const std::uint32_t labelCount = instructions.GetLabelCount();
		const std::uint64_t instCount = instructions.GetInstructionCount();

		stream << defIndent << "Instructions: " << instCount << '\n'
			   << defIndent << "\tLabels: " << labelCount;
		for (std::uint32_t i = 0; i < labelCount; ++i) {
			const std::uint64_t label = instructions.GetLabel(i);
			
			stream << '\n' << defIndent << "\t\t[" << i << "]: " << label << '(' << QWord(instructions.GetInstruction(label).Offset) << ')';
		}
		for (std::uint64_t i = 0; i < instCount; ++i) {
			stream << '\n' << defIndent << '\t' << instructions.GetInstruction(i);
		}

		return stream;
	}
}