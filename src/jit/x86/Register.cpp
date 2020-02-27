#include <svm/jit/x86/Register.hpp>

#ifdef SVM_X86

namespace svm::jit::x86 {
	RegisterData::RegisterData(const char* name, RegisterType type, RegisterSize size, std::uint8_t code) noexcept
		: Name(name), Type(type), Size(size), Code(code) {}
}

#endif