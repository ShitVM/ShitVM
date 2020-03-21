#include <svm/Version.hpp>

namespace svm {
	const char* GetShitVMVersionString() noexcept {
		return SVM_VER_STRING;
	}
	int GetShitVMVersionMajor() noexcept {
		return SVM_VER_MAJOR;
	}
	int GetShitVMVersionMinor() noexcept {
		return SVM_VER_MINOR;
	}
	int GetShitVMVersionPatch() noexcept {
		return SVM_VER_PATCH;
	}
	const char* GetShitVMVersionIdentifier() noexcept {
		return SVM_VER_IDENTIFIER;
	}
	const char* GetShitVMVersionTag() noexcept {
		return SVM_VER_TAG;
	}
}