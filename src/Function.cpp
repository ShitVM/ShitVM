#include <svm/Function.hpp>

#include <svm/IO.hpp>

#include <utility>

namespace svm {
	Function::Function(std::uint16_t arity, bool hasResult, Instructions&& instructions) noexcept
		: m_Arity(arity), m_HasResult(hasResult), m_Instructions(std::move(instructions)) {}
	Function::Function(Function&& function) noexcept
		: m_Arity(function.m_Arity), m_HasResult(function.m_HasResult), m_Instructions(std::move(function.m_Instructions)) {}

	Function& Function::operator=(Function&& function) noexcept {
		m_Arity = function.m_Arity;
		m_HasResult = function.m_HasResult;
		m_Instructions = std::move(function.m_Instructions);
		return *this;
	}

	std::uint16_t Function::GetArity() const noexcept {
		return m_Arity;
	}
	bool Function::HasResult() const noexcept {
		return m_HasResult;
	}
	const Instructions& Function::GetInstructions() const noexcept {
		return m_Instructions;
	}

	std::ostream& operator<<(std::ostream& stream, const Function& function) {
		const std::string defIndent = detail::MakeTabs(stream);

		stream << defIndent << "Function:\n"
			   << defIndent << "\tArity: " << function.GetArity() << '\n'
			   << defIndent << "\tHasResult: " << std::boolalpha << function.HasResult() << '\n'
			   << Indent << Indent << function.GetInstructions() << UnIndent << UnIndent;
		return stream;
	}
	std::ostream& operator<<(std::ostream& stream, const Functions& functions) {
		const std::string defIndent = detail::MakeTabs(stream);

		stream << defIndent << "Functions: " << functions.size() << Indent << Indent;
		for (std::uint32_t i = 0; i < static_cast<std::uint32_t>(functions.size()); ++i) {
			stream << '\n' << defIndent << "\t[" << i << "]:\n" << functions[i];
		}
		stream << UnIndent << UnIndent;
		return stream;
	}
}