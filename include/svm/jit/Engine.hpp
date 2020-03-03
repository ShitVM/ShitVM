#pragma once
#include <svm/Macro.hpp>

#ifdef SVM_JIT

#include <svm/Function.hpp>

#include <unordered_map>

namespace svm {
	class Instructions;
}

namespace svm::jit {
	class Engine final {
	private:
		std::unordered_map<const Instructions*, Function> m_Functions;

	public:
		Engine() = default;
		Engine(Engine&& engine) noexcept;
		~Engine() = default;

	public:
		Engine& operator=(Engine&& engine) noexcept;
		bool operator==(const Engine&) = delete;
		bool operator!=(const Engine&) = delete;
		const Function& operator[](const Instructions* instructions) const noexcept;
		Function& operator[](const Instructions* instructions) noexcept;

	public:
		void Clear() noexcept;
		bool IsEmpty() const noexcept;
	};
}

#endif