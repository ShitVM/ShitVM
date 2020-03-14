#pragma once
#include <svm/Object.hpp>

namespace svm {
	template<>
	inline IntObject IntObject::Cast<IntObject>() const noexcept {
		return *this;
	}
	template<>
	inline LongObject IntObject::Cast<LongObject>() const noexcept {
		return Value;
	}
	template<>
	inline DoubleObject IntObject::Cast<DoubleObject>() const noexcept {
		return static_cast<double>(Value);
	}
	template<>
	inline PointerObject IntObject::Cast<PointerObject>() const noexcept {
		return reinterpret_cast<void*>(static_cast<std::uintptr_t>(Value));
	}
}

namespace svm {
	template<>
	inline IntObject LongObject::Cast<IntObject>() const noexcept {
		return static_cast<std::uint32_t>(Value);
	}
	template<>
	inline LongObject LongObject::Cast<LongObject>() const noexcept {
		return *this;
	}
	template<>
	inline DoubleObject LongObject::Cast<DoubleObject>() const noexcept {
		return static_cast<double>(Value);
	}
	template<>
	inline PointerObject LongObject::Cast<PointerObject>() const noexcept {
		return reinterpret_cast<void*>(static_cast<std::uintptr_t>(Value));
	}
}

namespace svm {
	template<>
	inline IntObject DoubleObject::Cast<IntObject>() const noexcept {
		return static_cast<std::uint32_t>(Value);
	}
	template<>
	inline LongObject DoubleObject::Cast<LongObject>() const noexcept {
		return static_cast<std::uint64_t>(Value);
	}
	template<>
	inline DoubleObject DoubleObject::Cast<DoubleObject>() const noexcept {
		return *this;
	}
	template<>
	inline PointerObject DoubleObject::Cast<PointerObject>() const noexcept {
		return reinterpret_cast<void*>(static_cast<std::uintptr_t>(Value));
	}
}

namespace svm {
	template<>
	inline IntObject PointerObject::Cast<IntObject>() const noexcept {
		return static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(Value));
	}
	template<>
	inline LongObject PointerObject::Cast<LongObject>() const noexcept {
		return static_cast<std::uint64_t>(reinterpret_cast<std::uintptr_t>(Value));
	}
	template<>
	inline DoubleObject PointerObject::Cast<DoubleObject>() const noexcept {
		return static_cast<double>(reinterpret_cast<std::uintptr_t>(Value));
	}
	template<>
	inline PointerObject PointerObject::Cast<PointerObject>() const noexcept {
		return *this;
	}
}