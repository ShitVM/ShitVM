#pragma once

#include <svm/Type.hpp>
#include <svm/core/virtual/VirtualModule.hpp>
#include <svm/virtual/VirtualFunction.hpp>

#include <cstdint>
#include <string>
#include <utility>
#include <vector>

namespace svm {
	class VirtualModule final : public core::VirtualModule<VirtualFunctionInfo> {
	public:
		enum class DependencyIndex : std::uint32_t {};
		enum class StructureIndex : std::uint32_t {};
		enum class MappedStructureIndex : std::uint32_t {};

	public:
		using core::VirtualModule<VirtualFunctionInfo>::VirtualModule;

	private:
		using core::VirtualModule<VirtualFunctionInfo>::GetDependencies;
		using core::VirtualModule<VirtualFunctionInfo>::SetDependencies;
		using core::VirtualModule<VirtualFunctionInfo>::GetStructures;
		using core::VirtualModule<VirtualFunctionInfo>::SetStructures;
		using core::VirtualModule<VirtualFunctionInfo>::GetFunctions;
		using core::VirtualModule<VirtualFunctionInfo>::SetFunctions;
		using core::VirtualModule<VirtualFunctionInfo>::GetMappings;
		using core::VirtualModule<VirtualFunctionInfo>::SetMappings;

		using core::VirtualModule<VirtualFunctionInfo>::UpdateStructureInfos;

	public:
		DependencyIndex AddDependency(std::string dependency);
		MappedStructureIndex AddStructureMapping(DependencyIndex dependency, std::string name);
		StructureIndex AddStructure(std::string name, std::vector<std::pair<Type, std::uint64_t>> fields);
		template<typename F>
		void AddFunction(std::string name, std::uint16_t arity, bool hasResult, F&& function);
	};
}

#include "detail/impl/VirtualModule.hpp"