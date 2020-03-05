#pragma once

#include <svm/ConstantPool.hpp>
#include <svm/Function.hpp>
#include <svm/Instruction.hpp>
#include <svm/Structure.hpp>

#include <ostream>
#include <string>
#include <string_view>

namespace svm {
	class ByteFile final {
	private:
		std::string m_Path;
		ConstantPool m_ConstantPool;
		Structures m_Structures;
		Functions m_Functions;
		Instructions m_EntryPoint;

	public:
		ByteFile() noexcept = default;
		ByteFile(std::string path, ConstantPool&& constantPool, Structures&& structures, Functions&& functions, Instructions&& entryPoint) noexcept;
		ByteFile(ByteFile&& file) noexcept;
		~ByteFile() = default;

	public:
		ByteFile& operator=(ByteFile&& file) noexcept;
		bool operator==(const ByteFile&) = delete;
		bool operator!=(const ByteFile&) = delete;

	public:
		void Clear() noexcept;
		bool IsEmpty() const noexcept;

		std::string_view GetPath() const noexcept;
		const ConstantPool& GetConstantPool() const noexcept;
		const Structures& GetStructures() const noexcept;
		const Functions& GetFunctions() const noexcept;
		const Instructions& GetEntryPoint() const noexcept;
	};

	std::ostream& operator<<(std::ostream& stream, const ByteFile& byteFile);
}