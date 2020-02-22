#include <svm/Instruction.hpp>

#include <svm/IO.hpp>
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

	std::ostream& operator<<(std::ostream& stream, const Instruction& instruction) {
		if (stream.iword(detail::ByteModeIndex()) == 0) {
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

	namespace {
		void PrintInstructions(std::ostream& stream, const Instructions& instructions) {
			for (std::uint64_t i = 0; i < instructions.GetInstructionCount(); ++i) {
				if (i != 0 && stream.iword(detail::ByteModeIndex()) == 0) {
					stream << '\n';
				}
				stream << instructions[i];
			}
		}
		void PrintLabels(std::ostream& stream, const Instructions& instructions) {
			if (stream.iword(detail::ByteModeIndex()) == 1) {
				std::uint8_t bytes[4];
				if (GetEndian() == Endian::Little) {
					*reinterpret_cast<std::uint32_t*>(bytes) = instructions.GetLabelCount();
				} else {
					*reinterpret_cast<std::uint32_t*>(bytes) = ReverseEndian(instructions.GetLabelCount());
				}
				stream.write(reinterpret_cast<const char*>(bytes), 4);
			}

			for (std::uint32_t i = 0; i < instructions.GetLabelCount(); ++i) {
				if (stream.iword(detail::ByteModeIndex()) == 0) {
					if (i != 0) {
						stream << '\n';
					}
					stream << '[' << i << "]: " << std::hex << std::uppercase << std::setw(16) << std::setfill('0') << instructions.GetLabel(i)
						   << std::dec << std::nouppercase;
				} else {
					std::uint8_t bytes[8];
					if (GetEndian() == Endian::Little) {
						*reinterpret_cast<std::uint64_t*>(bytes) = instructions.GetLabel(i);
					} else {
						*reinterpret_cast<std::uint64_t*>(bytes) = ReverseEndian(instructions.GetLabel(i));
					}
					stream.write(reinterpret_cast<const char*>(bytes), 8);
				}
			}
		}
	}

	std::ostream& operator<<(std::ostream& stream, const Instructions& instructions) {
		switch (stream.iword(detail::InstLabelModeIndex())) {
		case 0:
			PrintInstructions(stream, instructions);
			break;

		case 1:
			PrintLabels(stream, instructions);
			break;

		case 2:
			stream << "<Labels>\n";
			PrintLabels(stream, instructions);
			stream << "\n\n<Instructions>\n";
			PrintInstructions(stream, instructions);
			break;
		}
		return stream;
	}
}