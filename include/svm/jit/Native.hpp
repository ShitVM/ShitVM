#pragma once

#include <svm/Macro.hpp>
#include <svm/jit/x86.hpp>

namespace svm::jit {
#ifdef SVM_X86
	namespace native = x86;
#endif
}