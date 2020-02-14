#pragma once

#include <svm/ByteFile.hpp>
#include <svm/Instruction.hpp>

#include <cstdint>
#include <string>
#include <vector>

namespace svm {
	class Parser final {
	private:
		std::vector<std::uint8_t> m_File;
		std::size_t m_Pos = 0;

		std::string m_Path;
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
		void Parse();
		bool IsParsed() const noexcept;

		ByteFile GetResult();
		std::string_view GetPath() const noexcept;
		const Instructions& GetInstructions() const noexcept;

	private:
		void ParseFunctions();
	};
}