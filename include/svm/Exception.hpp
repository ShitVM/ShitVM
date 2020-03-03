#pragma once

#include <svm/Function.hpp>
#include <svm/Instruction.hpp>

#include <cstdint>
#include <string>

namespace svm {
	struct InterpreterException final {
		const svm::Function* Function = nullptr;
		const svm::Instructions* Instructions = nullptr;
		std::uint64_t InstructionIndex = 0;

		std::uint32_t Code = 0;
	};
}