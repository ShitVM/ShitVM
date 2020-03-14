#pragma once
#include <svm/Object.hpp>

namespace svm {
	template<>
	LongObject IntObject::Cast<LongObject>() const noexcept {
		return Value;
	}
	template<>
	DoubleObject IntObject::Cast<DoubleObject>() const noexcept {
		return static_cast<double>(Value);
	}
	template<>
	PointerObject IntObject::Cast<PointerObject>() const noexcept {
		return reinterpret_cast<void*>(static_cast<std::uintptr_t>(Value));
	}
}

namespace svm {
	template<>
	IntObject LongObject::Cast<IntObject>() const noexcept {
		return static_cast<std::uint32_t>(Value);
	}
	template<>
	DoubleObject LongObject::Cast<DoubleObject>() const noexcept {
		return static_cast<double>(Value);
	}
	template<>
	PointerObject LongObject::Cast<PointerObject>() const noexcept {
		return reinterpret_cast<void*>(static_cast<std::uintptr_t>(Value));
	}
}

namespace svm {
	template<>
	IntObject DoubleObject::Cast<IntObject>() const noexcept {
		return static_cast<std::uint32_t>(Value);
	}
	template<>
	LongObject DoubleObject::Cast<LongObject>() const noexcept {
		return static_cast<std::uint64_t>(Value);
	}
	template<>
	PointerObject DoubleObject::Cast<PointerObject>() const noexcept {
		return reinterpret_cast<void*>(static_cast<std::uintptr_t>(Value));
	}
}

namespace svm {
	template<>
	IntObject PointerObject::Cast<IntObject>() const noexcept {
		return static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(Value));
	}
	template<>
	LongObject PointerObject::Cast<LongObject>() const noexcept {
		return static_cast<std::uint64_t>(reinterpret_cast<std::uintptr_t>(Value));
	}
	template<>
	DoubleObject PointerObject::Cast<DoubleObject>() const noexcept {
		return static_cast<double>(reinterpret_cast<std::uintptr_t>(Value));
	}
}