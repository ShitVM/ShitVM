#include <svm/gc/SimpleGarbageCollector.hpp>

#include <svm/Interpreter.hpp>
#include <svm/Object.hpp>
#include <svm/Structure.hpp>
#include <svm/Type.hpp>

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <cstring>
#include <iterator>
#include <utility>

namespace {
	using Any = std::uint8_t;
}

namespace svm {
	ManagedHeapGeneration::ManagedHeapGeneration(std::size_t defaultBlockSize) {
		Initialize(defaultBlockSize);
	}
	ManagedHeapGeneration::ManagedHeapGeneration(ManagedHeapGeneration&& generation) noexcept
		: m_Blocks(std::move(generation.m_Blocks)), m_CurrentBlock(generation.m_CurrentBlock), m_DefaultBlockSize(generation.m_DefaultBlockSize) {}

	ManagedHeapGeneration& ManagedHeapGeneration::operator=(ManagedHeapGeneration&& generation) noexcept {
		m_Blocks = std::move(generation.m_Blocks);
		m_CurrentBlock = generation.m_CurrentBlock;
		m_DefaultBlockSize = generation.m_DefaultBlockSize;
		return *this;
	}

	void ManagedHeapGeneration::Reset() noexcept {
		m_Blocks.clear();
	}
	void ManagedHeapGeneration::Initialize(std::size_t defaultBlockSize) {
		assert(!IsInitalized());

		m_Blocks.emplace_back(defaultBlockSize);
		m_CurrentBlock = m_Blocks.begin();
		m_DefaultBlockSize = defaultBlockSize;
	}
	bool ManagedHeapGeneration::IsInitalized() const noexcept {
		return !m_Blocks.empty();
	}

	void* ManagedHeapGeneration::Allocate(std::size_t size) noexcept {
		if (!m_CurrentBlock->Expand(size)) return nullptr;
		else return m_CurrentBlock->GetTop<Any>();
	}

	void* ManagedHeapGeneration::CreateNewBlock(std::size_t size) {
		try {
			Stack newBlock(std::max(size, m_DefaultBlockSize));
			newBlock.SetUsedSize(size);

			const auto result = m_Blocks.insert(std::next(m_CurrentBlock), std::move(newBlock))->GetTop<Any>();
			return ++m_CurrentBlock, result;
		} catch (...) {
			return nullptr;
		}
	}
	ManagedHeapGeneration::Block ManagedHeapGeneration::GetEmptyBlock() {
		Block iter = std::next(m_CurrentBlock);
		if (iter == m_Blocks.end()) {
			iter = m_Blocks.begin();
		}

		if (iter->GetUsedSize() != 0) return m_Blocks.insert(iter, Stack(m_DefaultBlockSize));
		else return iter;
	}
	void ManagedHeapGeneration::DeleteEmptyBlocks() {
		std::vector<Block> blocks;
		for (auto iter = m_Blocks.begin(); iter != m_Blocks.end(); ++iter) {
			if (iter->GetUsedSize() == 0) {
				blocks.push_back(iter);
			}
		}

		for (std::size_t i = 8; i < blocks.size(); ++i) {
			m_Blocks.erase(blocks[i]);
		}
	}

	ManagedHeapGeneration::Block ManagedHeapGeneration::GetCurrentBlock() noexcept {
		return m_CurrentBlock;
	}
	void ManagedHeapGeneration::SetCurrentBlock(Block newCurrentBlock) noexcept {
		m_CurrentBlock = newCurrentBlock;
	}
	std::size_t ManagedHeapGeneration::GetCurrentBlockSize() const noexcept {
		return m_CurrentBlock->GetSize();
	}
	std::size_t ManagedHeapGeneration::GetCurrnetBlockUsedSize() const noexcept {
		return m_CurrentBlock->GetUsedSize();
	}
	std::size_t ManagedHeapGeneration::GetCurrentBlockFreeSize() const noexcept {
		return m_CurrentBlock->GetFreeSize();
	}

	ManagedHeapGeneration::Block ManagedHeapGeneration::Begin() noexcept {
		return m_Blocks.begin();
	}
	ManagedHeapGeneration::Block ManagedHeapGeneration::End() noexcept {
		return m_Blocks.end();
	}
	ManagedHeapGeneration::Block ManagedHeapGeneration::FindBlock(const void* address) noexcept {
		const Any* ptr = static_cast<const Any*>(address);

		for (Block iter = m_Blocks.begin(); iter != m_Blocks.end(); ++iter) {
			const Any* begin = iter->Begin();
			const Any* last = iter->Last();
			if (begin <= ptr && ptr <= last) return iter;
		}

		return m_Blocks.end();
	}

	std::size_t ManagedHeapGeneration::GetDefaultBlockSize() const noexcept {
		return m_DefaultBlockSize;
	}
	std::size_t ManagedHeapGeneration::GetBlockCount() const noexcept {
		return m_Blocks.size();
	}
}

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
		const int bit = static_cast<int>((addressInt - byte) / 8);

		std::uint8_t& card = m_CardTable[byte];
		card |= (1 << bit);
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
		if (size > m_OldGeneration.GetDefaultBlockSize()) {
			void* const address = m_OldGeneration.CreateNewBlock(size);
			return address;
		}

		if (size > m_OldGeneration.GetCurrentBlockFreeSize()) {
			MajorGC(interpreter, minorPointerTable);
		}

		void* address = m_OldGeneration.Allocate(size);
		if (!address) {
			address = m_OldGeneration.CreateNewBlock(size);
		}
		return address;
	}

	namespace {
		ManagedHeapGeneration::Block Next(ManagedHeapGeneration& generation, ManagedHeapGeneration::Block iterator) noexcept {
			const auto result = std::next(iterator);
			if (result == generation.End()) return generation.Begin();
			else return result;
		}
		ManagedHeapGeneration::Block Prev(ManagedHeapGeneration& generation, ManagedHeapGeneration::Block iterator) noexcept {
			if (iterator == generation.Begin()) return std::prev(generation.End());
			else return std::prev(iterator);
		}
	}

	void SimpleGarbageCollector::MajorGC(Interpreter& interpreter, PointerTable* minorPointerTable) {
		PointerTable pointerTable;

		// Mark
		MarkGCRoot(interpreter, &m_OldGeneration, pointerTable);
		MarkGCObject(interpreter, &m_YoungGeneration, &m_OldGeneration, pointerTable);
		MarkGCObject(interpreter, &m_OldGeneration, &m_OldGeneration, pointerTable);

		// Sweep
		auto emptyBlock = m_OldGeneration.GetEmptyBlock();
		auto currentBlock = Prev(m_OldGeneration, emptyBlock);
		const auto firstBlock = emptyBlock;

		do {
			while (currentBlock->GetUsedSize()) {
				ManagedHeapInfo* const info = currentBlock->GetTop<ManagedHeapInfo>();
				if (info->Age >> 7 == 0) {
					currentBlock->Reduce(info->Size);
					continue;
				}

				info->Age &= 0b00111111;

				void* newAddress = nullptr;
				if (!emptyBlock->Expand(info->Size)) {
					(emptyBlock = Prev(m_YoungGeneration, emptyBlock))->Expand(info->Size);
				}
				newAddress = emptyBlock->GetTop<Any>();

				auto& pointers = pointerTable[&*currentBlock][info];
				for (const auto pointer : pointers) {
					*pointer = newAddress;
				}

				pointers.push_back(static_cast<void**>(newAddress));
				currentBlock->Reduce(info->Size);

				if (minorPointerTable) {
					UpdateMinorPointerTable(*minorPointerTable, info, newAddress);
				}
				UpdateCardTable(info, newAddress);
			}

			currentBlock = Prev(m_OldGeneration, currentBlock);
		} while (firstBlock != currentBlock);

		m_OldGeneration.SetCurrentBlock(emptyBlock);
		MoveSurvived(m_OldGeneration, firstBlock, pointerTable);
		m_OldGeneration.DeleteEmptyBlocks();
	}
	void SimpleGarbageCollector::MinorGC(Interpreter& interpreter) {
		PointerTable pointerTable;
		PointerList promoted;

		// Mark
		MarkGCRoot(interpreter, &m_YoungGeneration, pointerTable);
		CheckCardTable(interpreter, &m_YoungGeneration, pointerTable);
		MarkGCObject(interpreter, &m_YoungGeneration, &m_YoungGeneration, pointerTable);

		// Sweep
		auto emptyBlock = m_YoungGeneration.GetEmptyBlock();
		auto currentBlock = Prev(m_YoungGeneration, emptyBlock);
		const auto firstBlock = emptyBlock;

		do {
			while (currentBlock->GetUsedSize()) {
				ManagedHeapInfo* const info = currentBlock->GetTop<ManagedHeapInfo>();
				if (info->Age >> 7 == 0) {
					currentBlock->Reduce(info->Size);
					continue;
				}

				info->Age &= 0b00111111;
				info->Age += 1;

				void* newAddress = nullptr;
				if (info->Age == 32) {
					newAddress = AllocateOnOldGeneration(interpreter, &pointerTable, info->Size);
					if (reinterpret_cast<Type*>(info + 1)->IsStructure()) {
						promoted.push_back(newAddress);
					}
				} else {
					if (!emptyBlock->Expand(info->Size)) {
						(emptyBlock = Prev(m_YoungGeneration, emptyBlock))->Expand(info->Size);
					}
					newAddress = emptyBlock->GetTop<Any>();
				}

				auto& pointers = pointerTable[&*currentBlock][info];
				for (const auto pointer : pointers) {
					*pointer = newAddress;
				}

				pointers.push_back(static_cast<void**>(newAddress));
				currentBlock->Reduce(info->Size);
			}

			currentBlock = Prev(m_YoungGeneration, currentBlock);
		} while (firstBlock != currentBlock);

		m_YoungGeneration.SetCurrentBlock(emptyBlock);
		MoveSurvived(m_YoungGeneration, firstBlock, pointerTable);
		UpdateCardTable(interpreter, promoted);
		m_OldGeneration.DeleteEmptyBlocks();
	}

	void SimpleGarbageCollector::MarkGCRoot(Interpreter& interpreter, ManagedHeapGeneration* generation, PointerTable& pointerTable) {
		const std::uint32_t varCount = interpreter.GetLocalVariableCount();
		for (std::uint32_t i = 0; i < varCount; ++i) {
			Type* const varPtr = interpreter.GetLocalVariable(i);
			if (*varPtr != GCPointerType) continue;

			GCPointerObject* const var = reinterpret_cast<GCPointerObject*>(varPtr);
			const auto block = generation->FindBlock(var->Value);
			if (block == generation->End()) continue;

			ManagedHeapInfo* const info = static_cast<ManagedHeapInfo*>(var->Value);
			info->Age |= 1 << 7;
			pointerTable[&*block][info].push_back(&var->Value);
		}
	}
	void SimpleGarbageCollector::MarkGCObject(Interpreter& interpreter, ManagedHeapGeneration* targetGeneration,
		ManagedHeapGeneration* generation, PointerTable& pointerTable) {
		for (auto block = targetGeneration->Begin(); block != targetGeneration->End(); ++block) {
			std::size_t offset = block->GetUsedSize();
			while (offset) {
				ManagedHeapInfo* const info = block->Get<ManagedHeapInfo>(offset);
				offset -= MarkGCObject(interpreter, generation, pointerTable, info) + sizeof(ManagedHeapInfo);
			}
		}
	}
	std::size_t SimpleGarbageCollector::MarkGCObject(Interpreter& interpreter, ManagedHeapGeneration* generation, PointerTable& pointerTable, ManagedHeapInfo* info) {
		Type* const typePtr = reinterpret_cast<Type*>(info + 1);
		if (info->Age >> 7 == 0 || (info->Age >> 6 & 0b1) == 1 || !typePtr->IsStructure()) return typePtr->GetReference().Size;

		const std::uint32_t structCode = static_cast<std::uint32_t>(typePtr->GetReference().Code) - 10;
		const Structure structure = interpreter.GetByteFile().GetStructures()[structCode];
		const std::uint32_t fieldCount = static_cast<std::uint32_t>(structure->FieldTypes.size());

		for (std::uint32_t i = 0; i < fieldCount; ++i) {
			Type* const fieldPtr = reinterpret_cast<Type*>(reinterpret_cast<std::uint8_t*>(typePtr) + structure->FieldOffsets[i]);
			if (*fieldPtr != GCPointerType) continue;

			GCPointerObject* const field = reinterpret_cast<GCPointerObject*>(fieldPtr);
			ManagedHeapInfo* const targetInfo = static_cast<ManagedHeapInfo*>(field->Value);
			if (!targetInfo) continue;

			const auto block = generation->FindBlock(targetInfo);
			if (block == generation->End()) continue;

			targetInfo->Age |= 1 << 7;
			MarkGCObject(interpreter, generation, pointerTable, targetInfo);
			pointerTable[&*block][targetInfo].push_back(&field->Value);
		}

		info->Age |= 1 << 6;

		return typePtr->GetReference().Size;
	}
	void SimpleGarbageCollector::CheckCardTable(Interpreter& interpreter, ManagedHeapGeneration* generation, PointerTable& pointerTable) {
		const Structures& structures = interpreter.GetByteFile().GetStructures();

		for (auto block = m_OldGeneration.Begin(); block != m_OldGeneration.End(); ++block) {
			std::size_t offset = block->GetUsedSize();
			while (offset) {
				ManagedHeapInfo* const info = block->Get<ManagedHeapInfo>(offset);
				if (!IsDirty(info)) {
					offset -= info->Size;
					continue;
				}

				Type* const typePtr = reinterpret_cast<Type*>(info + 1);
				if (!typePtr->IsStructure()) continue;

				const std::uint32_t structCode = static_cast<std::uint32_t>(typePtr->GetReference().Code) - 10;
				const Structure structure = structures[structCode];
				const std::uint32_t fieldCount = static_cast<std::uint32_t>(structure->FieldTypes.size());

				for (std::uint32_t i = 0; i < fieldCount; ++i) {
					Type* const fieldPtr = reinterpret_cast<Type*>(reinterpret_cast<std::uint8_t*>(typePtr) + structure->FieldOffsets[i]);
					if (*fieldPtr != GCPointerType) continue;

					GCPointerObject* const field = reinterpret_cast<GCPointerObject*>(fieldPtr);
					ManagedHeapInfo* const targetInfo = static_cast<ManagedHeapInfo*>(field->Value);
					if (!targetInfo) continue;

					const auto targetBlock = generation->FindBlock(targetInfo);
					if (targetBlock == generation->End()) continue;

					targetInfo->Age |= 1 << 7;
					pointerTable[&*block][targetInfo].push_back(&field->Value);
				}

				offset -= info->Size;
			}
		}
	}
	void SimpleGarbageCollector::UpdateCardTable(const Interpreter& interpreter, const PointerList& promoted) {
		const Structures& structures = interpreter.GetByteFile().GetStructures();

		for (const auto address : promoted) {
			const ManagedHeapInfo* info = static_cast<const ManagedHeapInfo*>(address);
			const Type* typePtr = reinterpret_cast<const Type*>(info + 1);

			const std::uint32_t structCode = static_cast<std::uint32_t>(typePtr->GetReference().Code) - 10;
			const Structure structure = structures[structCode];
			const std::uint32_t fieldCount = static_cast<std::uint32_t>(structure->FieldTypes.size());

			for (std::uint32_t i = 0; i < fieldCount; ++i) {
				const Type* fieldPtr = reinterpret_cast<const Type*>(reinterpret_cast<const std::uint8_t*>(typePtr) + structure->FieldOffsets[i]);
				if (*fieldPtr != GCPointerType) continue;

				const void* target = reinterpret_cast<const GCPointerObject*>(fieldPtr)->Value;
				if (!target || m_YoungGeneration.FindBlock(target) == m_YoungGeneration.End()) continue;

				MakeDirty(address);
				break;
			}
		}
	}
	void SimpleGarbageCollector::UpdateCardTable(void* oldAddress, void* newAddress) {
		const std::uintptr_t oldAddressInt = reinterpret_cast<std::uintptr_t>(oldAddress);
		const std::uintptr_t oldByte = oldAddressInt / 512;
		const int oldBit = static_cast<int>((oldAddressInt - oldByte) / 8);
		const auto oldIter = m_CardTable.find(oldByte);
		if (oldIter != m_CardTable.end()) {
			const bool isDirty = oldIter->second >> oldBit & 0b1;
			if (isDirty) {
				MakeDirty(newAddress);
			} else if (oldIter->second == 0) {
				m_CardTable.erase(oldIter);
			}
		}
	}
	void SimpleGarbageCollector::UpdateMinorPointerTable(PointerTable& minorPointerTable, void* oldAddress, void* newAddress) {
		ManagedHeapInfo* const info = static_cast<ManagedHeapInfo*>(oldAddress);
		void* const begin = info + 1;
		void* const end = static_cast<std::uint8_t*>(oldAddress) + info->Size;
		const std::uintptr_t distance = static_cast<std::uint8_t*>(newAddress) - static_cast<std::uint8_t*>(oldAddress);

		for (auto& [block, table] : minorPointerTable) {
			for (auto& [young, pointers] : table) {
				for (auto& pointer : pointers) {
					if (begin <= pointer && pointer < end) {
						pointer = reinterpret_cast<void**>(reinterpret_cast<std::uint8_t*>(pointer) + distance);
					}
				}
			}
		}
	}
	void SimpleGarbageCollector::MoveSurvived(ManagedHeapGeneration& generation, ManagedHeapGeneration::Block firstBlock, const PointerTable& pointerTable) {
		auto currentBlock = Prev(generation, firstBlock);
		do {
			const auto iter = pointerTable.find(&*currentBlock);
			if (iter != pointerTable.end()) {
				const auto& table = iter->second;
				for (const auto& [from, to] : table) {
					std::memcpy(to.back(), from, static_cast<ManagedHeapInfo*>(from)->Size);
				}
			}
			currentBlock = Prev(generation, currentBlock);
		} while (firstBlock != currentBlock);
	}

	bool SimpleGarbageCollector::IsDirty(const void* address) const noexcept {
		const std::uintptr_t addressInt = reinterpret_cast<std::uintptr_t>(address);
		const std::uintptr_t byte = addressInt / 512;
		const int bit = static_cast<int>((addressInt - byte) / 8);
		const auto iter = m_CardTable.find(byte);

		if (iter == m_CardTable.end()) return false;
		else return iter->second >> bit & 0b1;
	}
}