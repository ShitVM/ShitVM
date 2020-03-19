#include <svm/gc/SimpleGarbageCollector.hpp>

#include <svm/Interpreter.hpp>
#include <svm/Object.hpp>
#include <svm/Structure.hpp>
#include <svm/Type.hpp>

#include <cassert>
#include <cstring>
#include <utility>

#include <iostream> // For developing

namespace svm {
	SimpleGarbageCollector::SimpleGarbageCollector(std::size_t youngGenerationSize, std::size_t oldGenerationSize) {
		Initialize(youngGenerationSize, oldGenerationSize);
	}
	SimpleGarbageCollector::SimpleGarbageCollector(SimpleGarbageCollector&& gc) noexcept
		: m_YoungGeneration(std::move(gc.m_YoungGeneration)), m_OldGeneration(std::move(gc.m_OldGeneration)), m_CardTable(std::move(gc.m_CardTable)) {}
	SimpleGarbageCollector::~SimpleGarbageCollector() {
		Reset();
	}

	SimpleGarbageCollector& SimpleGarbageCollector::operator=(SimpleGarbageCollector&& gc) noexcept {
		Reset();

		m_YoungGeneration = std::move(gc.m_YoungGeneration);
		m_OldGeneration = std::move(gc.m_OldGeneration);
		m_CardTable = std::move(gc.m_CardTable);
		return *this;
	}

	void SimpleGarbageCollector::Reset() noexcept {
		m_YoungGeneration.Reset();
		m_OldGeneration.Reset();
		m_CardTable.clear();
	}
	void SimpleGarbageCollector::Initialize(std::size_t youngGenerationSize, std::size_t oldGenerationSize) {
		assert(!IsInitialized());
		assert(youngGenerationSize % 512 == 0);
		assert(oldGenerationSize % 512 == 0);

		m_YoungGeneration.Initialize(youngGenerationSize);
		m_OldGeneration.Initialize(oldGenerationSize);
	}
	bool SimpleGarbageCollector::IsInitialized() const noexcept {
		return !m_YoungGeneration.IsInitalized() && m_YoungGeneration.IsInitalized();
	}

	void* SimpleGarbageCollector::Allocate(Interpreter& interpreter, std::size_t size) {
		size += sizeof(ManagedHeapInfo);

		ManagedHeapInfo* address = nullptr;
		if (size > m_YoungGeneration.GetDefaultBlockSize()) {
			address = static_cast<ManagedHeapInfo*>(AllocateOnOldGeneration(interpreter, nullptr, size));
		} else {
			address = static_cast<ManagedHeapInfo*>(AllocateOnYoungGeneration(interpreter, size));
		}

		std::memset(address + 1, 0, size - sizeof(ManagedHeapInfo));
		address->Size = size;
		address->Age = 0;
		return address;
	}
	void SimpleGarbageCollector::MakeDirty(const void* address) noexcept {
		const std::uintptr_t addressInt = reinterpret_cast<std::uintptr_t>(address);
		const std::uintptr_t byte = addressInt / 512;
		const int bit = static_cast<int>(addressInt % 512 / 64);

		std::uint8_t& card = m_CardTable[byte];
		card |= 1 << bit;
	}

	void* SimpleGarbageCollector::AllocateOnYoungGeneration(Interpreter& interpreter, std::size_t size) {
		if (size > m_YoungGeneration.GetCurrentBlockFreeSize()) {
			MinorGC(interpreter);
		}

		void* address = m_YoungGeneration.Allocate(size);
		if (!address) {
			address = m_YoungGeneration.CreateNewBlock(size);
		}
		return address;
	}
	void* SimpleGarbageCollector::AllocateOnOldGeneration(Interpreter& interpreter, PointerTable* minorPointerTable, std::size_t size) {
		if (size > m_OldGeneration.GetDefaultBlockSize()) return m_OldGeneration.CreateNewBlock(size);
		else if (size > m_OldGeneration.GetCurrentBlockFreeSize()) {
			MajorGC(interpreter, minorPointerTable);
		}

		void* address = m_OldGeneration.Allocate(size);
		if (!address) {
			address = m_OldGeneration.CreateNewBlock(size);
		}
		return address;
	}

	void SimpleGarbageCollector::MajorGC(Interpreter& interpreter, PointerTable* minorPointerTable) {
		std::cout << "MajorGC\n";

		PointerTable pointerTable;
		PointerList grayColorList;

		// Mark
		MarkGCRoots(interpreter, &m_OldGeneration, pointerTable, grayColorList);
		// TODO: Mark young
		MarkGCObjects(interpreter, &m_OldGeneration, pointerTable, grayColorList);

		// Sweep
		const auto firstBlock = Sweep(interpreter, &m_OldGeneration, pointerTable, nullptr);
		MoveSurvived(&m_OldGeneration, firstBlock, pointerTable);
		UpdateTables(pointerTable, minorPointerTable);
		m_OldGeneration.DeleteEmptyBlocks();
	}
	void SimpleGarbageCollector::MinorGC(Interpreter& interpreter) {
		std::cout << "MinorGC\n";

		PointerTable pointerTable;
		PointerList grayColorList;
		PointerList promoted;

		// Mark
		MarkGCRoots(interpreter, &m_YoungGeneration, pointerTable, grayColorList);
		CheckCardTable(interpreter, pointerTable, grayColorList);
		MarkGCObjects(interpreter, &m_YoungGeneration, pointerTable, grayColorList);

		// Sweep
		const auto firstBlock = Sweep(interpreter, &m_YoungGeneration, pointerTable, &promoted);
		MoveSurvived(&m_YoungGeneration, firstBlock, pointerTable);
		UpdateCardTable(interpreter, promoted);
		m_YoungGeneration.DeleteEmptyBlocks();
	}

	void SimpleGarbageCollector::MarkGCRoots(Interpreter& interpreter, ManagedHeapGeneration* generation, PointerTable& pointerTable, PointerList& grayColorList) {
		const std::uint32_t varCount = interpreter.GetLocalVariableCount();
		for (std::uint32_t i = 0; i < varCount; ++i) {
			MarkObject(generation, pointerTable, grayColorList, interpreter.GetLocalVariable(i));
		}
	}
	void SimpleGarbageCollector::MarkGCObjects(Interpreter& interpreter, ManagedHeapGeneration* generation, PointerTable& pointerTable, PointerList& grayColorList) {
		while (grayColorList.size()) {
			ManagedHeapInfo* const info = static_cast<ManagedHeapInfo*>(grayColorList.back());
			grayColorList.pop_back();

			MarkGCObject(interpreter, generation, pointerTable, grayColorList, info);
		}
	}
	void SimpleGarbageCollector::MarkGCObject(Interpreter& interpreter, ManagedHeapGeneration* generation,
		PointerTable& pointerTable, PointerList& grayColorList, ManagedHeapInfo* info) {
		Type* const typePtr = reinterpret_cast<Type*>(info + 1);
		if (!typePtr->IsStructure()) return;

		const std::uint32_t structCode = static_cast<std::uint32_t>(typePtr->GetReference().Code) - 10;
		const Structure structure = interpreter.GetByteFile().GetStructures()[structCode];
		const std::uint32_t fieldCount = static_cast<std::uint32_t>(structure->FieldTypes.size());

		for (std::uint32_t i = 0; i < fieldCount; ++i) {
			Type* const fieldPtr = reinterpret_cast<Type*>(reinterpret_cast<std::uint8_t*>(typePtr) + structure->FieldOffsets[i]);
			MarkObject(generation, pointerTable, grayColorList, fieldPtr);
		}
	}
	void SimpleGarbageCollector::MarkObject(ManagedHeapGeneration* generation, PointerTable& pointerTable, PointerList& grayColorList, Type* typePtr) {
		if (*typePtr != GCPointerType) return;

		GCPointerObject* const object = reinterpret_cast<GCPointerObject*>(typePtr);
		ManagedHeapInfo* const info = static_cast<ManagedHeapInfo*>(object->Value);
		const auto block = generation->FindBlock(object->Value);
		if (block == generation->End()) return;

		MakeGray(pointerTable, grayColorList, &object->Value, block, info);
	}
	void SimpleGarbageCollector::MakeGray(PointerTable& pointerTable, PointerList& grayColorList,
		void** variable, ManagedHeapGeneration::Block block, ManagedHeapInfo* info) {
		pointerTable[&*block][info].push_back(variable);
		if (info->Age >> 7 == 0) {
			info->Age |= 1 << 7;
			grayColorList.push_back(info);
		}
	}

	void SimpleGarbageCollector::CheckCardTable(Interpreter& interpreter, PointerTable& pointerTable, PointerList& grayColorList) {
		const Structures& structures = interpreter.GetByteFile().GetStructures();

		for (auto block = m_OldGeneration.Begin(); block != m_OldGeneration.End(); ++block) {
			std::size_t offset = block->GetUsedSize();
			while (offset) {
				ManagedHeapInfo* const info = block->Get<ManagedHeapInfo>(offset);
				offset -= info->Size;
				if (!IsDirty(info)) continue;

				Type* const typePtr = reinterpret_cast<Type*>(info + 1);
				if (!typePtr->IsStructure()) continue;

				const std::uint32_t structCode = static_cast<std::uint32_t>(typePtr->GetReference().Code) - 10;
				const Structure structure = structures[structCode];
				const std::uint32_t fieldCount = static_cast<std::uint32_t>(structure->FieldTypes.size());

				for (std::uint32_t i = 0; i < fieldCount; ++i) {
					Type* const fieldPtr = reinterpret_cast<Type*>(reinterpret_cast<std::uint8_t*>(typePtr) + structure->FieldOffsets[i]);
					MarkObject(&m_YoungGeneration, pointerTable, grayColorList, fieldPtr);
				}
			}
		}
	}
	void SimpleGarbageCollector::UpdateCardTable(const Interpreter& interpreter, const PointerList& promoted) {
		const Structures& structures = interpreter.GetByteFile().GetStructures();

		for (const auto address : promoted) {
			const ManagedHeapInfo* const info = static_cast<const ManagedHeapInfo*>(address);
			const Type* const typePtr = reinterpret_cast<const Type*>(info + 1);

			const std::uint32_t structCode = static_cast<std::uint32_t>(typePtr->GetReference().Code) - 10;
			const Structure structure = structures[structCode];
			const std::uint32_t fieldCount = static_cast<std::uint32_t>(structure->FieldTypes.size());

			for (std::uint32_t i = 0; i < fieldCount; ++i) {
				const Type* const fieldPtr = reinterpret_cast<const Type*>(reinterpret_cast<const std::uint8_t*>(typePtr) + structure->FieldOffsets[i]);
				if (*fieldPtr != GCPointerType) continue;

				const void* target = reinterpret_cast<const GCPointerObject*>(fieldPtr)->Value;
				if (m_YoungGeneration.FindBlock(target) == m_YoungGeneration.End()) continue;

				MakeDirty(info);
				break;
			}
		}
	}
	bool SimpleGarbageCollector::IsDirty(const void* address) const noexcept {
		const std::uintptr_t addressInt = reinterpret_cast<std::uintptr_t>(address);
		const std::uintptr_t byte = addressInt / 512;
		const int bit = static_cast<int>(addressInt % 512 / 64);
		const auto iter = m_CardTable.find(byte);

		if (iter == m_CardTable.end()) return false;
		else return iter->second >> bit & 0b1;
	}
	void SimpleGarbageCollector::UpdateCardTable(void* oldAddress, void* newAddress) {
		const std::uintptr_t oldAddressInt = reinterpret_cast<std::uintptr_t>(oldAddress);
		const std::uintptr_t oldByte = oldAddressInt / 512;
		const int oldBit = static_cast<int>(oldAddressInt % 512 / 64);
		const auto oldIter = m_CardTable.find(oldByte);
		if (oldIter != m_CardTable.end()) {
			const bool isDirty = oldIter->second >> oldBit & 0b1;
			if (isDirty) {
				MakeDirty(newAddress);
			}
		}
	}

	ManagedHeapGeneration::Block SimpleGarbageCollector::Sweep(Interpreter& interpreter, ManagedHeapGeneration* generation, PointerTable& pointerTable, PointerList* promoted) {
		auto emptyBlock = generation->GetEmptyBlock();
		auto currentBlock = generation->Prev(emptyBlock);
		const auto firstBlock = emptyBlock;

		do {
			while (currentBlock->GetUsedSize()) {
				ManagedHeapInfo* const info = currentBlock->GetTop<ManagedHeapInfo>();
				currentBlock->Reduce(info->Size);
				if (info->Age >> 7 == 0) continue;

				info->Age &= 0b00111111;
				info->Age += 1;

				void* newAddress = nullptr;
				if (info->Age == 4 && generation == &m_YoungGeneration) {
					newAddress = AllocateOnOldGeneration(interpreter, &pointerTable, info->Size);
					if (reinterpret_cast<Type*>(info + 1)->IsStructure()) {
						promoted->push_back(newAddress);
					}
					info->Age = 0;
				} else {
					if (!emptyBlock->Expand(info->Size)) {
						(emptyBlock = generation->Prev(emptyBlock))->Expand(info->Size);
					}
					newAddress = emptyBlock->GetTop<std::uint8_t>();
				}

				auto& pointers = pointerTable[&*currentBlock][info];
				for (const auto pointer : pointers) {
					*pointer = newAddress;
				}

				pointers.push_back(static_cast<void**>(newAddress));
			}

			currentBlock = generation->Prev(currentBlock);
		} while (currentBlock != firstBlock);

		generation->SetCurrentBlock(emptyBlock);
		return firstBlock;
	}
	void SimpleGarbageCollector::MoveSurvived(ManagedHeapGeneration* generation, ManagedHeapGeneration::Block firstBlock, const PointerTable& pointerTable) {
		auto currentBlock = generation->Prev(firstBlock);
		do {
			const auto iter = pointerTable.find(&*currentBlock);
			if (iter != pointerTable.end()) {
				const auto& table = iter->second;
				for (const auto& [from, to] : table) {
					std::memcpy(to.back(), from, static_cast<ManagedHeapInfo*>(from)->Size);
				}
			}

			currentBlock = generation->Prev(currentBlock);
		} while (currentBlock != firstBlock);
	}
	void SimpleGarbageCollector::UpdateTables(const PointerTable& pointerTable, PointerTable* minorPointerTable) {
		for (const auto& [block, table] : pointerTable) {
			for (const auto& [from, to] : table) {
				UpdateCardTable(from, to.back());
				UpdateMinorPointerTable(minorPointerTable, from, to.back());
			}
		}

		for (auto iter = m_CardTable.begin(); iter != m_CardTable.end();) {
			if ((iter++)->second == 0) {
				m_CardTable.erase(std::prev(iter));
			}
		}
	}
	void SimpleGarbageCollector::UpdateMinorPointerTable(PointerTable* minorPointerTable, const void* oldAddress, const void* newAddress) {
		const ManagedHeapInfo* const info = static_cast<const ManagedHeapInfo*>(oldAddress);
		const void* const begin = info + 1;
		const void* const end = static_cast<const std::uint8_t*>(oldAddress) + info->Size;
		const std::ptrdiff_t distance = static_cast<const std::uint8_t*>(newAddress) - static_cast<const std::uint8_t*>(oldAddress);

		for (auto& [block, table] : *minorPointerTable) {
			for (auto& [young, pointers] : table) {
				for (auto& pointer : pointers) {
					if (begin <= pointer && pointer < end) {
						pointer = reinterpret_cast<void**>(reinterpret_cast<std::uint8_t*>(pointer) + distance);
					}
				}
			}
		}
	}
}