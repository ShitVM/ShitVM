#include <svm/ByteFile.hpp>

#include <svm/IO.hpp>

#include <utility>

namespace svm {
	ByteFile::ByteFile(std::string path, ConstantPool&& constantPool, Structures&& structures, Functions&& functions, Instructions&& entryPoint) noexcept
		: m_Path(std::move(path)), m_ConstantPool(std::move(constantPool)), m_Structures(std::move(structures)),
		m_Functions(std::move(functions)), m_EntryPoint(std::move(entryPoint)) {}
	ByteFile::ByteFile(ByteFile&& file) noexcept
		: m_Path(std::move(file.m_Path)), m_ConstantPool(std::move(file.m_ConstantPool)), m_Structures(std::move(file.m_Structures)),
		m_Functions(std::move(file.m_Functions)), m_EntryPoint(std::move(file.m_EntryPoint)) {}

	ByteFile& ByteFile::operator=(ByteFile&& file) noexcept {
		m_Path = std::move(file.m_Path);
		m_ConstantPool = std::move(file.m_ConstantPool);
		m_Structures = std::move(file.m_Structures);
		m_Functions = std::move(file.m_Functions);
		m_EntryPoint = std::move(file.m_EntryPoint);
		return *this;
	}

	void ByteFile::Clear() noexcept {
		m_Path.clear();
		m_ConstantPool.Clear();
		m_Structures.Clear();
		m_Functions.clear();
		m_EntryPoint.Clear();
	}
	bool ByteFile::IsEmpty() const noexcept {
		return m_Path.empty() && m_ConstantPool.IsEmpty() && m_Structures.IsEmpty() &&
			m_Functions.empty() && m_EntryPoint.IsEmpty();
	}

	std::string_view ByteFile::GetPath() const noexcept {
		return m_Path;
	}
	const ConstantPool& ByteFile::GetConstantPool() const noexcept {
		return m_ConstantPool;
	}
	const Structures& ByteFile::GetStructures() const noexcept {
		return m_Structures;
	}
	const Functions& ByteFile::GetFunctions() const noexcept {
		return m_Functions;
	}
	const Instructions& ByteFile::GetEntryPoint() const noexcept {
		return m_EntryPoint;
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