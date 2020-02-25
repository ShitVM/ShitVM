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

#if defined(_MSC_VER)
#	define SVM_MSVC
#elif defined(__GNUC__)
#	define SVM_GCC
#elif defined(__clang__)
#	define SVM_CLANG
#endif

#if defined(SVM_MSVC)
#	define SVM_INLINE __forceinline
#elif defined(SVM_GCC) || defined(SVM_CLANG)
#	define SVM_INLINE __attribute__((always_inline))
#else
#	define SVM_INLINE inline
#endif

#if defined(SVM_MSVC) && defined(SVM_PROFILING)
#	define SVM_NOINLINE_FOR_PROFILING __declspec(noinline)
#else
#	define SVM_NOINLINE_FOR_PROFILING
#endif