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
	enum class ByteFileVersion : std::uint16_t {
		v0_1_0,

		Latest = v0_1_0,
	};

	enum class ByteCodeVersion : std::uint16_t {
		v0_1_0,
		v0_2_0,

		Latest = v0_2_0,
	};

	class Parser final {
	private:
		std::vector<std::uint8_t> m_File;
		std::size_t m_Pos = 0;

		std::string m_Path;
		ConstantPool m_ConstantPool;
		Functions m_Functions;
		Instructions m_EntryPoint;

		ByteFileVersion m_ByteFileVersion = ByteFileVersion::Latest;
		ByteCodeVersion m_ByteCodeVersion = ByteCodeVersion::Latest;

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
		const ConstantPool& GetConstantPool() const noexcept;
		const Functions& GetFunctions() const noexcept;
		const Instructions& GetEntryPoint() const noexcept;

	private:
		template<typename T>
		T ReadFile() noexcept {
			T result = *reinterpret_cast<T*>(&m_File[m_Pos]);
			m_Pos += sizeof(T);

			if constexpr (sizeof(T) > 1) {
				return GetEndian() == Endian::Little ? result : ReverseEndian(result);
			} else return result;
		}
		auto ReadFile(std::size_t size) noexcept {
			const auto begin = m_File.begin() + m_Pos;
			const auto end = m_File.begin() + m_Pos + size;
			m_Pos += size;
			return std::make_pair(begin, end);
		}

		void ParseConstantPool();
		template<typename T>
		void ParseConstants(std::vector<T>& pool) {
			static_assert(std::is_base_of_v<Object, T>);
			for (T& obj : pool) {
				obj.Value = ReadFile<decltype(obj.Value)>();
			}
		}
		void ParseFunctions();
		Instructions ParseInstructions();

		OpCode ReadOpCode() noexcept;
	};
}