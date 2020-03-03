#include <svm/jit/Engine.hpp>

#ifdef SVM_JIT

#include <svm/Instruction.hpp>

#include <utility>

namespace svm::jit {
	Engine::Engine(Engine&& engine) noexcept
		: m_Functions(std::move(engine.m_Functions)) {}

	Engine& Engine::operator=(Engine&& engine) noexcept {
		m_Functions = std::move(engine.m_Functions);
		return *this;
	}
	const Function& Engine::operator[](const Instructions* instructions) const noexcept {
		return m_Functions.at(instructions);
	}
	Function& Engine::operator[](const Instructions* instructions) noexcept {
		return m_Functions[instructions];
	}

	void Engine::Clear() noexcept {
		m_Functions.clear();
	}
	bool Engine::IsEmpty() const noexcept {
		return m_Functions.empty();
	}
}

#endif