#pragma once

#include <svm/GarbageCollector.hpp>
#include <svm/Stack.hpp>

#include <cstddef>
#include <list>
#include <unordered_map>
#include <vector>

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
		std::size_t GetCurrnetBlockUsedSize() const noexcept;
		std::size_t GetCurrentBlockFreeSize() const noexcept;

		Block Begin() noexcept;
		Block End() noexcept;
		Block FindBlock(const void* address) noexcept;

		std::size_t GetDefaultBlockSize() const noexcept;
		std::size_t GetBlockCount() const noexcept;
	};
}

namespace svm {
	class SimpleGarbageCollector final : public GarbageCollector {
	private:
		using PointerTable = std::unordered_map<Stack*, std::unordered_map<void*, std::vector<void**>>>;
		using PointerList = std::vector<void*>;

	private:
		ManagedHeapGeneration m_YoungGeneration;
		ManagedHeapGeneration m_OldGeneration;
		std::unordered_map<std::uintptr_t, std::uint8_t> m_CardTable;

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

		virtual void* Allocate(Interpreter& interpreter, std::size_t size) override;
		virtual void MakeDirty(const void* address) noexcept override;

	private:
		void* AllocateOnYoungGeneration(Interpreter& interpreter, std::size_t size);
		void* AllocateOnOldGeneration(Interpreter& interpreter, PointerTable* minorPointerTable, std::size_t size);

		void MajorGC(Interpreter& interpreter, PointerTable* minorPointerTable);
		void MinorGC(Interpreter& interpreter);

		void MarkGCRoot(Interpreter& interpreter, ManagedHeapGeneration* generation, PointerTable& pointerTable);
		void MarkGCObject(Interpreter& interpreter, ManagedHeapGeneration* targetGeneration,
			ManagedHeapGeneration* generation, PointerTable& pointerTable);
		std::size_t MarkGCObject(Interpreter& interpreter, ManagedHeapGeneration* generation, PointerTable& pointerTable, ManagedHeapInfo* info);
		void CheckCardTable(Interpreter& interpreter, ManagedHeapGeneration* generation, PointerTable& pointerTable);
		void UpdateCardTable(const Interpreter& interpreter, const PointerList& promoted);
		void UpdateCardTable(void* oldAddress, void* newAddress);
		void UpdateMinorPointerTable(PointerTable& minorPointerTable, void* oldAddress, void* newAddress);
		void MoveSurvived(ManagedHeapGeneration& generation, ManagedHeapGeneration::Block firstBlock, const PointerTable& pointerTable);

		bool IsDirty(const void* address) const noexcept;
	};
}