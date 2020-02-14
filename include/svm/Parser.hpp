#pragma once

#include <svm/Instruction.hpp>

#include <cstdint>
#include <string>
#include <vector>

namespace svm {
	class Parser final {
	private:
		std::vector<std::uint8_t> m_File;
			
		Instructions m_Instructions;

	public:
		Parser() noexcept = default;
		Parser(const Parser& parser);
		Parser(Parser&& parser) noexcept;
		~Parser() = default;

	public:
		Parser& operator=(const Parser& parser);
		Parser& operator=(Parser&& parser) noexcept;
		bool operator==(const Parser&) = delete;
		bool operator!=(const Parser&) = delete;

	public:
		void Clear() noexcept;
		void Load(const std::string& path);
		bool IsLoaded() const noexcept;
		bool IsParsed() const noexcept;

		const Instructions& GetInstructions() const noexcept;
	};
}