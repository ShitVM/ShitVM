#include <svm/Version.hpp>

namespace svm {
	const char* Version = SVM_VERSION;
	unsigned int VersionMajor = SVM_VERSION_MAJOR;
	unsigned int VersionMinor = SVM_VERSION_MINOR;
	unsigned int VersionPatch = SVM_VERSION_PATCH;
	const char* VersionIdentifier = SVM_VERSION_IDENTIFIER;
	const char* VersionTag = SVM_VERSION_TAG;
}