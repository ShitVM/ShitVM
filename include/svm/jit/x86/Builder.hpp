#pragma once
#include <svm/Macro.hpp>

#ifdef SVM_X86

#include <svm/jit/x86/Address.hpp>
#include <svm/jit/x86/Instruction.hpp>
#include <svm/jit/x86/Register.hpp>

#include <type_traits>
#include <vector>

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

		void Add(Register a, Register b);
		void Add(Register a, const Address& b);
		void Add(const Address& a, Register b);
		void Add(Register a, std::uint32_t b);
		void Add(const Address& a, std::uint32_t b);

	private:
		void GenerateModRM(Register reg, REX& rex, ModRM& modRM) noexcept;
		void GenerateModRM(const Address& addr, REX& rex, ModRM& modRM) noexcept;
		void GenerateModRM(Register reg1, Register reg2, REX& rex, ModRM& modRM) noexcept;
		void GenerateModRM(Register reg, const Address& addr, REX& rex, ModRM& modRM) noexcept;
		bool GenerateSIB(const Address& addr, REX& rex, SIB& sib) noexcept;
		void GenerateDisplacement(const Address& addr, DispImm& disp, std::uint8_t& dispSize) noexcept;
	};
}

#endif