#include <svm/Structure.hpp>

#include <utility>

namespace svm {
	StructureData::StructureData(std::vector<Type> fieldTypes, std::vector<std::size_t> fieldOffsets) noexcept
		: m_FieldTypes(std::move(fieldTypes)), m_FieldOffsets(std::move(fieldOffsets)) {}
	StructureData::StructureData(StructureData&& structure) noexcept
		: m_FieldTypes(std::move(structure.m_FieldTypes)), m_FieldOffsets(std::move(structure.m_FieldOffsets)) {}

	StructureData& StructureData::operator=(StructureData&& structure) noexcept {
		m_FieldTypes = std::move(structure.m_FieldTypes);
		m_FieldOffsets = std::move(structure.m_FieldOffsets);
		return *this;
	}

	const std::vector<Type>& StructureData::GetFieldTypes() const noexcept {
		return m_FieldTypes;
	}
	const std::vector<std::size_t>& StructureData::GetFieldOffsets() const noexcept {
		return m_FieldOffsets;
	}
}