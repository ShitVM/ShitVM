#pragma once

#include <svm/GarbageCollector.hpp>
#include <svm/Stack.hpp>

#include <cstddef>
#include <cstdint>
#include <unordered_map>
#include <vector>

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

		void MarkGCRoots(Interpreter& interpreter, ManagedHeapGeneration* generation, PointerTable& pointerTable, PointerList& grayColorList);
		void MarkGCObjects(Interpreter& interpreter, ManagedHeapGeneration* generation, PointerTable& pointerTable, PointerList& grayColorList);
		void MarkGCObject(Interpreter& interpreter, ManagedHeapGeneration* generation, PointerTable& pointerTable, PointerList& grayColorList, ManagedHeapInfo* info);
		void MarkObject(ManagedHeapGeneration* generation, PointerTable& pointerTable, PointerList& grayColorList, Type* typePtr);
		void MakeGray(PointerTable& pointerTable, PointerList& grayColorList, void** variable, ManagedHeapGeneration::Block block, ManagedHeapInfo* info);

		void CheckCardTable(Interpreter& interpreter, PointerTable& pointerTable, PointerList& grayColorList);
		void UpdateCardTable(const Interpreter& interpreter, const PointerList& promoted);
		bool IsDirty(const void* address) const noexcept;
		void UpdateCardTable(void* oldAddress, void* newAddress);

		ManagedHeapGeneration::Block Sweep(Interpreter& interpreter, ManagedHeapGeneration* generation, PointerTable& pointerTable, PointerList* promoted);
		void MoveSurvived(ManagedHeapGeneration* generation, ManagedHeapGeneration::Block firstBlock, const PointerTable& pointerTable);
		void UpdateTables(const PointerTable& pointerTable, PointerTable* minorPointerTable);
		void UpdateMinorPointerTable(PointerTable* minorPointerTable, const void* oldAddress, const void* newAddress);
	};
}