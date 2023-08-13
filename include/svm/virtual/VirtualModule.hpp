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
		std::uint32_t AddDependency(std::string dependency);
		std::uint32_t AddStructureMapping(std::uint32_t dependency, std::string name);
		std::uint32_t AddFunctionMapping(std::uint32_t dependency, std::string name);
		std::uint32_t AddStructure(std::string name, std::vector<std::pair<Type, std::uint64_t>> fields);
		template<typename F>
		std::uint32_t AddFunction(std::string name, std::uint16_t arity, bool hasResult, F&& function);
	};
}

#include "detail/impl/VirtualModule.hpp"