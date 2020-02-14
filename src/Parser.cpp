#include <svm/Parser.hpp>

#include <fstream>
#include <iomanip>
#include <ios>
#include <sstream>
#include <utility>

namespace svm {
	Parser::Parser(const Parser& parser)
		: m_File(parser.m_File), m_Pos(parser.m_Pos), m_Instructions(parser.m_Instructions) {}
	Parser::Parser(Parser&& parser) noexcept
		: m_File(std::move(parser.m_File)), m_Pos(parser.m_Pos), m_Instructions(std::move(parser.m_Instructions)) {}

	Parser& Parser::operator=(const Parser& parser) {
		m_File = parser.m_File;
		m_Pos = parser.m_Pos;
		m_Instructions = parser.m_Instructions;
		return *this;
	}
	Parser& Parser::operator=(Parser&& parser) noexcept {
		m_File = std::move(parser.m_File);
		m_Pos = parser.m_Pos;
		m_Instructions = std::move(parser.m_Instructions);
		return *this;
	}

	void Parser::Clear() noexcept {
		m_File.clear();
		m_Pos = 0;
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
		m_Pos = 0;
		m_Instructions.clear();
	}
	bool Parser::IsLoaded() const noexcept {
		return m_File.size() != 0;
	}
	void Parser::Parse() {
		ParseFunctions();
	}
	bool Parser::IsParsed() const noexcept {
		return m_Instructions.size() != 0;
	}

	const Instructions& Parser::GetInstructions() const noexcept {
		return m_Instructions;
	}

	namespace {
		template<typename Integer>
		std::string ToHexString(Integer value) {
			thread_local std::ostringstream oss;
			oss << std::hex << std::uppercase << value;

			const std::string result = oss.str();
			oss.clear();
			return result;
		}
	}

	void Parser::ParseFunctions() {
		if (!IsLoaded()) return;

		const std::size_t size = m_File.size();
		for (; m_Pos < size; ++m_Pos) {
			const std::uint8_t opCodeByte = m_File[m_Pos];
			if (opCodeByte > 32) throw std::runtime_error("Failed to parse the file. Unrecognized opcode 0x" + ToHexString(opCodeByte) + " at offset " + ToHexString(m_Pos));
			std::uint32_t operand = Instruction::NoOperand;

			if ((static_cast<int>(OpCode::Push) <= opCodeByte && opCodeByte <= static_cast<int>(OpCode::Store)) ||
				(static_cast<int>(OpCode::Jmp) <= opCodeByte && opCodeByte <= static_cast<int>(OpCode::Call))) {
				if (m_Pos + 5 > size) throw std::runtime_error("Failed to parse the file. Missing operand after opcode 0x" + ToHexString(opCodeByte) + " at offset " + ToHexString(m_Pos));
				operand = *reinterpret_cast<const std::uint32_t*>(m_File.data() + m_Pos + 1);
				m_Pos += 4;
			}

			m_Instructions.emplace_back(static_cast<OpCode>(opCodeByte), operand, m_Pos);
		}
	}
}