#include <svm/Interpreter.hpp>

namespace svm {
	Stack::Stack() noexcept {}
	Stack::Stack(std::size_t size) {
		Allocate(size);
	}
	Stack::Stack(Stack&& stack) noexcept
		: m_Data(std::move(stack.m_Data)), m_Size(stack.m_Size), m_Used(stack.m_Used) {}

	Stack& Stack::operator=(Stack&& stack) noexcept {
		m_Data = std::move(stack.m_Data);
		m_Size = stack.m_Size;
		m_Used = stack.m_Used;
		return *this;
	}

	void Stack::Allocate(std::size_t size) {
		m_Data = std::unique_ptr<std::uint8_t[]>(new std::uint8_t[size]());
		m_Size = size;
		m_Used = 0;
	}
	void Stack::Resize(std::size_t newSize) {
		if (m_Used > newSize) throw std::runtime_error("Failed to resize the stack. Smaller size than the used size.");

		std::unique_ptr<std::uint8_t[]> newData(new std::uint8_t[newSize]());
		std::copy(m_Data.get(), m_Data.get() + m_Used, newData.get());

		m_Data = std::move(newData);
		m_Size = newSize;
	}
	void Stack::Deallocate() noexcept {
		m_Data.reset();
		m_Size = 0;
		m_Used = 0;
	}

	std::size_t Stack::GetSize() const noexcept {
		return m_Size;
	}
	std::size_t Stack::GetUsedSize() const noexcept {
		return m_Used;
	}
}

namespace svm {
	Interpreter::Interpreter(ByteFile&& byteFile) noexcept
		: m_ByteFile(std::move(byteFile)) {}
	Interpreter::Interpreter(Interpreter&& interpreter) noexcept
		: m_ByteFile(std::move(interpreter.m_ByteFile)),
		m_Stack(std::move(interpreter.m_Stack)) {}

	Interpreter& Interpreter::operator=(Interpreter&& interpreter) noexcept {
		m_ByteFile = std::move(interpreter.m_ByteFile);

		m_Stack = std::move(interpreter.m_Stack);
		return *this;
	}

	void Interpreter::Clear() noexcept {
		m_ByteFile.Clear();
		m_Stack.Deallocate();
	}
	void Interpreter::Load(ByteFile&& byteFile) noexcept {
		m_ByteFile = std::move(byteFile);
	}
	void Interpreter::AllocateStack(std::size_t size) {
		m_Stack.Allocate(size);
	}
	void Interpreter::ReallocateStack(std::size_t newSize) {
		m_Stack.Resize(newSize);
	}
}