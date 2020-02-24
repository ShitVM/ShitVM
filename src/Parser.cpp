#include <svm/Parser.hpp>

#include <fstream>
#include <iomanip>
#include <ios>
#include <memory>
#include <sstream>

namespace svm {
	Parser::Parser(Parser&& parser) noexcept
		: m_File(std::move(parser.m_File)), m_Pos(parser.m_Pos),
		m_Path(std::move(parser.m_Path)), m_ConstantPool(std::move(parser.m_ConstantPool)),
		m_Functions(std::move(parser.m_Functions)), m_EntryPoint(std::move(parser.m_EntryPoint)) {}

	Parser& Parser::operator=(Parser&& parser) noexcept {
		m_File = std::move(parser.m_File);
		m_Pos = parser.m_Pos;

		m_Path = std::move(parser.m_Path);
		m_ConstantPool = std::move(parser.m_ConstantPool);
		m_Functions = std::move(parser.m_Functions);
		m_EntryPoint = std::move(parser.m_EntryPoint);
		return *this;
	}

	void Parser::Clear() noexcept {
		m_File.clear();
		m_Pos = 0;

		m_Path.clear();
		m_ConstantPool.Clear();
		m_Functions.clear();
		m_EntryPoint.Clear();
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

		m_Path = path;
		m_ConstantPool.Clear();
		m_Functions.clear();
		m_EntryPoint.Clear();
	}
	bool Parser::IsLoaded() const noexcept {
		return m_File.size() != 0;
	}
	void Parser::Parse() {
		if (!IsLoaded()) throw std::runtime_error("Failed to parse the file. Incomplete loading.");
		else if (m_File.size() < 32) throw std::runtime_error("Failed to parse the file. Invalid format.");

		static constexpr std::uint8_t magic[] = { 0x74, 0x68, 0x74, 0x68 };
		const auto [magicBegin, magicEnd] = ReadFile(4);
		if (!std::equal(magicBegin, magicEnd, magic)) throw std::runtime_error("Failed to parse the file. Invalid format.");

		const auto version = ReadFile<std::uint32_t>();
		switch (version) {
		case 0x0000:
			ParseVer0000();
			break;

		default: throw std::runtime_error("Failed to parse the file. Incompatible version.");
		}
	}
	bool Parser::IsParsed() const noexcept {
		return !m_Functions.empty() || !m_EntryPoint.IsEmpty();
	}

	ByteFile Parser::GetResult() {
		if (!IsParsed()) throw std::runtime_error("Failed to move the result. Incomplete parsing.");

		ByteFile result(std::move(m_Path), std::move(m_ConstantPool), std::move(m_Functions), std::move(m_EntryPoint));
		Clear();
		return result;
	}
	std::string_view Parser::GetPath() const noexcept {
		return m_Path;
	}
	const ConstantPool& Parser::GetConstantPool() const noexcept {
		return m_ConstantPool;
	}
	const Functions& Parser::GetFunctions() const noexcept {
		return m_Functions;
	}
	const Instructions& Parser::GetEntryPoint() const noexcept {
		return m_EntryPoint;
	}

	void Parser::ParseVer0000() {
		ParseConstantPool();
		ParseFunctions();
		m_EntryPoint = ParseInstructions();
	}

	void Parser::ParseConstantPool() {
		const auto intCount = ReadFile<std::uint32_t>();
		std::vector<IntObject> intPool(intCount);
		ParseConstants(intPool);

		const auto longCount = ReadFile<std::uint32_t>();
		std::vector<LongObject> longPool(longCount);
		ParseConstants(longPool);

		const auto doubleCount = ReadFile<std::uint32_t>();
		std::vector<DoubleObject> doublePool(doubleCount);
		ParseConstants(doublePool);

		m_ConstantPool = ConstantPool(std::move(intPool), std::move(longPool), std::move(doublePool));
	}
	void Parser::ParseFunctions() {
		const auto funcCount = ReadFile<std::uint32_t>();
		m_Functions.reserve(funcCount);

		for (std::uint32_t i = 0; i < funcCount; ++i) {
			const auto arity = ReadFile<std::uint16_t>();
			const auto hasResult = ReadFile<bool>();
			auto instructions = ParseInstructions();

			m_Functions.emplace_back(arity, hasResult, std::move(instructions));
		}
	}
	Instructions Parser::ParseInstructions() {
		const auto labelCount = ReadFile<std::uint32_t>();
		std::vector<std::uint64_t> labels(labelCount);
		for (std::uint32_t i = 0; i < labelCount; ++i) {
			labels[i] = ReadFile<std::uint64_t>();
		}

		const auto instCount = ReadFile<std::uint64_t>();
		std::vector<Instruction> insts;
		insts.reserve(static_cast<std::size_t>(instCount));

		std::uint64_t nextOffset = 0;
		for (std::uint64_t i = 0; i < instCount; ++i) {
			const OpCode opCode = static_cast<OpCode>(ReadFile<std::uint8_t>());
			std::uint32_t operand = Instruction::NoOperand;
			const std::uint64_t offset = nextOffset;

			if (OpCode::Push <= opCode && opCode <= OpCode::Store && opCode != OpCode::Pop ||
				OpCode::Jmp <= opCode && opCode <= OpCode::Call) {
				operand = ReadFile<std::uint32_t>();
				nextOffset += 4;
			}

			insts.emplace_back(opCode, operand, offset);
			++nextOffset;
		}

		return { std::move(labels), std::move(insts) };
	}
}