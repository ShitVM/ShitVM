#pragma once

#include <svm/Instruction.hpp>

#include <string>
#include <string_view>
#include <vector>

namespace svm {
	class ByteFile final {
	private:
		std::string m_Path;
		Instructions m_Instructions;

	public:
		ByteFile() noexcept = default;
		ByteFile(std::string path, Instructions instructions) noexcept;
		ByteFile(const ByteFile& file);
		ByteFile(ByteFile&& file) noexcept;
		~ByteFile() = default;

	public:
		ByteFile& operator=(const ByteFile& file);
		ByteFile& operator=(ByteFile&& file) noexcept;
		bool operator==(const ByteFile&) = delete;
		bool operator!=(const ByteFile&) = delete;

	public:
		std::string_view GetPath() const noexcept;
		const Instructions& GetInstructions() const noexcept;
	};
}