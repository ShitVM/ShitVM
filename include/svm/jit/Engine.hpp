#pragma once
#include <svm/Macro.hpp>

#ifdef SVM_JIT

#include <unordered_map>
#include <vector>

namespace svm {
	class Instructions;
}

namespace svm::jit {
	class Engine final {
	private:
#if defined(SVM_WINDOWS)
		std::unordered_map<const Instructions*, void*> m_Memory;
#elif defined(SVM_POSIX)
		std::unordered_map<const Instructions*, std::pair<void*, std::size_t>> m_Memory;
#endif

	public:
		Engine() = default;
		Engine(Engine&& engine) noexcept;
		~Engine() = default;

	public:
		Engine& operator=(Engine&& engine) noexcept;
		bool operator==(const Engine&) = delete;
		bool operator!=(const Engine&) = delete;
		void(*operator[](const Instructions* inst) const)();

	public:
		void Clear() noexcept;
		bool IsEmpty() const noexcept;

		void Allocate(const Instructions* inst, const std::vector<std::uint8_t>& instructions);
	};
}

#endif