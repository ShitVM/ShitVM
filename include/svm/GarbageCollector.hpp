#pragma once

#include <svm/Stack.hpp>

#include <cstddef>
#include <cstdint>
#include <list>

namespace svm {
	struct ManagedHeapInfo final {
		std::size_t Size = 0;
		std::uint8_t Age = 0;

		explicit ManagedHeapInfo(std::size_t size) noexcept;
		ManagedHeapInfo(const ManagedHeapInfo& info) noexcept;
		~ManagedHeapInfo() = default;

		ManagedHeapInfo& operator=(const ManagedHeapInfo& info) noexcept;
		bool operator==(const ManagedHeapInfo&) = delete;
		bool operator!=(const ManagedHeapInfo&) = delete;
	};
}

namespace svm {
	class ManagedHeapGeneration final {
	public:
		using Block = std::list<Stack>::iterator;

	private:
		std::list<Stack> m_Blocks;
		Block m_CurrentBlock;
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

		void* Allocate(std::size_t size) noexcept;

		void* CreateNewBlock(std::size_t size);
		Block GetEmptyBlock();
		void DeleteEmptyBlocks();

		Block GetCurrentBlock() noexcept;
		void SetCurrentBlock(Block newCurrentBlock) noexcept;
		std::size_t GetCurrentBlockSize() const noexcept;
		std::size_t GetCurrentBlockUsedSize() const noexcept;
		std::size_t GetCurrentBlockFreeSize() const noexcept;

		Block Begin() noexcept;
		Block End() noexcept;
		Block Prev(Block iterator) noexcept;
		Block Next(Block iterator) noexcept;
		Block FindBlock(const void* address) noexcept;

		std::size_t GetDefaultBlockSize() const noexcept;
		std::size_t GetBlockCount() const noexcept;
	};
}

namespace svm {
	class Interpreter;

	class GarbageCollector {
	protected:
		GarbageCollector() noexcept = default;
		GarbageCollector(const GarbageCollector&) = delete;

	public:
		virtual ~GarbageCollector() = default;

	public:
		GarbageCollector& operator=(const GarbageCollector&) = delete;
		bool operator==(const GarbageCollector&) = delete;
		bool operator!=(const GarbageCollector&) = delete;

	public:
		virtual void* Allocate(Interpreter& interpreter, std::size_t size) = 0;
		virtual void MakeDirty(const void* address) noexcept = 0;
	};
}