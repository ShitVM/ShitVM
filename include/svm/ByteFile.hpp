#pragma once

#include <svm/ConstantPool.hpp>
#include <svm/Instruction.hpp>

#include <string>
#include <string_view>
#include <vector>

namespace svm {
	class ByteFile final {
	private:
		std::string m_Path;
		ConstantPool m_ConstantPool;
		Instructions m_Instructions;

	public:
		ByteFile() noexcept = default;
		ByteFile(std::string path, ConstantPool&& constantPool, Instructions instructions) noexcept;
		ByteFile(ByteFile&& file) noexcept;
		~ByteFile() = default;

	public:
		ByteFile& operator=(ByteFile&& file) noexcept;
		bool operator==(const ByteFile&) = delete;
		bool operator!=(const ByteFile&) = delete;

	public:
		std::string_view GetPath() const noexcept;
		const ConstantPool& GetConstantPool() const noexcept;
		const Instructions& GetInstructions() const noexcept;
	};
}