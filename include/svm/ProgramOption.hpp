#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <optional>
#include <unordered_map>

namespace svm {
	struct Variable final {
		std::optional<std::uint64_t> Value;
		std::uint64_t DefaultValue;
	};

	struct Flag final {
		std::optional<bool> Value;
		bool DefaultValue;
	};

	class ProgramOption final {
	public:
		std::string Path;

		std::unordered_map<std::string_view, Flag> Options;
		std::unordered_map<std::string_view, Variable> Variables;
		std::unordered_map<std::string_view, Flag> Flags;

	public:
		ProgramOption() = default;
		ProgramOption(ProgramOption&& option) noexcept;
		~ProgramOption() = default;

	public:
		ProgramOption& operator=(ProgramOption&& option) noexcept;
		bool operator==(const ProgramOption&) = delete;
		bool operator!=(const ProgramOption&) = delete;

	public:
		void Clear() noexcept;

		ProgramOption& AddOption(const char* name);
		ProgramOption& AddVariable(const char* name, std::uint64_t defaultValue);
		ProgramOption& AddFlag(const char* name, bool defaultValue);
		bool GetOption(const char* name) const;
		std::uint64_t GetVariable(const char* name) const;
		bool GetFlag(const char* name) const;

		bool Parse(int argc, char* argv[]);
		bool Verity();
	};
}