#include <svm/jit/Engine.hpp>

#ifdef SVM_JIT

#include <svm/Instruction.hpp>

#include <memory>
#include <new>
#include <utility>

#if defined(SVM_WINDOWS)
#	include <Windows.h>
#elif defined(SVM_POSIX)
#	include <sys/mman.h>
#endif

namespace svm::jit {
	Engine::Engine(Engine&& engine) noexcept
		: m_Memory(std::move(engine.m_Memory)) {}

	Engine& Engine::operator=(Engine&& engine) noexcept {
		m_Memory = std::move(engine.m_Memory);
		return *this;
	}
	void(*Engine::operator[](const Instructions* inst) const)() {
		return static_cast<void(*)()>(m_Memory.at(inst));
	}

	void Engine::Clear() noexcept {
		for (const auto& mem : m_Memory) {
#if defined(SVM_WINDOWS)
			VirtualFree(mem.second, 0, MEM_RELEASE);
#elif defined(SVM_POSIX)
			munmap(mem.second.first, func.second.second);
#endif
		}
		m_Memory.clear();
	}
	bool Engine::IsEmpty() const noexcept {
		return m_Memory.empty();
	}

	void Engine::Allocate(const Instructions* inst, const std::vector<std::uint8_t>& instructions) {
#if defined(SVM_WINDOWS)
		struct Deleter final {
			void operator()(void* mem) noexcept {
				VirtualFree(mem, 0, MEM_RELEASE);
			}
		};

		std::unique_ptr<void, Deleter> mem(VirtualAlloc(nullptr, instructions.size(), MEM_COMMIT, PAGE_READWRITE));
		if (mem == nullptr) throw std::bad_alloc();
		std::copy(instructions.begin(), instructions.end(), static_cast<std::uint8_t*>(mem.get()));

		DWORD dummy;
		if (!VirtualProtect(mem.get(), instructions.size(), PAGE_EXECUTE_READ, &dummy)) throw std::bad_alloc();

		m_Memory[inst] = mem.get();
		mem.release();
#elif defined(SVM_POSIX)
		void* mem = mmap(0, instructions.size(), PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
		if (mem == nullptr) throw std::bad_alloc();
		std::copy(instructions.begin(), instructions.end(), static_cast<std::uint8_t*>(mem));

		struct RaiiStruct final {
			std::size_t Size;
			void* Memory;

			RaiiStruct(std::size_t size, void* memory) noexcept
				: Size(size), Memory(memory) {}
			~RaiiStruct() {
				if (Memory) {
					munmap(memory, size);
				}
			}
		} raii(instructions.size(), mem);
		if (mprotect(mem, instructions.size(), PROT_READ | PROT_EXEC) == -1) throw std::bad_alloc();

		m_Memory[inst] = std::make_pair(mem, instructions.size());
		raii.Memory = nullptr;
#endif
	}
}

#endif