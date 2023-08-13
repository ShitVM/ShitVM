#include <svm/virtual/VirtualModule.hpp>

namespace svm {
	std::uint32_t VirtualModule::AddDependency(std::string dependency) {
		GetDependencies().push_back(std::move(dependency));
		return static_cast<std::uint32_t>(GetDependencies().size() - 1);
	}
	std::uint32_t VirtualModule::AddStructureMapping(std::uint32_t dependency, std::string name) {
		GetMappings().AddStructureMapping(dependency, name);
		return GetMappings().GetStructureMappingCount() - 1;
	}
	std::uint32_t VirtualModule::AddFunctionMapping(std::uint32_t dependency, std::string name) {
		GetMappings().AddFunctionMapping(dependency, std::move(name));
		return GetMappings().GetFunctionMappingCount() - 1;
	}
	std::uint32_t VirtualModule::AddStructure(std::string name, std::vector<std::pair<Type, std::uint64_t>> fields) {
		const auto index = static_cast<std::uint32_t>(GetStructures().size());
		
		StructureInfo& structure = GetStructures().emplace_back();
		structure.Type.Name = name;
		structure.Name = std::move(name);
		structure.Type.Code = static_cast<TypeCode>(index + static_cast<std::uint32_t>(TypeCode::Structure));

		for (const auto& [type, count] : fields) {
			Field& field = structure.Fields.emplace_back();
			field.Type = type;
			field.Count = count;
		}

		return index;
	}
}