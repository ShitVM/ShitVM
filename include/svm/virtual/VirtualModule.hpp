#pragma once

#include <svm/Structure.hpp>
#include <svm/virtual/VirtualFunction.hpp>

#include <vector>

namespace svm {
	class VirtualModule final {
	private:
		Structures m_Structures;
		VirtualFunctions m_Functions;

	public:
		VirtualModule() noexcept = default;
		VirtualModule(Structures&& structures, VirtualFunctions&& functions) noexcept;
		VirtualModule(VirtualModule&& module) noexcept;
		~VirtualModule() = default;

	public:
		VirtualModule& operator=(VirtualModule&& module) noexcept;
		bool operator==(const VirtualModule&) = delete;
		bool operator!=(const VirtualModule&) = delete;

	public:
		void Clear() noexcept;
		bool IsEmpty() const noexcept;

		const Structures& GetStructures() const noexcept;
		const VirtualFunctions& GetFunctions() const noexcept;
	};
}