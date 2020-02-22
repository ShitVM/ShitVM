#include <svm/ByteFile.hpp>

#include <utility>

namespace svm {
	ByteFile::ByteFile(std::string path, ConstantPool&& constantPool, Instructions instructions) noexcept
		: m_Path(std::move(path)), m_ConstantPool(std::move(constantPool)), m_Instructions(std::move(instructions)) {}
	ByteFile::ByteFile(ByteFile&& file) noexcept
		: m_Path(std::move(file.m_Path)), m_ConstantPool(std::move(file.m_ConstantPool)), m_Instructions(std::move(file.m_Instructions)) {}

	ByteFile& ByteFile::operator=(ByteFile&& file) noexcept {
		m_Path = std::move(file.m_Path);
		m_ConstantPool = std::move(file.m_ConstantPool);
		m_Instructions = std::move(file.m_Instructions);
		return *this;
	}

	std::string_view ByteFile::GetPath() const noexcept {
		return m_Path;
	}
	const ConstantPool& ByteFile::GetConstantPool() const noexcept {
		return m_ConstantPool;
	}
	const Instructions& ByteFile::GetInstructions() const noexcept {
		return m_Instructions;
	}
}