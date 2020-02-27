#pragma once
#include <svm/Macro.hpp>

#ifdef SVM_X86

#include <svm/jit/x86/Register.hpp>

namespace svm::jit::x86 {
	// DWord GPRs
	extern const Register Eax;
	extern const Register Ecx;
	extern const Register Edx;
	extern const Register Ebx;
	extern const Register Esp;
	extern const Register Ebp;
	extern const Register Esi;
	extern const Register Edi;
	extern const Register R8D;
	extern const Register R9D;
	extern const Register R10D;
	extern const Register R11D;
	extern const Register R12D;
	extern const Register R13D;
	extern const Register R14D;
	extern const Register R15D;

	// QWord GPRs
	extern const Register Rax;
	extern const Register Rcx;
	extern const Register Rdx;
	extern const Register Rbx;
	extern const Register Rsp;
	extern const Register Rbp;
	extern const Register Rsi;
	extern const Register Rdi;
	extern const Register R8;
	extern const Register R9;
	extern const Register R10;
	extern const Register R11;
	extern const Register R12;
	extern const Register R13;
	extern const Register R14;
	extern const Register R15;
}

#endif