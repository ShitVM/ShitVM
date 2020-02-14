#include <svm/ByteFile.hpp>

#include <utility>

namespace svm {
	ByteFile::ByteFile(std::string path, Instructions instructions) noexcept
		: m_Path(std::move(path)), m_Instructions(std::move(instructions)) {}
	ByteFile::ByteFile(const ByteFile& file)
		: m_Path(file.m_Path), m_Instructions(file.m_Instructions) {}
	ByteFile::ByteFile(ByteFile&& file) noexcept
		: m_Path(std::move(file.m_Path)), m_Instructions(std::move(file.m_Instructions)) {}

	ByteFile& ByteFile::operator=(const ByteFile& file) {
		m_Path = file.m_Path;
		m_Instructions = file.m_Instructions;
		return *this;
	}
	ByteFile& ByteFile::operator=(ByteFile&& file) noexcept {
		m_Path = std::move(file.m_Path);
		m_Instructions = std::move(file.m_Instructions);
		return *this;
	}

	std::string_view ByteFile::GetPath() const noexcept {
		return m_Path;
	}
	const Instructions& ByteFile::GetInstructions() const noexcept {
		return m_Instructions;
	}
}