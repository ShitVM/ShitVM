#pragma once

namespace svm {
	enum class Type {
		None,

		Int,
		Long,
		Double,
		Reference,
	};

	class Object {
	public:
		Object() noexcept = default;
		Object(const Object& object) noexcept = default;
		Object(Object&& object) noexcept = default;
		virtual ~Object() = default;

	public:
		Object& operator=(const Object& object) noexcept = default;
		Object& operator=(Object&& object) noexcept = default;
		
	public:
		virtual Type GetType() const noexcept;
		bool IsInt() const noexcept;
		bool IsLong() const noexcept;
		bool IsDouble() const noexcept;
		bool IsReference() const noexcept;
	};
}