#include <svm/virtual/VirtualModule.hpp>

namespace svm {
	VirtualModule::DependencyIndex VirtualModule::AddDependency(std::string dependency) {
		GetDependencies().push_back(std::move(dependency));
		return static_cast<DependencyIndex>(GetDependencies().size() - 1);
	}
	VirtualModule::MappedStructureIndex VirtualModule::AddStructureMapping(DependencyIndex dependency, std::string name) {
		GetMappings().AddStructureMapping(static_cast<std::uint32_t>(dependency), name);
		return static_cast<MappedStructureIndex>(GetMappings().GetStructureMappingCount() - 1);
	}
	VirtualModule::StructureIndex VirtualModule::AddStructure(std::string name, std::vector<std::pair<Type, std::uint64_t>> fields) {
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

		return static_cast<StructureIndex>(index);
	}
}