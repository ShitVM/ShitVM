#pragma once

#include <svm/Type.hpp>
#include <svm/detail/ReferenceWrapper.hpp>

#include <cstdint>
#include <vector>

namespace svm {
	class StructureData final {
	private:
		std::vector<Type> m_FieldTypes;
		std::vector<std::size_t> m_FieldOffsets;

	public:
		StructureData() noexcept = default;
		StructureData(std::vector<Type> fieldTypes, std::vector<std::size_t> fieldOffsets) noexcept;
		StructureData(StructureData&& structure) noexcept;
		~StructureData() = default;

	public:
		StructureData& operator=(StructureData&& structure) noexcept;
		bool operator==(const StructureData&) = delete;
		bool operator!=(const StructureData&) = delete;

	public:
		const std::vector<Type>& GetFieldTypes() const noexcept;
		const std::vector<std::size_t>& GetFieldOffsets() const noexcept;
	};
}

namespace svm {
	class Structure final : public detail::ReferenceWrapper<StructureData> {
	public:
		using detail::ReferenceWrapper<StructureData>::ReferenceWrapper;
	};
}