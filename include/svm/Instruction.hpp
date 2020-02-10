#pragma once

namespace svm {
	enum class OpCode {
		Nop,

		Push,
		Pop,
		Load,
		Store,
		
		Add,
		Sub,
		Mul,
		IMul,
		Div,
		IDiv,
		Mod,
		IMod,
		Neg,

		And,
		Or,
		Xor,
		Not,
		Shl,
		Sal,
		Shr,
		Sar,

		Cmp,
		ICmp,
		Jmp,
		Je,
		Jne,
		Ja,
		Jnae,
		Jb,
		Jnbe,
		Call,
		Ret,
	};
}