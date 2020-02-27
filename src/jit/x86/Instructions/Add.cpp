#include <svm/jit/x86/Builder.hpp>

#ifdef SVM_X86

#include <cassert>
#include <limits>

namespace svm::jit::x86 {
	void Builder::Add(Register a, Register b) {
		assert(a->Size == b->Size);

		Instruction& instruction = m_Instructions.emplace_back();
		instruction.OpCode = 0x01_b;

		REX rex;
		rex.Fields.W = a->Size == RegisterSize::QWord;

		ModRM modRM;
		GenerateModRM(b, a, rex, modRM);

		instruction.REXPrefix = rex;
		instruction.ModRM = modRM;
	}
	void Builder::Add(Register a, const Address& b) {
		assert(a->Size == b.GetSize());

		Instruction& instruction = m_Instructions.emplace_back();
		instruction.OpCode = 0x03_b;

		REX rex;
		rex.Fields.W = a->Size == RegisterSize::QWord;

		ModRM modRM;
		GenerateModRM(a, b, rex, modRM);

		SIB sib;
		const bool hasSIB = GenerateSIB(b, rex, sib);

		GenerateDisplacement(b, instruction.DispImm, instruction.DisplacementSize);

		instruction.REXPrefix = rex;
		instruction.ModRM = modRM;
		if (hasSIB) {
			instruction.SIB = sib;
		}
	}
	void Builder::Add(const Address& a, Register b) {
		assert(a.GetSize() == b->Size);

		Instruction& instruction = m_Instructions.emplace_back();
		instruction.OpCode = 0x01_b;

		REX rex;
		rex.Fields.W = a.GetSize() == RegisterSize::QWord;

		ModRM modRM;
		GenerateModRM(b, a, rex, modRM);

		SIB sib;
		const bool hasSIB = GenerateSIB(a, rex, sib);

		GenerateDisplacement(a, instruction.DispImm, instruction.DisplacementSize);

		instruction.REXPrefix = rex;
		instruction.ModRM = modRM;
		if (hasSIB) {
			instruction.SIB = sib;
		}
	}
	void Builder::Add(Register a, std::uint32_t b) {
		Instruction& instruction = m_Instructions.emplace_back();

		REX rex;
		rex.Fields.W = a->Size == RegisterSize::QWord;

		ModRM modRM;
		GenerateModRM(a, rex, modRM);

		instruction.REXPrefix = rex;
		instruction.ModRM = modRM;

		if (b <= std::numeric_limits<std::uint8_t>::max()) {
			instruction.OpCode = 0x83_b;

			instruction.DispImm.Fields.Displacement = static_cast<std::uint8_t>(b);
			instruction.DisplacementSize = sizeof(std::uint8_t);
		} else {
			instruction.OpCode = 0x81_b;

			instruction.DispImm.Fields.Displacement = b;
			instruction.DisplacementSize = sizeof(std::uint32_t);
		}
	}
	void Builder::Add(const Address& a, std::uint32_t b) {
		Instruction& instruction = m_Instructions.emplace_back();

		REX rex;
		rex.Fields.W = a.GetSize() == RegisterSize::QWord;

		ModRM modRM;
		GenerateModRM(a, rex, modRM);

		instruction.REXPrefix = rex;
		instruction.ModRM = modRM;

		if (b <= std::numeric_limits<std::uint8_t>::max()) {
			instruction.OpCode = 0x83_b;

			instruction.DispImm.Fields.Displacement = static_cast<std::uint8_t>(b);
			instruction.DisplacementSize = sizeof(std::uint8_t);
		} else {
			instruction.OpCode = 0x81_b;

			instruction.DispImm.Fields.Displacement = b;
			instruction.DisplacementSize = sizeof(std::uint32_t);
		}
	}
}

#endif