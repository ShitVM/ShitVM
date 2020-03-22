#pragma once

namespace svm {
#define SVM_VER_STRING "0.4.0"
#define SVM_VER_MAJOR 0
#define SVM_VER_MINOR 4
#define SVM_VER_PATCH 0
#define SVM_VER_IDENTIFIER ""
#define SVM_VER_TAG ""

	const char* GetShitVMVersionString() noexcept;
	int GetShitVMVersionMajor() noexcept;
	int GetShitVMVersionMinor() noexcept;
	int GetShitVMVersionPatch() noexcept;
	const char* GetShitVMVersionIdentifier() noexcept;
	const char* GetShitVMVersionTag() noexcept;
}