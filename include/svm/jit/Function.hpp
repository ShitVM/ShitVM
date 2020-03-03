#pragma once
#include <svm/Macro.hpp>

#ifdef SVM_JIT

#include <cstddef>
#include <cstdint>
#include <vector>

namespace svm::jit {
	class Function final {
	private:
		void* m_Address = nullptr;
		std::size_t m_Size = 0;

	public:
		Function() noexcept = default;
		Function(Function&& function) noexcept;
		~Function() = default;

	public:
		Function& operator=(Function&& function) noexcept;
		bool operator==(const Function&) = delete;
		bool operator!=(const Function&) = delete;
		void operator()() const;

	public:
		void Reset() noexcept;
		bool IsEmpty() const noexcept;
		
		void Allocate(const std::vector<std::uint8_t>& codes);
	};
}

#endif