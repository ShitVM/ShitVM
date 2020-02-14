#include <svm/Parser.hpp>

#include <fstream>
#include <utility>

namespace svm {
	Parser::Parser(const Parser& parser)
		: m_File(parser.m_File), m_Instructions(parser.m_Instructions) {}
	Parser::Parser(Parser&& parser) noexcept
		: m_File(std::move(parser.m_File)), m_Instructions(std::move(parser.m_Instructions)) {}

	Parser& Parser::operator=(const Parser& parser) {
		m_File = parser.m_File;
		m_Instructions = parser.m_Instructions;
		return *this;
	}
	Parser& Parser::operator=(Parser&& parser) noexcept {
		m_File = std::move(parser.m_File);
		m_Instructions = std::move(parser.m_Instructions);
		return *this;
	}

	void Parser::Clear() noexcept {
		m_File.clear();
		m_Instructions.clear();
	}
	void Parser::Load(const std::string& path) {
		std::ifstream stream(path, std::ifstream::binary);
		if (!stream) throw std::runtime_error("Failed to open the file.");

		stream.seekg(0, std::ifstream::end);
		const std::streamsize length = stream.tellg();
		stream.seekg(0, std::ifstream::beg);

		if (length == 0) {
			Clear();
			return;
		}

		std::vector<std::uint8_t> bytes(static_cast<std::size_t>(length));
		stream.read(reinterpret_cast<char*>(bytes.data()), length);
		if (stream.gcount() != length) throw std::runtime_error("Failed to read the file.");

		m_File = std::move(bytes);
		m_Instructions.clear();
	}
	bool Parser::IsLoaded() const noexcept {
		return m_File.size() != 0;
	}
	bool Parser::IsParsed() const noexcept {
		return m_Instructions.size() != 0;
	}

	const Instructions& Parser::GetInstructions() const noexcept {
		return m_Instructions;
	}
}