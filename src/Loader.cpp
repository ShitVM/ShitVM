#include <svm/Loader.hpp>

#include <svm/ByteFile.hpp>
#include <svm/Parser.hpp>
#include <svm/detail/FileSystem.hpp>

#include <utility>

namespace svm {
	Loader::Loader(Loader&& loader) noexcept
		: m_Modules(std::move(loader.m_Modules)) {}

	Loader& Loader::operator=(Loader&& loader) noexcept {
		m_Modules = std::move(loader.m_Modules);
		return *this;
	}

	void Loader::Clear() noexcept {
		m_Modules.clear();
	}
	Module Loader::Load(const std::string& path) {
		Parser parser;
		parser.Load(path);
		parser.Parse();

		ModuleInfo& module = (m_Modules[detail::fs::absolute(path).string()] = parser.GetResult());
		ByteFile& byteFile = std::get<ByteFile>(module.Module);

		// TODO

		return module;
	}
	VirtualModule Loader::Create(const std::string& virtualPath) {
		ModuleInfo& module = (m_Modules[detail::fs::absolute(virtualPath).string()] = VirtualModule());
		return std::move(std::get<VirtualModule>(module.Module));
	}
}