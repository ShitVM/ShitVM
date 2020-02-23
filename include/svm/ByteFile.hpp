#pragma once

#include <svm/ConstantPool.hpp>
#include <svm/Function.hpp>

#include <string>
#include <string_view>

namespace svm {
	class ByteFile final {
	private:
		std::string m_Path;
		ConstantPool m_ConstantPool;
		Functions m_Functions;

	public:
		ByteFile() noexcept = default;
		ByteFile(std::string path, ConstantPool&& constantPool, Functions&& functions) noexcept;
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
		const Functions& GetFunctions() const noexcept;
	};
}