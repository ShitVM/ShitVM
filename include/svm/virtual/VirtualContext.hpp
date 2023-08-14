#pragma once

#include <svm/Structure.hpp>
#include <svm/virtual/VirtualModule.hpp>
#include <svm/virtual/VirtualObject.hpp>
#include <svm/virtual/VirtualStack.hpp>

#include <cstdint>
#include <optional>

namespace svm {
	class Interpreter;
	class Heap;

	class VirtualContext final {
	private:
		Interpreter& m_Interpreter;
		VirtualStack& m_Stack;
		Heap& m_Heap;

	public:
		VirtualContext(Interpreter& interpreter, VirtualStack& stack, Heap& heap) noexcept;
		VirtualContext(const VirtualContext&) = delete;
		~VirtualContext() = default;

	public:
		VirtualContext& operator=(const VirtualContext&) = delete;
		bool operator==(const VirtualContext&) = delete;
		bool operator!=(const VirtualContext&) = delete;

	public:
		Structure GetStructure(VirtualModule::StructureIndex structure);
		Structure GetStructure(VirtualModule::MappedStructureIndex structure);

		VirtualObject GetField(const VirtualObject& structure, std::uint32_t index);
		VirtualObject GetElement(const VirtualObject& array, std::uint64_t index);

		VirtualObject GetParameter(std::uint16_t index);

		VirtualObject PushFundamental(const Object& object, std::uint64_t count = 0);
		VirtualObject PushStructure(Structure structure, std::uint64_t count = 0);
		bool Pop();

		VirtualObject::PointerTarget GetPointer(const VirtualObject& object);
		VirtualObject GetObject(VirtualObject::PointerTarget pointer);
		VirtualObject GetObject(VirtualObject::GCPointerTarget pointer);

		VirtualObject NewFundamental(const Object& object, std::uint64_t count = 0);
		VirtualObject GCNewFundamental(const Object& object, std::uint64_t count = 0);
		VirtualObject NewStructure(Structure structure, std::uint64_t count = 0);
		VirtualObject GCNewStructure(Structure structure, std::uint64_t count = 0);
		void DeleteObject(const VirtualObject& object);

	private:
		void InitFundamental(void* target, const Object& object, const Type& type);
		void InitFundamental(void* target, const Type& type, std::uint64_t count);
		void InitStructure(void* target, Structure structure);
		void InitStructure(void* target, Structure structure, std::uint64_t count);
	};
}