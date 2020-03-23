#pragma once
#include <svm/virtual/VirtualObject.hpp>

#include <svm/detail/Overload.hpp>

namespace svm {
	template<typename T, typename F>
	T VirtualObject::Native(F&& function) const noexcept {
		if (std::holds_alternative<std::monostate>(m_Object)) return 0;
		else return std::visit(function, std::visit(detail::Overload{
			[](std::monostate) noexcept -> ObjectVariant {
				return IntObject();
			},
			[](const auto& object) noexcept -> ObjectVariant {
				return GetObject(object);
			}
		}, m_Object));
	}
	template<typename T>
	T VirtualObject::NativeArithmetic() const noexcept {
		return Native<T>(detail::Overload{
			[](const PointerObject& pointer) noexcept {
				return static_cast<T>(reinterpret_cast<std::uintptr_t>(pointer.Value));
			},
			[](const GCPointerObject& pointer) noexcept {
				return static_cast<T>(reinterpret_cast<std::uintptr_t>(pointer.Value));
			},
			[](const auto& object) noexcept {
				return static_cast<T>(object.Value);
			}
		});
	}
}