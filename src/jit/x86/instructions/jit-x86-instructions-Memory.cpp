#include <svm/jit/x86/Builder.hpp>

#ifdef SVM_X86

#include <cassert>
#include <limits>

namespace svm::jit::x86 {
	void Builder::Mov(Register a, const RM& b) {
		AddSubInternal(0x8A, a, b);
	}
	void Builder::Mov(const Memory& a, Register b) {
		AddSubInternal(0x88, a, b);
	}
#ifdef SVM_X64
	void Builder::Mov(Register a, std::uint64_t b) {
		Mov(a, static_cast<std::uint32_t>(b));

		if (b > std::numeric_limits<std::uint32_t>::max()) {
			m_Instructions.back().REXPrefix.value().Fields.W = 1;
			m_Instructions.back().DispImm.Immediate64 |= b & 0xFFFFFFFF00000000;
			m_Instructions.back().ImmediateSize = 8;
		}
	}
#endif
	void Builder::Mov(Register a, std::uint32_t b) {
		Instruction& instruction = m_Instructions.emplace_back();
		instruction.OpCode = static_cast<std::uint8_t>((a->Code & 0b111) + 0xB8);

		REX rex;
		rex.Fields.B = a->Code >> 3;

		switch (a->Size) {
		case RegisterSize::Byte: instruction.ImmediateSize = 1; break;
		case RegisterSize::DWord:
		case RegisterSize::QWord: instruction.ImmediateSize = 4; break;
		default: assert(false); break;
		}

		instruction.REXPrefix = rex;
		instruction.DispImm.Fields.Immediate = b;
	}
	void Builder::Mov(const Memory& a, std::uint32_t b) {
		Instruction& instruction = m_Instructions.emplace_back();
		instruction.OpCode = 0xC6_b;
		if (a.GetSize() != MemorySize::Byte) {
			instruction.OpCode |= 0b1; //s=1
		}

		REX rex;
		rex.Fields.W = a.GetSize() == MemorySize::QWord;

		ModRM modRM;
		GenerateModRM(a, rex, modRM);

		SIB sib;
		if (GenerateSIB(a, rex, sib)) {
			instruction.SIB = sib;
		}

		GenerateDisplacement(a, instruction.DispImm, instruction.DisplacementSize);

		instruction.REXPrefix = rex;
		instruction.ModRM = modRM;
		instruction.DispImm.Fields.Immediate = b;
		if (a.GetSize() == MemorySize::Byte) {
			assert(b <= std::numeric_limits<std::uint8_t>::max());
			instruction.ImmediateSize = 1;
		} else {
			instruction.ImmediateSize = 4;
		}
	}
}

namespace svm::jit::x86 {
	void Builder::PushPopInternal(std::uint8_t opCode, Register a) {
#ifdef SVM_X64
		assert(a->Size == RegisterSize::QWord);
#else
		assert(a->Size == RegisterSize::DWord);
#endif

		Instruction& instruction = m_Instructions.emplace_back();
		instruction.OpCode = static_cast<std::uint8_t>((a->Code & 0b111) + opCode);

		REX rex;
		rex.Fields.B = a->Code >> 3;

		instruction.REXPrefix = rex;
	}
	void Builder::PushPopInternal(std::uint8_t opCode, std::uint8_t opCodeExt, const Memory& a) {
#ifdef SVM_X64
		assert(a.GetSize() == RegisterSize::QWord);
#else
		assert(a.GetSize() == RegisterSize::DWord);
#endif

		Instruction& instruction = m_Instructions.emplace_back();
		instruction.OpCode = opCode;

		REX rex;

		ModRM modRM;
		GenerateModRM(a, rex, modRM);
		modRM.Fields.Reg = opCodeExt;

		SIB sib;
		if (GenerateSIB(a, rex, sib)) {
			instruction.SIB = sib;
		}

		GenerateDisplacement(a, instruction.DispImm, instruction.DisplacementSize);

		instruction.REXPrefix = rex;
		instruction.ModRM = modRM;
	}
}

namespace svm::jit::x86 {
	void Builder::Push(Register a) {
		PushPopInternal(0x50, a);
	}
	void Builder::Push(const Memory& a) {
		PushPopInternal(0xFF, 0b110, a);
	}
	void Builder::Push(std::uint32_t a) {
		Instruction& instruction = m_Instructions.emplace_back();

		if (a <= std::numeric_limits<std::uint8_t>::max()) {
			instruction.OpCode = 0x6A;
			instruction.DispImm.Fields.Immediate = a;
			instruction.ImmediateSize = 1;
		} else {
			instruction.OpCode = 0x68;
			instruction.DispImm.Fields.Immediate = a;
			instruction.ImmediateSize = 4;
		}
	}
	void Builder::Pop(Register a) {
		PushPopInternal(0x58, a);
	}
	void Builder::Pop(const Memory& a) {
		PushPopInternal(0x8F, 0b000, a);
	}
}

#endif