#include <svm/ProgramOption.hpp>

#include <iostream>
#include <string_view>
#include <utility>

namespace svm {
	ProgramOption::ProgramOption(ProgramOption&& option) noexcept
		: Path(std::move(option.Path)), Variables(std::move(option.Variables)), Flags(std::move(option.Flags)) {}

	ProgramOption& ProgramOption::operator=(ProgramOption&& option) noexcept {
		Path = std::move(option.Path);

		Variables = std::move(option.Variables);
		Flags = std::move(option.Flags);
		return *this;
	}

	void ProgramOption::Clear() noexcept {
		Path.clear();

		Variables.clear();
		Flags.clear();
	}

	ProgramOption& ProgramOption::AddVariable(const char* name, std::uint64_t defaultValue) {
		Variables[name].DefaultValue = defaultValue;
		return *this;
	}
	ProgramOption& ProgramOption::AddFlag(const char* name, bool defaultValue) {
		Flags[name].DefaultValue = defaultValue;
		return *this;
	}
	std::uint64_t ProgramOption::GetVariable(const char* name) const {
		return *Variables.at(name).Value;
	}
	bool ProgramOption::GetFlag(const char* name) const {
		return *Flags.at(name).Value;
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
				if (option.compare(0, 4, "fno-") == 0) {
					const std::string_view flag = option.substr(4);
					const auto iter = Flags.find(flag);
					if (iter == Flags.end()) {
						std::cout << "Error: Unknown flag '" << flag << "'.\n";
						return false;
					}
					iter->second.Value = false;
				} else if (option.front() == 'f') {
					const std::string_view flag = option.substr(1);
					const auto iter = Flags.find(flag);
					if (iter == Flags.end()) {
						std::cout << "Error: Unknown flag '" << flag << "'.\n";
						return false;
					}
					iter->second.Value = true;
				} else {
					const std::size_t assign = option.find('=');
					if (assign == std::string_view::npos) {
						std::cout << "Error: Invalid command line option '-" << option << "'.\n";
						return false;
					}
					const std::string_view var = option.substr(0, assign);
					const std::string value(option.substr(assign + 1));
					const auto iter = Variables.find(var);
					if (iter == Variables.end()) {
						std::cout << "Error: Unknown variable '" << var << "'.\n";
						return false;
					}
					iter->second.Value = static_cast<std::uint64_t>(std::stoull(value));
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
		if (Path.empty()) {
			std::cout << "Error: There is no input file.\n";
			return false;
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

		if (GetVariable("stack") == 0) {
			std::cout << "Error: Stack's size cannot be zero.\n";
			return false;
		} else if (GetVariable("stack") < 1 * 1024) {
			std::cout << "Warning: Stack's size is too small.\n";
		}

		if (GetVariable("young") == 0) {
			std::cout << "Error: Young geneartion's block size cannot be zero.\n";
			return false;
		} else if (GetVariable("young") % 512) {
			std::cout << "Error: Young geneartion's block size must be a multiple of 512.\n";
			return false;
		} else if (GetVariable("old") == 0) {
			std::cout << "Error: Old geneartion's block size cannot be zero.\n";
			return false;
		} else if (GetVariable("old") % 512) {
			std::cout << "Error: Old geneartion's block size must be a multiple of 512.\n";
			return false;
		}

		return true;
	}
}