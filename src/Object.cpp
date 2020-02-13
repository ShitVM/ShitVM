#include <svm/Object.hpp>

namespace svm {
	Type Object::GetType() const noexcept {
		return Type::None;
	}
	bool Object::IsInt() const noexcept {
		return GetType() == Type::Int;
	}
	bool Object::IsLong() const noexcept {
		return GetType() == Type::Long;
	}
	bool Object::IsDouble() const noexcept {
		return GetType() == Type::Double;
	}
	bool Object::IsReference() const noexcept {
		return GetType() == Type::Reference;
	}
}