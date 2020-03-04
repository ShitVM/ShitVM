#pragma once

#include <svm/Type.hpp>

#include <cstdint>

namespace svm {
	class Object {
	private:
		Type m_Type;

	public:
		Object() noexcept = default;
		~Object() = default;

	protected:
		Object(Type type) noexcept;
		Object(const Object& object) noexcept;

	protected:
		Object& operator=(const Object& object) noexcept;

	public:
		Type GetType() const noexcept;
		bool IsInt() const noexcept;
		bool IsLong() const noexcept;
		bool IsDouble() const noexcept;
		bool IsPointer() const noexcept;
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
	};
}