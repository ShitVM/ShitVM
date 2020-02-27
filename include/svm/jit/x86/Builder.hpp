#pragma once
#include <svm/Macro.hpp>

#ifdef SVM_X86

#include <svm/jit/x86/Address.hpp>
#include <svm/jit/x86/Instruction.hpp>
#include <svm/jit/x86/Register.hpp>

#include <functional>
#include <type_traits>
#include <variant>
#include <vector>

namespace svm::jit::x86 {
	class RM final {
	private:
		std::variant<Register, std::reference_wrapper<const Address>> m_Data;

	public:
		RM(Register reg) noexcept;
		RM(const Address& addr) noexcept;
		RM(const RM&) = delete;
		~RM() = default;

	public:
		RM& operator=(const RM&) = delete;
		bool operator==(const RM&) = delete;
		bool operator!=(const RM&) = delete;

	public:
		bool IsRegister() const noexcept;
		bool IsAddress() const noexcept;
		Register GetRegister() const noexcept;
		const Address& GetAddress() const noexcept;
		
		MemorySize GetSize() const noexcept;
	};
}

namespace svm::jit::x86 {
	class Builder final {
	private:
		std::vector<Instruction> m_Instructions;

	public:
		Builder() noexcept = default;
		Builder(Builder&& builder) noexcept;
		~Builder() = default;

	public:
		Builder& operator=(Builder&& builder) noexcept;
		bool operator==(const Builder&) = delete;
		bool operator!=(const Builder&) = delete;

	public:
		std::size_t GetResult(void* buffer) noexcept;

		void Add(Register a, const RM& b);
		void Add(const Address& a, Register b);
		void Add(const RM& a, std::uint32_t b);

		void Sub(Register a, const RM& b);
		void Sub(const Address& a, Register b);
		void Sub(const RM& a, std::uint32_t b);

	private:
		void GenerateModRM(const RM& rm, REX& rex, ModRM& modRM) noexcept;
		void GenerateModRM(Register reg, REX& rex, ModRM& modRM) noexcept;
		void GenerateModRM(const Address& addr, REX& rex, ModRM& modRM) noexcept;
		void GenerateModRM(Register reg, const RM& rm, REX& rex, ModRM& modRM) noexcept;
		void GenerateModRM(Register reg1, Register reg2, REX& rex, ModRM& modRM) noexcept;
		void GenerateModRM(Register reg, const Address& addr, REX& rex, ModRM& modRM) noexcept;
		bool GenerateSIB(const Address& addr, REX& rex, SIB& sib) noexcept;
		void GenerateDisplacement(const Address& addr, DispImm& disp, std::uint8_t& dispSize) noexcept;

		void Arithmetic(std::uint8_t opCode, Register a, const RM& b);
		void Arithmetic(std::uint8_t opCode, const Address& a, Register b);
		void Arithmetic(std::uint8_t opCode, const RM& a, std::uint32_t b);
	};
}

#endif