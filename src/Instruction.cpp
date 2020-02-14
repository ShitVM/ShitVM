#include <svm/Instruction.hpp>

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
		if (instruction.HasOffset()) {
			stream << std::hex << std::uppercase << std::setw(16) << std::setfill('0') << instruction.Offset << ": ";
		}
		stream << Mnemonics[static_cast<int>(instruction.OpCode)];
		if (instruction.HasOperand()) {
			stream << " 0x" << instruction.Operand << std::dec << std::nouppercase;
		}
		return stream;
	}
}

namespace svm {
	Instructions& operator<<(Instructions& instructions, const Instruction& instruction) {
		instructions.push_back(instruction);
		return instructions;
	}
	Instructions& operator<<(Instructions& instructions, Instruction&& instruction) {
		instructions.push_back(std::move(instruction));
		return instructions;
	}

	std::ostream& operator<<(std::ostream& stream, const Instructions& instructions) {
		bool isFirst = true;
		for (const auto& inst : instructions) {
			if (isFirst) {
				isFirst = false;
			} else {
				stream << '\n';
			}
			stream << inst;
		}
		return stream;
	}
}