#pragma once

#include <svm/Structure.hpp>
#include <svm/Type.hpp>

#include <cstdint>

namespace svm {
	class Object {
	private:
		Type m_Type;

	protected:
		Object() noexcept = default;
		Object(Type type) noexcept;
		Object(const Object& object) noexcept;

	public:
		~Object() = default;

	protected:
		Object& operator=(const Object& object) noexcept;

	public:
		bool operator==(const Object&) = delete;
		bool operator!=(const Object&) = delete;

	public:
		Type GetType() const noexcept;
	};

	class IntObject final : public Object {
	public:
		std::uint32_t Value = 0;

	public:
		IntObject() noexcept;
		IntObject(std::uint32_t value) noexcept;
		IntObject(const IntObject& object) noexcept;
		~IntObject() = default;

	public:
		IntObject& operator=(const IntObject& object) noexcept;
		bool operator==(const IntObject&) = delete;
		bool operator!=(const IntObject&) = delete;

	public:
		template<typename T>
		inline T Cast() const noexcept;
	};

	class LongObject final : public Object {
	public:
		std::uint64_t Value = 0;

	public:
		LongObject() noexcept;
		LongObject(std::uint64_t value) noexcept;
		LongObject(const LongObject& object) noexcept;
		~LongObject() = default;

	public:
		LongObject& operator=(const LongObject& object) noexcept;
		bool operator==(const LongObject&) = delete;
		bool operator!=(const LongObject&) = delete;

	public:
		template<typename T>
		inline T Cast() const noexcept;
	};

	class DoubleObject final : public Object {
	public:
		double Value = 0.0;

	public:
		DoubleObject() noexcept;
		DoubleObject(double value) noexcept;
		DoubleObject(const DoubleObject& object) noexcept;
		~DoubleObject() = default;

	public:
		DoubleObject& operator=(const DoubleObject& object) noexcept;
		bool operator==(const DoubleObject&) = delete;
		bool operator!=(const DoubleObject&) = delete;

	public:
		template<typename T>
		inline T Cast() const noexcept;
	};

	class PointerObject final : public Object {
	public:
		void* Value = nullptr;

	public:
		PointerObject() noexcept;
		PointerObject(void* value) noexcept;
		PointerObject(const PointerObject& object) noexcept;
		~PointerObject() = default;

	public:
		PointerObject& operator=(const PointerObject& object) noexcept;
		bool operator==(const PointerObject&) = delete;
		bool operator!=(const PointerObject&) = delete;

	public:
		template<typename T>
		inline T Cast() const noexcept;
	};

	class StructureObject final : public Object {
	public:
		StructureObject(Type type) noexcept;
		StructureObject(const StructureObject&) = delete;
		~StructureObject() = default;

	public:
		StructureObject& operator=(StructureObject) = delete;
		bool operator==(const StructureObject&) = delete;
		bool operator!=(const StructureObject&) = delete;
	};
}

#include "detail/impl/Object.hpp"