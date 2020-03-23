#pragma once

#include <svm/Module.hpp>
#include <svm/VirtualModule.hpp>

#include <string>
#include <unordered_map>

namespace svm {
	class Loader final {
	private:
		std::unordered_map<std::string, ModuleInfo> m_Modules;

	public:
		Loader() = default;
		Loader(Loader&& loader) noexcept;
		~Loader() = default;

	public:
		Loader& operator=(Loader&& loader) noexcept;
		bool operator==(const Loader&) = delete;
		bool operator!=(const Loader&) = delete;

	public:
		void Clear() noexcept;
		Module Load(const std::string& path);
		VirtualModule Create(const std::string& virtualPath);
	};
}