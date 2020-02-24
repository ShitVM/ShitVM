#pragma once

#include <svm/ByteFile.hpp>
#include <svm/Macro.hpp>

#include <cstddef>
#include <cstdint>
#include <limits>
#include <utility>
#include <vector>

namespace svm {
	class Stack final {
	private:
		std::vector<std::uint8_t> m_Data;
		std::size_t m_Used = 0;

	public:
		Stack() noexcept = default;
		explicit Stack(std::size_t size);
		Stack(Stack&& stack) noexcept;
		~Stack() = default;

	public:
		Stack& operator=(Stack&& stack) noexcept;

	public:
		void Allocate(std::size_t size);
		void Reallocate(std::size_t newSize);
		void Deallocate() noexcept;

		template<typename T>
		void Push(const T& value) {
			if (GetFreeSize() < sizeof(T)) throw std::runtime_error("Failed to push the value to the stack. Stack overflow.");
			*reinterpret_cast<T*>(&*(m_Data.rbegin() + (m_Used += sizeof(T)) - 1)) = value;
		}
		template<typename T>
		T Pop() {
			T result = GetTop<T>();
			m_Used -= sizeof(T);
			return std::move(result);
		}
		template<typename T>
		T& Get(std::size_t offset) {
			if (m_Used < offset - sizeof(T)) throw std::runtime_error("Failed to get the value from the stack. No enough space.");
			return *reinterpret_cast<T*>(&*(m_Data.rbegin() + offset - 1));
		}
		template<typename T>
		T& GetTop() {
			return Get<T>(m_Used);
		}
		const Type* GetTopType();
		std::size_t GetSize() const noexcept;
		std::size_t GetUsedSize() const noexcept;
		std::size_t GetFreeSize() const noexcept;
		void RemoveTo(std::size_t newSize) noexcept;
	};

	struct StackFrame final {
		std::size_t StackBegin = 0;
		std::size_t VariableBegin = 0;
		std::size_t Caller = std::numeric_limits<std::size_t>::max();
		const svm::Function* Function = nullptr;
		const svm::Instructions* Instructions = nullptr;
	};

	class Interpreter final {
	private:
		ByteFile m_ByteFile;

		Stack m_Stack;
		StackFrame m_StackFrame;
		std::vector<std::size_t> m_LocalVariables;

	public:
		Interpreter() noexcept = default;
		explicit Interpreter(ByteFile&& byteFile) noexcept;
		Interpreter(Interpreter&& interpreter) noexcept;
		~Interpreter() = default;

	public:
		Interpreter& operator=(Interpreter&& interpreter) noexcept;
		bool operator==(const ByteFile&) = delete;
		bool operator!=(const ByteFile&) = delete;

	public:
		void Clear() noexcept;
		void Load(ByteFile&& byteFile) noexcept;
		void AllocateStack(std::size_t size = 1 * 1024 * 1024);
		void ReallocateStack(std::size_t newSize);

		void Interpret();
		template<typename T>
		const T& GetResult() {
			return m_Stack.GetTop<T>();
		}

	private:
		void InterpretPush(std::uint32_t operand);
		void InterpretPop();
		void InterpretLoad(std::uint32_t operand);
		void InterpretStore(std::uint32_t operand);

		void InterpretAdd();
		void InterpretSub();
		void InterpretMul();
		void InterpretIMul();
		void InterpretDiv();
		void InterpretIDiv();
		void InterpretMod();
		void InterpretIMod();
		void InterpretNeg();

		void InterpretAnd();
		void InterpretOr();
		void InterpretXor();
		void InterpretNot();
		void InterpretShl();
		void InterpretSal();
		void InterpretShr();
		void InterpretSar();

		void InterpretCmp();
		void InterpretICmp();
		void InterpretJmp(std::uint64_t& i, std::uint32_t operand);
		void InterpretJe(std::uint64_t& i, std::uint32_t operand);
		void InterpretJne(std::uint64_t& i, std::uint32_t operand);
		void InterpretJa(std::uint64_t& i, std::uint32_t operand);
		void InterpretJae(std::uint64_t& i, std::uint32_t operand);
		void InterpretJb(std::uint64_t& i, std::uint32_t operand);
		void InterpretJbe(std::uint64_t& i, std::uint32_t operand);
		SVM_INLINE void InterpretCall(std::uint64_t& i, std::uint32_t operand);
		SVM_INLINE void InterpretRet(std::uint64_t& i);

		void InterpretI2L();
		void InterpretI2D();
		void InterpretL2I();
		void InterpretL2D();
		void InterpretD2I();
		void InterpretD2L();
	};
}