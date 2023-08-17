#pragma once

#include <svm/Function.hpp>
#include <svm/Instruction.hpp>
#include <svm/virtual/VirtualFunction.hpp>

#include <cstdint>
#include <string_view>
#include <variant>

namespace svm {
	struct InterpreterException final {
		std::variant<std::monostate, svm::Function, VirtualFunction> Function;
		const svm::Instructions* Instructions = nullptr;
		std::uint64_t InstructionIndex = 0;

		std::uint32_t Code = 0;
	};

	std::string_view GetInterpreterExceptionMessage(std::uint32_t code) noexcept;
}