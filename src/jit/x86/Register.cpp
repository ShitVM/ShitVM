#include <svm/jit/x86/Register.hpp>

#ifdef SVM_X86

namespace svm::jit::x86 {
	RegisterData::RegisterData(const char* name, RegisterType type, RegisterSize size, std::uint8_t code) noexcept
		: Name(name), Type(type), Size(size), Code(code) {}
}

namespace svm::jit::x86 {
	Register::Register(std::nullptr_t) noexcept {}
	Register::Register(const RegisterData* data) noexcept
		: m_Data(data) {}
	Register::Register(const Register& reg) noexcept
		: m_Data(reg.m_Data) {}

	Register& Register::operator=(std::nullptr_t) noexcept {
		m_Data = nullptr;
		return *this;
	}
	Register& Register::operator=(const RegisterData* data) noexcept {
		m_Data = data;
		return *this;
	}
	Register& Register::operator=(const Register& reg) noexcept {
		m_Data = reg.m_Data;
		return *this;
	}
	bool Register::operator==(const Register& reg) const noexcept {
		return m_Data == reg.m_Data;
	}
	bool Register::operator!=(const Register& reg) const noexcept {
		return m_Data != reg.m_Data;
	}

	const RegisterData& Register::operator*() const noexcept {
		return *m_Data;
	}
	const RegisterData* Register::operator->() const noexcept {
		return m_Data;
	}
}

#endif