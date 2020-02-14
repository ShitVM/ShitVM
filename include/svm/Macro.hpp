#pragma once

#ifdef _WIN32
#	define SVM_WINDOWS
#endif

#if defined(_M_IX86) || defined(__i386) || defined(_X86_) || defined(__X86__) || defined(__THW_INTEL__) || defined(__I86__) || defined(__INTEL__) || defined(__386)
#	define SVM_X86
#endif

#if defined(_M_X64) || defined(__amd64)
#	ifndef SVM_X86
#		define SVM_X86
#	endif
#	define SVM_X64
#endif

#if defined(SVM_WINDOWS) || defined(SVM_X86)
#	define SVM_LITTLE
#endif