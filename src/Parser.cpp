#include <svm/Parser.hpp>

#include <svm/Type.hpp>

#include <fstream>
#include <iomanip>
#include <ios>
#include <memory>
#include <sstream>

namespace svm {
	Parser::Parser(Parser&& parser) noexcept
		: m_File(std::move(parser.m_File)), m_Pos(parser.m_Pos),
		m_Path(std::move(parser.m_Path)), m_ConstantPool(std::move(parser.m_ConstantPool)),
		m_Structures(std::move(parser.m_Structures)), m_Functions(std::move(parser.m_Functions)),
		m_EntryPoint(std::move(parser.m_EntryPoint)),
		m_ByteFileVersion(parser.m_ByteFileVersion), m_ByteCodeVersion(parser.m_ByteCodeVersion) {}

	Parser& Parser::operator=(Parser&& parser) noexcept {
		m_File = std::move(parser.m_File);
		m_Pos = parser.m_Pos;

		m_Path = std::move(parser.m_Path);
		m_ConstantPool = std::move(parser.m_ConstantPool);
		m_Structures = std::move(parser.m_Structures);
		m_Functions = std::move(parser.m_Functions);
		m_EntryPoint = std::move(parser.m_EntryPoint);

		m_ByteFileVersion = parser.m_ByteFileVersion;
		m_ByteCodeVersion = parser.m_ByteCodeVersion;
		return *this;
	}

	void Parser::Clear() noexcept {
		m_File.clear();
		m_Pos = 0;

		m_Path.clear();
		m_ConstantPool.Clear();
		m_Structures.Clear();
		m_Functions.clear();
		m_EntryPoint.Clear();

		m_ByteFileVersion = ByteFileVersion::Latest;
		m_ByteCodeVersion = ByteCodeVersion::Latest;
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
		m_Structures.Clear();
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

		m_ByteFileVersion = ReadFile<ByteFileVersion>();
		m_ByteCodeVersion = ReadFile<ByteCodeVersion>();

		if (m_ByteFileVersion > ByteFileVersion::Latest) throw std::runtime_error("Failed to parse the file. Incompatible ShitVM Byte File version.");
		if (m_ByteCodeVersion > ByteCodeVersion::Latest ||
			m_ByteCodeVersion < ByteCodeVersion::Least) throw std::runtime_error("Failed to parse the file. Incompatible ShitBC version.");

		ParseConstantPool();
		if (m_ByteFileVersion >= ByteFileVersion::v0_2_0) {
			ParseStructures();
		}
		ParseFunctions();
		m_EntryPoint = ParseInstructions();
	}
	bool Parser::IsParsed() const noexcept {
		return !m_Functions.empty() || !m_EntryPoint.IsEmpty();
	}

	ByteFile Parser::GetResult() {
		if (!IsParsed()) throw std::runtime_error("Failed to move the result. Incomplete parsing.");

		ByteFile result(std::move(m_Path), std::move(m_ConstantPool), std::move(m_Structures),
			std::move(m_Functions), std::move(m_EntryPoint));
		Clear();
		return result;
	}
	std::string_view Parser::GetPath() const noexcept {
		return m_Path;
	}
	const ConstantPool& Parser::GetConstantPool() const noexcept {
		return m_ConstantPool;
	}
	const Structures& Parser::GetStructures() const noexcept {
		return m_Structures;
	}
	const Functions& Parser::GetFunctions() const noexcept {
		return m_Functions;
	}
	const Instructions& Parser::GetEntryPoint() const noexcept {
		return m_EntryPoint;
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
	void Parser::ParseStructures() {
		const auto structCount = ReadFile<std::uint32_t>();
		std::vector<StructureInfo> structures(structCount);

		for (std::uint32_t i = 0; i < structCount; ++i) {
			const auto fieldCount = ReadFile<std::uint32_t>();
			structures[i].FieldTypes.resize(fieldCount);
			structures[i].Type.Name = "structure" + std::to_string(i);
			structures[i].Type.Code = static_cast<TypeCode>(i + static_cast<std::uint32_t>(TypeCode::Structure));
			structures[i].Type.Size = 0;

			for (std::uint32_t j = 0; j < fieldCount; ++j) {
				structures[i].FieldTypes[j] = GetTypeFromTypeCode(structures, ReadFile<TypeCode>());
			}
		}

		if (structCount >= 2) {
			FindCycle(structures);
		}
		CalcSize(structures);

		m_Structures = std::move(structures);
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
			const OpCode opCode = ReadOpCode();
			std::uint32_t operand = Instruction::NoOperand;
			const std::uint64_t offset = nextOffset;

			if (OpCode::Push <= opCode && opCode <= OpCode::FLea && opCode != OpCode::Pop ||
				OpCode::Inc <= opCode && opCode <= OpCode::Dec ||
				OpCode::Jmp <= opCode && opCode <= OpCode::Call) {
				operand = ReadFile<std::uint32_t>();
				nextOffset += 4;
			}

			insts.emplace_back(opCode, operand, offset);
			++nextOffset;
		}

		return { std::move(labels), std::move(insts) };
	}

	void Parser::FindCycle(const std::vector<StructureInfo>& structures) const {
		std::vector<Structure> cycle;
		for (std::uint32_t i = 0; i < static_cast<std::uint32_t>(structures.size()); ++i) {
			std::unordered_map<std::uint32_t, int> visited;
			if (FindCycle(structures, visited, cycle, i)) {
				std::ostringstream oss;
				oss << "Failed to parse the file. Detected circular reference in the structures([" << i << ']';
				for (auto iter = cycle.rbegin(); iter < cycle.rend(); ++iter) {
					oss << "-[" << static_cast<std::uint32_t>((*iter)->Type.Code) - static_cast<std::uint32_t>(TypeCode::Structure) << ']';
				}
				oss << ").";
				throw std::runtime_error(oss.str());
			}
		}
	}
	bool Parser::FindCycle(const std::vector<StructureInfo>& structures, std::unordered_map<std::uint32_t, int>& visited, std::vector<Structure>& cycle, std::uint32_t node) const {
		int& status = visited[node];
		if (status) return visited[node] == 1;

		status = 1;
		for (std::size_t i = 0; i < structures[node].FieldTypes.size(); ++i) {
			const Type type = structures[node].FieldTypes[i];
			if (!type.IsStructure()) continue;
			else if (const auto index = static_cast<std::uint32_t>(type->Code) - static_cast<std::uint32_t>(TypeCode::Structure);
				FindCycle(structures, visited, cycle, index)) {
				cycle.push_back(structures[index]);
				return true;
			}
		}
		status = 2;
		return false;
	}
	void Parser::CalcSize(std::vector<StructureInfo>& structures) const {
		for (std::uint32_t i = 0; i < static_cast<std::uint32_t>(structures.size()); ++i) {
			structures[i].Type.Size = CalcSize(structures, i);
		}
	}
	std::size_t Parser::CalcSize(std::vector<StructureInfo>& structures, std::uint32_t node) const {
		std::size_t& s = structures[node].Type.Size;
		if (s) return s;

		for (std::size_t i = 0; i < structures[node].FieldTypes.size(); ++i) {
			const Type type = structures[node].FieldTypes[i];
			if (!type.IsStructure()) {
				s += type->Size;
			} else {
				s += CalcSize(structures, static_cast<std::uint32_t>(type->Code) - static_cast<std::uint32_t>(TypeCode::Structure));
			}
		}

		s += sizeof(Type);
		return s = PadeSize(s);
	}
	std::size_t Parser::PadeSize(std::size_t size) const noexcept {
		const auto temp = size / sizeof(void*) * sizeof(void*);
		if (size == temp) return size;
		else return temp + sizeof(void*);
	}
	OpCode Parser::ReadOpCode() noexcept {
		return ReadFile<OpCode>();
	}
}