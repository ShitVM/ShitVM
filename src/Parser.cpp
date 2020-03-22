#include <svm/Parser.hpp>

#include <svm/Memory.hpp>
#include <svm/Type.hpp>

#include <fstream>
#include <ios>
#include <sstream>
#include <stdexcept>

namespace svm {
	Parser::Parser(Parser&& parser) noexcept
		: m_File(std::move(parser.m_File)), m_Pos(parser.m_Pos),
		m_ByteFile(std::move(parser.m_ByteFile)), m_ByteFileVersion(parser.m_ByteFileVersion), m_ByteCodeVersion(parser.m_ByteCodeVersion) {}

	Parser& Parser::operator=(Parser&& parser) noexcept {
		m_File = std::move(parser.m_File);
		m_Pos = parser.m_Pos;

		m_ByteFile = std::move(parser.m_ByteFile);
		m_ByteFileVersion = parser.m_ByteFileVersion;
		m_ByteCodeVersion = parser.m_ByteCodeVersion;
		return *this;
	}

	void Parser::Clear() noexcept {
		m_File.clear();
		m_Pos = 0;

		m_ByteFile.Clear();
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

		m_ByteFile.Clear();
		m_ByteFile.SetPath(path);
	}
	bool Parser::IsLoaded() const noexcept {
		return m_File.size() != 0;
	}
	void Parser::Parse() {
		if (!IsLoaded()) throw std::runtime_error("Failed to parse the file. Incomplete loading.");
		else if (m_File.size() < 36) throw std::runtime_error("Failed to parse the file. Invalid format.");

		static constexpr std::uint8_t magic[] = { 0x74, 0x68, 0x74, 0x68 };
		const auto [magicBegin, magicEnd] = ReadFile(4);
		if (!std::equal(magicBegin, magicEnd, magic)) throw std::runtime_error("Failed to parse the file. Invalid format.");

		m_ByteFileVersion = ReadFile<ByteFileVersion>();
		m_ByteCodeVersion = ReadFile<ByteCodeVersion>();

		if (m_ByteFileVersion > ByteFileVersion::Latest ||
			m_ByteFileVersion < ByteFileVersion::Least) throw std::runtime_error("Failed to parse the file. Incompatible ShitBF version.");
		if (m_ByteCodeVersion > ByteCodeVersion::Latest ||
			m_ByteCodeVersion < ByteCodeVersion::Least) throw std::runtime_error("Failed to parse the file. Incompatible ShitBC version.");

		ParseConstantPool();
		ParseStructures();
		ParseFunctions();
		m_ByteFile.SetEntryPoint(ParseInstructions());
	}
	bool Parser::IsParsed() const noexcept {
		return !m_ByteFile.IsEmpty();
	}

	ByteFile Parser::GetResult() {
		if (!IsParsed()) throw std::runtime_error("Failed to move the result. Incomplete parsing.");

		m_Pos = 0;
		return std::move(m_ByteFile);
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

		m_ByteFile.SetConstantPool({ std::move(intPool), std::move(longPool), std::move(doublePool) });
	}
	void Parser::ParseStructures() {
		const auto structCount = ReadFile<std::uint32_t>();
		std::vector<StructureInfo> structures(structCount);

		for (std::uint32_t i = 0; i < structCount; ++i) {
			const auto fieldCount = ReadFile<std::uint32_t>();
			structures[i].Fields.resize(fieldCount);
			structures[i].Type.Name = "structure" + std::to_string(i);
			structures[i].Type.Code = static_cast<TypeCode>(i + static_cast<std::uint32_t>(TypeCode::Structure));

			for (std::uint32_t j = 0; j < fieldCount; ++j) {
				Field& field = structures[i].Fields[j];

				const auto typeCode = ReadFile<std::uint32_t>();
				field.Type = GetTypeFromTypeCode(structures, static_cast<TypeCode>(typeCode & 0x7FFFFFFF));
				if (typeCode >> 31) {
					field.Count = static_cast<std::size_t>(ReadFile<std::uint64_t>());
				}
			}
		}

		if (structCount >= 2) {
			FindCycle(structures);
		}
		CalcSize(structures);
		CalcOffset(structures);

		m_ByteFile.SetStructures({ std::move(structures) });
	}
	void Parser::ParseFunctions() {
		const auto funcCount = ReadFile<std::uint32_t>();
		Functions functions;
		functions.reserve(funcCount);

		for (std::uint32_t i = 0; i < funcCount; ++i) {
			const auto arity = ReadFile<std::uint16_t>();
			const auto hasResult = ReadFile<bool>();
			auto instructions = ParseInstructions();

			functions.emplace_back(arity, hasResult, std::move(instructions));
		}

		m_ByteFile.SetFunctions(std::move(functions));
	}
	Instructions Parser::ParseInstructions() {
		const auto labelCount = ReadFile<std::uint32_t>();
		std::vector<std::uint64_t> labels(labelCount);
		for (std::uint32_t i = 0; i < labelCount; ++i) {
			labels[i] = ReadFile<std::uint64_t>();
		}

		const auto instCount = ReadFile<std::uint64_t>();
		std::vector<Instruction> insts(static_cast<std::size_t>(instCount));

		std::uint64_t nextOffset = 0;
		for (std::size_t i = 0; i < instCount; ++i) {
			insts[i].OpCode = ReadOpCode();
			insts[i].Offset = nextOffset;
			if (insts[i].HasOperand()) {
				insts[i].Operand = ReadFile<std::uint32_t>();
				nextOffset += 4;
			}
			++nextOffset;
		}

		return { std::move(labels), std::move(insts) };
	}

	void Parser::FindCycle(const std::vector<StructureInfo>& structures) const {
		const std::uint32_t structCount = static_cast<std::uint32_t>(structures.size());
		std::vector<Structure> cycle;

		for (std::uint32_t i = 0; i < structCount; ++i) {
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
		if (status) return status == 1;

		const std::uint32_t fieldCount = static_cast<std::uint32_t>(structures[node].Fields.size());

		status = 1;
		for (std::uint32_t i = 0; i < fieldCount; ++i) {
			const Type type = structures[node].Fields[i].Type;
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

		const std::uint32_t fieldCount = static_cast<std::uint32_t>(structures[node].Fields.size());

		for (std::size_t i = 0; i < fieldCount; ++i) {
			const Field& field = structures[node].Fields[i];
			const Type type = field.Type;
			std::size_t size = type->Size;
			if (type.IsStructure()) {
				size = CalcSize(structures, static_cast<std::uint32_t>(type->Code) - static_cast<std::uint32_t>(TypeCode::Structure));
			}

			if (field.IsArray()) {
				s += size * field.Count + sizeof(std::uint64_t);
			} else {
				s += size;
			}
		}

		s += sizeof(Type);
		return s = Pade(s);
	}
	void Parser::CalcOffset(std::vector<StructureInfo>& structures) const {
		for (std::uint32_t i = 0; i < structures.size(); ++i) {
			StructureInfo& structure = structures[i];
			const std::uint32_t fieldCount = static_cast<std::uint32_t>(structure.Fields.size());

			std::size_t offset = sizeof(Type);
			for (std::uint32_t j = 0; j < fieldCount; ++j) {
				Field& field = structure.Fields[j];

				field.Offset = offset;
				if (field.IsArray()) {
					offset += field.Type->Size * field.Count + sizeof(ArrayObject);
				} else {
					offset += field.Type->Size;
				}
			}
		}
	}
	OpCode Parser::ReadOpCode() noexcept {
		return ReadFile<OpCode>();
	}
}