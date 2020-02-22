#include <svm/Parser.hpp>

#include <fstream>
#include <iomanip>
#include <ios>
#include <memory>
#include <sstream>

namespace svm {
	Parser::Parser(Parser&& parser) noexcept
		: m_File(std::move(parser.m_File)), m_Pos(parser.m_Pos),
		m_Path(std::move(parser.m_Path)), m_ConstantPool(std::move(parser.m_ConstantPool)), m_Instructions(std::move(parser.m_Instructions)) {}

	Parser& Parser::operator=(Parser&& parser) noexcept {
		m_File = std::move(parser.m_File);
		m_Pos = parser.m_Pos;

		m_Path = std::move(parser.m_Path);
		m_ConstantPool = std::move(parser.m_ConstantPool);
		m_Instructions = std::move(parser.m_Instructions);
		return *this;
	}

	void Parser::Clear() noexcept {
		m_File.clear();
		m_Pos = 0;

		m_Path.clear();
		m_ConstantPool.Clear();
		m_Instructions.Clear();
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
		m_Instructions.Clear();
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

		default: throw std::runtime_error("Failed to parse the file. Incompatible format.");
		}
	}
	bool Parser::IsParsed() const noexcept {
		return !m_Instructions.IsEmpty();
	}

	ByteFile Parser::GetResult() {
		if (!IsParsed()) throw std::runtime_error("Failed to move the result. Incomplete parsing.");

		ByteFile result(std::move(m_Path), std::move(m_ConstantPool), std::move(m_Instructions));
		Clear();
		return result;
	}
	std::string_view Parser::GetPath() const noexcept {
		return m_Path;
	}
	const Instructions& Parser::GetInstructions() const noexcept {
		return m_Instructions;
	}

	void Parser::ParseVer0000() {
		ParseConstantPool();
		ParseInstructions();
	}

	void Parser::ParseConstantPool() {
		const auto intCount = ReadFile<std::uint32_t>();
		const auto longCount = ReadFile<std::uint32_t>();
		const auto doubleCount = ReadFile<std::uint32_t>();

		std::unique_ptr<std::uint8_t[]> constantPool(new std::uint8_t[
			intCount * sizeof(IntObject) +
			longCount * sizeof(LongObject) +
			doubleCount * sizeof(DoubleObject)
		]());

		void* objPtr = ParseConstants<IntObject>(constantPool.get(), intCount);
		objPtr = ParseConstants<LongObject>(objPtr, longCount);
		objPtr = ParseConstants<DoubleObject>(objPtr, doubleCount);

		m_ConstantPool = ConstantPool(std::move(constantPool), intCount, longCount, doubleCount);
	}
	void Parser::ParseInstructions() {
		const auto labelCount = ReadFile<std::uint32_t>();
		const auto instCount = ReadFile<std::uint64_t>();

		std::unique_ptr<std::uint64_t[]> labels(new std::uint64_t[labelCount]());
		std::unique_ptr<Instruction[]> instructions(new Instruction[static_cast<std::size_t>(instCount)]);

		std::uint64_t* labelPtr = labels.get();
		for (std::uint32_t i = 0; i < labelCount; ++i) {
			*labelPtr++ = ReadFile<std::uint64_t>();
		}

		std::uint64_t nextOffset = 0;
		Instruction* instPtr = instructions.get();
		for (std::uint64_t i = 0; i < instCount; ++i) {
			const std::uint8_t opCodeByte = ReadFile<std::uint8_t>();
			std::uint32_t operand = Instruction::NoOperand;
			const std::uint64_t offset = nextOffset;
			if (static_cast<int>(OpCode::Push) <= opCodeByte && opCodeByte <= static_cast<int>(OpCode::Store) ||
				static_cast<int>(OpCode::Jmp) <= opCodeByte && opCodeByte <= static_cast<int>(OpCode::Call)) {
				operand = ReadFile<std::uint32_t>();
				nextOffset += 4;
			}

			*instPtr++ = Instruction(static_cast<OpCode>(opCodeByte), operand, offset);
			++nextOffset;
		}

		m_Instructions = Instructions(std::move(labels), labelCount, std::move(instructions), instCount);
	}
}