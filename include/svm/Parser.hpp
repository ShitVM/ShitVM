#pragma once

#include <svm/ByteFile.hpp>
#include <svm/Instruction.hpp>
#include <svm/Memory.hpp>
#include <svm/Object.hpp>

#include <cstddef>
#include <cstdint>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

namespace svm {
	class Parser final {
	private:
		std::vector<std::uint8_t> m_File;
		std::size_t m_Pos = 0;

		std::string m_Path;
		ConstantPool m_ConstantPool;
		Instructions m_Instructions;

	public:
		Parser() noexcept = default;
		Parser(Parser&& parser) noexcept;
		~Parser() = default;

	public:
		Parser& operator=(Parser&& parser) noexcept;
		bool operator==(const Parser&) = delete;
		bool operator!=(const Parser&) = delete;

	public:
		void Clear() noexcept;
		void Load(const std::string& path);
		bool IsLoaded() const noexcept;
		void Parse();
		bool IsParsed() const noexcept;

		ByteFile GetResult();
		std::string_view GetPath() const noexcept;
		const Instructions& GetInstructions() const noexcept;

	private:
		template<typename T>
		T ReadFile() noexcept {
			T result = *reinterpret_cast<T*>(&m_File[m_Pos]);
			m_Pos += sizeof(T);

			if constexpr (sizeof(T) > 1) {
				return GetEndian() == Endian::Little ? result : ReverseEndian(result);
			} else return result;
		}
		std::pair<std::vector<std::uint8_t>::iterator, std::vector<std::uint8_t>::iterator> ReadFile(std::size_t size) noexcept {
			const auto begin = m_File.begin() + m_Pos;
			const auto end = m_File.begin() + m_Pos + size;
			m_Pos += size;
			return { begin, end };
		}

		void ParseVer0000();

		void ParseConstantPool();
		template<typename T>
		void* ParseConstants(void* begin, std::uint32_t count) {
			static_assert(std::is_base_of_v<Object, T>);

			T obj;
			T* objPtr = static_cast<T*>(begin);
			for (std::uint32_t i = 0; i < count; ++i) {
				obj.Value = ReadFile<decltype(obj.Value)>();
				*objPtr++ = obj;
			}
			return objPtr;
		}
		void ParseFunctions();
	};
}