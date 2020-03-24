#pragma once

#include <svm/ConstantPool.hpp>
#include <svm/Function.hpp>
#include <svm/Instruction.hpp>
#include <svm/Structure.hpp>

#include <cstdint>
#include <ostream>
#include <string>
#include <string_view>
#include <vector>

namespace svm {
	struct Mapping final {
		std::uint32_t Module;
		std::uint32_t Index;
	};

	class Mappings final {
	private:
		std::vector<Mapping> m_StructureMappings;
		std::vector<Mapping> m_FunctionMappings;

	public:
		Mappings() = default;
		Mappings(std::vector<Mapping>&& structures, std::vector<Mapping>&& functions) noexcept;
		Mappings(Mappings&& mappings) noexcept;
		~Mappings() = default;

	public:
		Mappings& operator=(Mappings&& mappings) noexcept;
		bool operator==(const Mappings&) = delete;
		bool operator!=(const Mappings&) = delete;

	public:
		void Clear() noexcept;
		bool IsEmpty() const noexcept;

		const Mapping& GetStructureMapping(std::uint32_t index) const noexcept;
		std::uint32_t GetStructureMappingCount() const noexcept;
		const Mapping& GetFunctionMapping(std::uint32_t index) const noexcept;
		std::uint32_t GetFunctionMappingCount() const noexcept;
	};
}

namespace svm {
	class ByteFile final {
	private:
		std::string m_Path;
		std::vector<std::string> m_Dependencies;
		ConstantPool m_ConstantPool;
		Structures m_Structures;
		Functions m_Functions;
		Instructions m_EntryPoint;
		Mappings m_Mappings;

	public:
		ByteFile() noexcept = default;
		ByteFile(std::string path, std::vector<std::string>&& dependencies, ConstantPool&& constantPool,
			Structures&& structures, Functions&& functions, Instructions&& entryPoint,
			Mappings&& mappings) noexcept;
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
		void SetPath(std::string newPath) noexcept;
		const std::vector<std::string>& GetDependencies() const noexcept;
		void SetDependencies(std::vector<std::string>&& newDependencies) noexcept;
		const ConstantPool& GetConstantPool() const noexcept;
		void SetConstantPool(ConstantPool&& newConstantPool) noexcept;
		const Structures& GetStructures() const noexcept;
		void SetStructures(Structures&& newStructures) noexcept;
		const Functions& GetFunctions() const noexcept;
		void SetFunctions(Functions&& newFunctions) noexcept;
		const Instructions& GetEntryPoint() const noexcept;
		void SetEntryPoint(Instructions&& newEntryPoint) noexcept;
		const Mappings& GetMappings() const noexcept;
		void SetMappings(Mappings&& newMappings) noexcept;
	};

	std::ostream& operator<<(std::ostream& stream, const ByteFile& byteFile);
}