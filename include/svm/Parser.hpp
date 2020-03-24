#pragma once

#include <svm/ByteFile.hpp>
#include <svm/Structure.hpp>

#include <cstddef>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace svm {
	enum class ByteFileVersion : std::uint16_t {
		v0_4_0 = 3,

		Least = v0_4_0,
		Latest = v0_4_0,
	};

	enum class ByteCodeVersion : std::uint16_t {
		v0_4_0 = 3,

		Least = v0_4_0,
		Latest = v0_4_0,
	};

	class Parser final {
	private:
		std::vector<std::uint8_t> m_File;
		std::size_t m_Pos = 0;

		ByteFile m_ByteFile;
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

	private:
		template<typename T>
		T ReadFile() noexcept;
		inline auto ReadFile(std::size_t size) noexcept;

		void ParseDependencies();
		void ParseConstantPool();
		template<typename T>
		void ParseConstants(std::vector<T>& pool) noexcept;
		void ParseStructures();
		void ParseFunctions();
		Instructions ParseInstructions();
		void ParseMappings();
		void ParseMappings(std::vector<Mapping>& mappings) noexcept;

		void FindCycle(const std::vector<StructureInfo>& structures) const;
		bool FindCycle(const std::vector<StructureInfo>& structures, std::unordered_map<std::uint32_t, int>& visited, std::vector<Structure>& cycle, std::uint32_t node) const;
		void CalcSize(std::vector<StructureInfo>& structures) const;
		std::size_t CalcSize(std::vector<StructureInfo>& structures, std::uint32_t node) const;
		void CalcOffset(std::vector<StructureInfo>& structures) const;
		OpCode ReadOpCode() noexcept;
	};
}

#include "detail/impl/Parser.hpp"