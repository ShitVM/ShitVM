#pragma once

#include <svm/core/Loader.hpp>
#include <svm/virtual/VirtualFunction.hpp>
#include <svm/virtual/VirtualModule.hpp>

#include <memory>
#include <string>

namespace svm {
	namespace detail {
		class LoaderAdapter : public core::Loader<VirtualFunctionInfo> {
		public:
			using core::Loader<VirtualFunctionInfo>::Loader;

		protected:
			VirtualModule& CreateWrapped(std::string virtualPath);

		private:
			using core::Loader<VirtualFunctionInfo>::Create;
		};
	}
	
	class Loader final : public detail::LoaderAdapter {
	public:
		using detail::LoaderAdapter::LoaderAdapter;

	public:
		VirtualModule& Create(std::string virtualPath);
	};
}

namespace svm {
	namespace detail {
		struct StdModuleState;
	}

	using StdModule = std::shared_ptr<detail::StdModuleState>;

	StdModule InitStdModule(Loader& loader);
}