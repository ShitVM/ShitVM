#include <svm/ByteFile.hpp>

#include <svm/IO.hpp>

#include <utility>

namespace svm {
	Mappings::Mappings(std::vector<Mapping>&& structures, std::vector<Mapping>&& functions) noexcept
		: m_StructureMappings(std::move(structures)), m_FunctionMappings(std::move(functions)) {}
	Mappings::Mappings(Mappings&& mappings) noexcept
		: m_StructureMappings(std::move(mappings.m_StructureMappings)), m_FunctionMappings(std::move(mappings.m_FunctionMappings)) {}

	Mappings& Mappings::operator=(Mappings&& mappings) noexcept {
		m_StructureMappings = std::move(mappings.m_StructureMappings);
		m_FunctionMappings = std::move(mappings.m_FunctionMappings);
		return *this;
	}

	void Mappings::Clear() noexcept {
		m_StructureMappings.clear();
		m_FunctionMappings.clear();
	}
	bool Mappings::IsEmpty() const noexcept {
		return m_StructureMappings.empty() && m_FunctionMappings.empty();
	}

	const Mapping& Mappings::GetStructureMapping(std::uint32_t index) const noexcept {
		return m_StructureMappings[index];
	}
	std::uint32_t Mappings::GetStructureMappingCount() const noexcept {
		return static_cast<std::uint32_t>(m_StructureMappings.size());
	}
	const Mapping& Mappings::GetFunctionMapping(std::uint32_t index) const noexcept {
		return m_FunctionMappings[index];
	}
	std::uint32_t Mappings::GetFunctionMappingCount() const noexcept {
		return static_cast<std::uint32_t>(m_FunctionMappings.size());
	}
}

namespace svm {
	ByteFile::ByteFile(std::string path, std::vector<std::string>&& dependencies, ConstantPool&& constantPool,
		Structures&& structures, Functions&& functions, Instructions&& entryPoint,
		Mappings&& mappings) noexcept
		: m_Path(std::move(path)), m_Dependencies(std::move(dependencies)), m_ConstantPool(std::move(constantPool)),
		m_Structures(std::move(structures)), m_Functions(std::move(functions)), m_EntryPoint(std::move(entryPoint)),
		m_Mappings(std::move(mappings)) {}
	ByteFile::ByteFile(ByteFile&& file) noexcept
		: m_Path(std::move(file.m_Path)), m_Dependencies(std::move(file.m_Dependencies)), m_ConstantPool(std::move(file.m_ConstantPool)),
		m_Structures(std::move(file.m_Structures)), m_Functions(std::move(file.m_Functions)), m_EntryPoint(std::move(file.m_EntryPoint)),
		m_Mappings(std::move(file.m_Mappings)) {}

	ByteFile& ByteFile::operator=(ByteFile&& file) noexcept {
		m_Path = std::move(file.m_Path);
		m_Dependencies = std::move(file.m_Dependencies);
		m_ConstantPool = std::move(file.m_ConstantPool);
		m_Structures = std::move(file.m_Structures);
		m_Functions = std::move(file.m_Functions);
		m_EntryPoint = std::move(file.m_EntryPoint);
		m_Mappings = std::move(file.m_Mappings);
		return *this;
	}

	void ByteFile::Clear() noexcept {
		m_Path.clear();
		m_Dependencies.clear();
		m_ConstantPool.Clear();
		m_Structures.Clear();
		m_Functions.clear();
		m_EntryPoint.Clear();
		m_Mappings.Clear();
	}
	bool ByteFile::IsEmpty() const noexcept {
		return m_Path.empty() && m_Dependencies.empty() && m_ConstantPool.IsEmpty() &&
			m_Structures.IsEmpty() && m_Functions.empty() && m_EntryPoint.IsEmpty() &&
			m_Mappings.IsEmpty();
	}

	std::string_view ByteFile::GetPath() const noexcept {
		return m_Path;
	}
	void ByteFile::SetPath(std::string newPath) noexcept {
		m_Path = std::move(newPath);
	}
	const std::vector<std::string>& ByteFile::GetDependencies() const noexcept {
		return m_Dependencies;
	}
	void ByteFile::SetDependencies(std::vector<std::string>&& newDependencies) noexcept {
		m_Dependencies = std::move(newDependencies);
	}
	const ConstantPool& ByteFile::GetConstantPool() const noexcept {
		return m_ConstantPool;
	}
	void ByteFile::SetConstantPool(ConstantPool&& newConstantPool) noexcept {
		m_ConstantPool = std::move(newConstantPool);
	}
	const Structures& ByteFile::GetStructures() const noexcept {
		return m_Structures;
	}
	void ByteFile::SetStructures(Structures&& newStructures) noexcept {
		m_Structures = std::move(newStructures);
	}
	const Functions& ByteFile::GetFunctions() const noexcept {
		return m_Functions;
	}
	void ByteFile::SetFunctions(Functions&& newFunctions) noexcept {
		m_Functions = std::move(newFunctions);
	}
	const Instructions& ByteFile::GetEntryPoint() const noexcept {
		return m_EntryPoint;
	}
	void ByteFile::SetEntryPoint(Instructions&& newEntryPoint) noexcept {
		m_EntryPoint = std::move(newEntryPoint);
	}
	const Mappings& ByteFile::GetMappings() const noexcept {
		return m_Mappings;
	}
	void ByteFile::SetMappings(Mappings&& newMappings) noexcept {
		m_Mappings = std::move(newMappings);
	}

	void ByteFile::UpdateStructureCodes(std::uint32_t offset) noexcept {
		for (std::uint32_t i = 0; i < m_Structures.GetStructureCount(); ++i) {
			const_cast<StructureInfo&>(*m_Structures[i]).Type.Code = static_cast<TypeCode>(static_cast<std::uint32_t>(m_Structures[i]->Type.Code) + offset);
		}
	}

	std::ostream& operator<<(std::ostream& stream, const ByteFile& byteFile) {
		const std::string defIndent = detail::MakeTabs(stream);

		stream << defIndent << "ByteFile:\n"
			   << defIndent << "\tPath: \"" << byteFile.GetPath() << "\"\n"
			   << Indent << byteFile.GetConstantPool() << '\n'
						 << byteFile.GetStructures() << '\n'
						 << byteFile.GetFunctions() << '\n' << UnIndent
			   << defIndent << "\tEntryPoint:\n"
			   << Indent << Indent << byteFile.GetEntryPoint() << UnIndent << UnIndent;
		return stream;
	}
}