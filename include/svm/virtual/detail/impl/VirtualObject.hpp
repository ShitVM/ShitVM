#pragma once
#include <svm/virtual/VirtualObject.hpp>

namespace svm {
	template<typename T>
	decltype(std::declval<T>().Value) VirtualObject::GetValue() const noexcept {
		return static_cast<T*>(GetObjectPtr())->Value;
	}
}