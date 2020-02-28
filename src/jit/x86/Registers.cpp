#include <svm/jit/x86/Registers.hpp>

#ifdef SVM_X86

namespace svm::jit::x86 {
	// Byte GPRs
	namespace {
		const RegisterData s_Cl("cl", RegisterType::General, RegisterSize::Byte, 0b0'001);
	}
	const Register Cl = &s_Cl;

	// DWord GPRs
	namespace {
		const RegisterData s_Eax("eax", RegisterType::General, RegisterSize::DWord, 0b0'000);
		const RegisterData s_Ecx("ecx", RegisterType::General, RegisterSize::DWord, 0b0'001);
		const RegisterData s_Edx("edx", RegisterType::General, RegisterSize::DWord, 0b0'010);
		const RegisterData s_Ebx("ebx", RegisterType::General, RegisterSize::DWord, 0b0'011);
		const RegisterData s_Esp("esp", RegisterType::General, RegisterSize::DWord, 0b0'100);
		const RegisterData s_Ebp("ebp", RegisterType::General, RegisterSize::DWord, 0b0'101);
		const RegisterData s_Esi("esi", RegisterType::General, RegisterSize::DWord, 0b0'110);
		const RegisterData s_Edi("edi", RegisterType::General, RegisterSize::DWord, 0b0'111);
#ifdef SVM_X64
		const RegisterData s_R8D("r8d", RegisterType::General, RegisterSize::DWord, 0b1'000);
		const RegisterData s_R9D("r9d", RegisterType::General, RegisterSize::DWord, 0b1'001);
		const RegisterData s_R10D("r10d", RegisterType::General, RegisterSize::DWord, 0b1'010);
		const RegisterData s_R11D("r11d", RegisterType::General, RegisterSize::DWord, 0b1'011);
		const RegisterData s_R12D("r12d", RegisterType::General, RegisterSize::DWord, 0b1'100);
		const RegisterData s_R13D("r13d", RegisterType::General, RegisterSize::DWord, 0b1'101);
		const RegisterData s_R14D("r14d", RegisterType::General, RegisterSize::DWord, 0b1'110);
		const RegisterData s_R15D("r15d", RegisterType::General, RegisterSize::DWord, 0b1'111);
#endif
	}
	const Register Eax = &s_Eax;
	const Register Ecx = &s_Ecx;
	const Register Edx = &s_Edx;
	const Register Ebx = &s_Ebx;
	const Register Esp = &s_Esp;
	const Register Ebp = &s_Ebp;
	const Register Esi = &s_Esi;
	const Register Edi = &s_Edi;
#ifdef SVM_X64
	const Register R8D = &s_R8D;
	const Register R9D = &s_R9D;
	const Register R10D = &s_R10D;
	const Register R11D = &s_R11D;
	const Register R12D = &s_R12D;
	const Register R13D = &s_R13D;
	const Register R14D = &s_R14D;
	const Register R15D = &s_R15D;
#endif

	// QWord GPRs
	namespace {
#ifdef SVM_X64
		const RegisterData s_Rax("rax", RegisterType::General, RegisterSize::QWord, 0b0'000);
		const RegisterData s_Rcx("rcx", RegisterType::General, RegisterSize::QWord, 0b0'001);
		const RegisterData s_Rdx("rdx", RegisterType::General, RegisterSize::QWord, 0b0'010);
		const RegisterData s_Rbx("rbx", RegisterType::General, RegisterSize::QWord, 0b0'011);
		const RegisterData s_Rsp("rsp", RegisterType::General, RegisterSize::QWord, 0b0'100);
		const RegisterData s_Rbp("rbp", RegisterType::General, RegisterSize::QWord, 0b0'101);
		const RegisterData s_Rsi("rsi", RegisterType::General, RegisterSize::QWord, 0b0'110);
		const RegisterData s_Rdi("rdi", RegisterType::General, RegisterSize::QWord, 0b0'111);
		const RegisterData s_R8("r8", RegisterType::General, RegisterSize::QWord, 0b1'000);
		const RegisterData s_R9("r9", RegisterType::General, RegisterSize::QWord, 0b1'001);
		const RegisterData s_R10("r10", RegisterType::General, RegisterSize::QWord, 0b1'010);
		const RegisterData s_R11("r11", RegisterType::General, RegisterSize::QWord, 0b1'011);
		const RegisterData s_R12("r12", RegisterType::General, RegisterSize::QWord, 0b1'100);
		const RegisterData s_R13("r13", RegisterType::General, RegisterSize::QWord, 0b1'101);
		const RegisterData s_R14("r14", RegisterType::General, RegisterSize::QWord, 0b1'110);
		const RegisterData s_R15("r15", RegisterType::General, RegisterSize::QWord, 0b1'111);
#endif
	}
#ifdef SVM_X64
	const Register Rax = &s_Rax;
	const Register Rcx = &s_Rcx;
	const Register Rdx = &s_Rdx;
	const Register Rbx = &s_Rbx;
	const Register Rsp = &s_Rsp;
	const Register Rbp = &s_Rbp;
	const Register Rsi = &s_Rsi;
	const Register Rdi = &s_Rdi;
	const Register R8 = &s_R8;
	const Register R9 = &s_R9;
	const Register R10 = &s_R10;
	const Register R11 = &s_R11;
	const Register R12 = &s_R12;
	const Register R13 = &s_R13;
	const Register R14 = &s_R14;
	const Register R15 = &s_R15;
#endif
}

#endif