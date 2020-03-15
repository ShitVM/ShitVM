#pragma once

#include <svm/GarbageCollector.hpp>
#include <svm/Stack.hpp>

#include <cstddef>
#include <list>

namespace svm {
	class ManagedHeapGeneration final {
	private:
		std::list<Stack> m_Blocks;
		std::list<Stack>::iterator m_CurrentBlock;
		std::size_t m_DefaultBlockSize = 0;

	public:
		ManagedHeapGeneration() = default;
		explicit ManagedHeapGeneration(std::size_t defaultBlockSize);
		ManagedHeapGeneration(ManagedHeapGeneration&& generation) noexcept;
		~ManagedHeapGeneration() = default;

	public:
		ManagedHeapGeneration& operator=(ManagedHeapGeneration&& generation) noexcept;
		bool operator==(const ManagedHeapGeneration&) = delete;
		bool operator!=(const ManagedHeapGeneration&) = delete;

	public:
		void Reset() noexcept;
		void Initialize(std::size_t defaultBlockSize);
		bool IsInitalized() const noexcept;
	};
}

namespace svm {
	class SimpleGarbageCollector final : public GarbageCollector {
	private:
		ManagedHeapGeneration m_YoungGeneration;
		ManagedHeapGeneration m_OldGeneration;

	public:
		SimpleGarbageCollector() = default;
		SimpleGarbageCollector(std::size_t youngGenerationSize, std::size_t oldGenerationSize);
		SimpleGarbageCollector(SimpleGarbageCollector&& gc) noexcept;
		~SimpleGarbageCollector();

	public:
		SimpleGarbageCollector& operator=(SimpleGarbageCollector&& gc) noexcept;
		bool operator==(const SimpleGarbageCollector&) = delete;
		bool operator!=(const SimpleGarbageCollector&) = delete;

	public:
		void Reset() noexcept;
		void Initialize(std::size_t youngGenerationSize, std::size_t oldGenerationSize);
		bool IsInitialized() const noexcept;

		virtual void* Allocate(std::size_t size) override;
	};
}