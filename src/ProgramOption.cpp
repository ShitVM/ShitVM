#include <svm/ProgramOption.hpp>

#include <filesystem>
#include <iostream>
#include <utility>

namespace svm {
	ProgramOption::ProgramOption(ProgramOption&& option) noexcept
		: Path(std::move(option.Path)),
		Options(std::move(option.Options)), Variables(std::move(option.Variables)), Flags(std::move(option.Flags)) {}

	ProgramOption& ProgramOption::operator=(ProgramOption&& option) noexcept {
		Path = std::move(option.Path);

		Options = std::move(option.Options);
		Variables = std::move(option.Variables);
		Flags = std::move(option.Flags);
		return *this;
	}

	void ProgramOption::Clear() noexcept {
		Path.clear();

		Options.clear();
		Variables.clear();
		Flags.clear();
	}

	ProgramOption& ProgramOption::AddOption(const char* name) {
		Options[name].DefaultValue = false;
		return *this;
	}
	ProgramOption& ProgramOption::AddVariable(const char* name, std::uint64_t defaultValue) {
		Variables[name].DefaultValue = defaultValue;
		return *this;
	}
	ProgramOption& ProgramOption::AddFlag(const char* name, bool defaultValue) {
		Flags[name].DefaultValue = defaultValue;
		return *this;
	}
	ProgramOption& ProgramOption::AddStringList(char prefix) {
		StringLists[prefix];
		return *this;
	}
	bool ProgramOption::GetOption(const char* name) const {
		return *Options.at(name).Value;
	}
	std::uint64_t ProgramOption::GetVariable(const char* name) const {
		return *Variables.at(name).Value;
	}
	bool ProgramOption::GetFlag(const char* name) const {
		return *Flags.at(name).Value;
	}
	const std::vector<std::string>& ProgramOption::GetStringList(char prefix) const {
		return StringLists.at(prefix);
	}

	bool ProgramOption::Parse(int argc, char* argv[]) {
		if (argc == 1) {
			std::cout << "Usage: ./ShitVM <File> [Options...]\n";
			return false;
		}

		for (int i = 1; i < argc; ++i) {
			const std::string_view arg = argv[i];
			if (arg.front() == '-') {
				if (arg.size() == 1) {
					std::cout << "Error: Invalid command line option '-'.\n";
					return false;
				}

				const std::string_view option = arg.substr(1);
				if (option.compare(0, 4, "fno-") == 0) { // Flag
					const std::string_view flag = option.substr(4);
					const auto iter = Flags.find(flag);
					if (iter == Flags.end()) {
						std::cout << "Error: Unknown flag '" << flag << "'.\n";
						return false;
					}
					iter->second.Value = false;
				} else if (option.front() == 'f') { // Flag
					const std::string_view flag = option.substr(1);
					const auto iter = Flags.find(flag);
					if (iter == Flags.end()) {
						std::cout << "Error: Unknown flag '" << flag << "'.\n";
						return false;
					}
					iter->second.Value = true;
				} else if (option.size() >= 2 && option.front() == '-') { // Option
					const std::string_view opt = option.substr(1);
					const auto iter = Options.find(opt);
					if (iter == Options.end()) {
						std::cout << "Error: Unknown option '" << opt << "'.\n";
						return false;
					}
					iter->second.Value = true;
				} else {
					const std::size_t assign = option.find('=');
					if (assign != std::string_view::npos) { // Variable
						const std::string_view var = option.substr(0, assign);
						const std::string value(option.substr(assign + 1));
						const auto iter = Variables.find(var);
						if (iter == Variables.end()) {
							std::cout << "Error: Unknown variable '" << var << "'.\n";
							return false;
						}
						iter->second.Value = static_cast<std::uint64_t>(std::stoull(value));
					} else if (option.size() >= 2) { // StringList
						const char prefix = option[0];
						const auto iter = StringLists.find(prefix);
						if (iter == StringLists.end()) {
							std::cout << "Error: Invalid command line option '-" << option << "'.\n";
							return false;
						}
						iter->second.push_back(std::string(option.substr(1)));
					} else {
						std::cout << "Error: Invalid command line option '-" << option << "'.\n";
						return false;
					}
				}
			} else {
				if (!Path.empty()) {
					std::cout << "Error: Too many files.\n";
					return false;
				}
				Path = arg;
			}
		}

		return true;
	}
	bool ProgramOption::Verity() {
		for (auto& opt : Options) {
			if (!opt.second.Value) {
				opt.second.Value = opt.second.DefaultValue;
			}
		}
		for (auto& var : Variables) {
			if (!var.second.Value) {
				var.second.Value = var.second.DefaultValue;
			}
		}
		for (auto& flag : Flags) {
			if (!flag.second.Value) {
				flag.second.Value = flag.second.DefaultValue;
			}
		}

		if (GetOption("version")) {
			return true;
		} else if (Path.empty()) {
			std::cout << "Error: There is no input file.\n";
			return false;
		}

		if (GetVariable("stack") == 0) {
			std::cout << "Error: Size of the stack cannot be zero.\n";
			return false;
		} else if (GetVariable("stack") < 1 * 1024) {
			std::cout << "Warning: Size of the stack is too small.\n";
		}

		if (GetVariable("young") == 0) {
			std::cout << "Error: Size of a young block cannot be zero.\n";
			return false;
		} else if (GetVariable("young") % 512) {
			std::cout << "Error: Size of a young block must be a multiple of 512.\n";
			return false;
		} else if (GetVariable("old") == 0) {
			std::cout << "Error: Size of a old block cannot be zero.\n";
			return false;
		} else if (GetVariable("old") % 512) {
			std::cout << "Error: Size of a old block must be a multiple of 512.\n";
			return false;
		}

		const auto& linkDirs = GetStringList('L');
		for (const auto& linkDir : linkDirs) {
			const std::filesystem::path path(linkDir);
			if (!std::filesystem::exists(path)) {
				std::cout << "Error: \"" << linkDir << "\" does not exist.\n";
				return false;
			} else if (!std::filesystem::is_directory(path)) {
				std::cout << "Error: \"" << linkDir << "\" is not a directory.\n";
				return false;
			}
		}

		return true;
	}
}