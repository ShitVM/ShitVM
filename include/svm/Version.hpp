#pragma once

#define SVM_VERSION "0.5.0"
#define SVM_VERSION_MAJOR 0
#define SVM_VERSION_MINOR 5
#define SVM_VERSION_PATCH 0
#define SVM_VERSION_IDENTIFIER ""
#define SVM_VERSION_TAG ""

namespace svm {
	extern const char* Version;
	extern unsigned int VersionMajor;
	extern unsigned int VersionMinor;
	extern unsigned int VersionPatch;
	extern const char* VersionIdentifier;
	extern const char* VersionTag;
}