#include <svm/jit/Function.hpp>

#ifdef SVM_JIT

#include <cassert>
#include <cstdint>

#if defined(SVM_WINDOWS)
#	include <Windows.h>
#elif defined(SVM_POSIX)
#	include <sys/mman.h>
#endif

namespace svm::jit {
	void Function::operator()() const {
		assert(m_Address != nullptr);

		static_cast<void(*)()>(m_Address)();
	}

	void Function::Reset() noexcept {
		if (m_Address) {
#if defined(SVM_WINDOWS)
			VirtualFree(m_Address, 0, MEM_RELEASE);
#elif defined(SVM_POSIX)
			munmap(m_Address, m_Size);
#endif
		}
	}
	bool Function::IsEmpty() const noexcept {
		return m_Address == nullptr;
	}

	void Function::Allocate(const std::vector<std::uint8_t>& codes) {
		assert(m_Address == nullptr);

#if defined(SVM_WINDOWS)
		LPVOID mem = VirtualAlloc(nullptr, codes.size(), MEM_COMMIT, PAGE_READWRITE);
		if (mem == nullptr) throw std::bad_alloc();
		std::copy(codes.begin(), codes.end(), static_cast<std::uint8_t*>(mem));

		DWORD dummy;
		if (!VirtualProtect(mem, codes.size(), PAGE_EXECUTE_READ, &dummy)) {
			VirtualFree(mem, 0, MEM_RELEASE);
			throw std::bad_alloc();
		}

		m_Address = mem;
		m_Size = codes.size();
#elif defined(SVM_POSIX)
		void* mem = mmap(nullptr, codes.size(), PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
		if (mem == nullptr) throw std::bad_alloc();
		std::copy(codes.begin(), codes.end(), static_cast<std::uint8_t*>(mem));

		if (mprotect(mem, codes.size(), PROT_READ | PROT_EXEC) == -1) {
			munmap(m_Address, codes.size());
			throw std::bad_alloc();
		}

		m_Address = mem;
		m_Size = codes.size();
#endif
	}
}

#endif