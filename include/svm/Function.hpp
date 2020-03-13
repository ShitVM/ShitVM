#pragma once

#include <svm/Instruction.hpp>

#include <cstdint>
#include <ostream>
#include <vector>

namespace svm {
	class Function final {
	private:
		std::uint16_t m_Arity = 0;
		bool m_HasResult = false;
		Instructions m_Instructions;

	public:
		Function() noexcept = default;
		Function(std::uint16_t arity, bool hasResult, Instructions&& instructions) noexcept;
		Function(Function&& function) noexcept;
		~Function() = default;

	public:
		Function& operator=(Function&& function) noexcept;
		bool operator==(const Function&) = delete;
		bool operator!=(const Function&) = delete;

	public:
		std::uint16_t GetArity() const noexcept;
		void SetArity(std::uint16_t newArity) noexcept;
		bool HasResult() const noexcept;
		void SetHasResult(bool newHasResult) noexcept;
		const Instructions& GetInstructions() const noexcept;
		void SetInstructions(Instructions&& newInstructions) noexcept;
	};

	using Functions = std::vector<Function>;

	std::ostream& operator<<(std::ostream& stream, const Function& function);
	std::ostream& operator<<(std::ostream& stream, const Functions& functions);
}