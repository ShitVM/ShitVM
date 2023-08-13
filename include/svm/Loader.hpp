#pragma once

#include <svm/core/Loader.hpp>
#include <svm/virtual/VirtualFunction.hpp>

namespace svm {
	class Loader final : public core::Loader<VirtualFunctionInfo> {
	public:
		using core::Loader<VirtualFunctionInfo>::Loader;

	public:
		void LoadStdLibraries();

	private:
		void LoadStdIoLibrary();
	};
}