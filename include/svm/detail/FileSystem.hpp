#pragma once

#if __has_include(<filesystem>)
#	include <filesystem>
#elif __has_include(<experimental/filesystem>)
#	include <experimental/filesystem>
#else
#	error
#endif

namespace svm::detail {
	namespace fs =
#if __has_include(<filesystem>)
		std::filesystem
#elif __has_include(<experimental/filesystem>)
		std::experimental::filesystem
#endif
		;
}