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
		SVM_INLINE void InterpretPush(std::uint32_t operand);
		SVM_INLINE void InterpretPop();
		SVM_INLINE void InterpretLoad(std::uint32_t operand);
		SVM_INLINE void InterpretStore(std::uint32_t operand);

		SVM_INLINE void InterpretAdd();
		SVM_INLINE void InterpretSub();
		SVM_INLINE void InterpretMul();
		SVM_INLINE void InterpretIMul();
		SVM_INLINE void InterpretDiv();
		SVM_INLINE void InterpretIDiv();
		SVM_INLINE void InterpretMod();
		SVM_INLINE void InterpretIMod();
		SVM_INLINE void InterpretNeg();

		SVM_INLINE void InterpretAnd();
		SVM_INLINE void InterpretOr();
		SVM_INLINE void InterpretXor();
		SVM_INLINE void InterpretNot();
		SVM_INLINE void InterpretShl();
		SVM_INLINE void InterpretSal();
		SVM_INLINE void InterpretShr();
		SVM_INLINE void InterpretSar();

		SVM_INLINE void InterpretCmp();
		SVM_INLINE void InterpretICmp();
		SVM_INLINE void InterpretJmp(std::uint64_t& i, std::uint32_t operand);
		SVM_INLINE void InterpretJe(std::uint64_t& i, std::uint32_t operand);
		SVM_INLINE void InterpretJne(std::uint64_t& i, std::uint32_t operand);
		SVM_INLINE void InterpretJa(std::uint64_t& i, std::uint32_t operand);
		SVM_INLINE void InterpretJae(std::uint64_t& i, std::uint32_t operand);
		SVM_INLINE void InterpretJb(std::uint64_t& i, std::uint32_t operand);
		SVM_INLINE void InterpretJbe(std::uint64_t& i, std::uint32_t operand);
		SVM_INLINE void InterpretCall(std::uint64_t& i, std::uint32_t operand);
		SVM_INLINE void InterpretRet(std::uint64_t& i);

		SVM_INLINE void InterpretI2L();
		SVM_INLINE void InterpretI2D();
		SVM_INLINE void InterpretL2I();
		SVM_INLINE void InterpretL2D();
		SVM_INLINE void InterpretD2I();
		SVM_INLINE void InterpretD2L();
	};
}