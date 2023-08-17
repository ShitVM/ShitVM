#include <svm/Loader.hpp>

#include <svm/detail/InterpreterExceptionCode.hpp>
#include <svm/virtual/VirtualContext.hpp>
#include <svm/virtual/VirtualModule.hpp>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <iterator>
#include <list>
#include <utf8.h>
#include <utility>
#include <vector>

namespace svm {
	namespace detail {
		VirtualModule& LoaderAdapter::CreateWrapped(std::string virtualPath) {
			return static_cast<VirtualModule&>(Create(std::move(virtualPath)));
		}
	}

	VirtualModule& Loader::Create(std::string virtualPath) {
		return CreateWrapped(std::move(virtualPath));
	}
}

#define PREF(o) (context.GetPointer(o)) // Pointer reference
#define PDREF(p) (context.GetObject(p.ToPointer())) // Pointer Dereference
#define STRUCT(i) (context.GetStructure(i))
#define FIELD(s, i) (context.GetField(s, i))
#define ITEM(a, i) (context.GetElement(a, i)) // Array element
#define PARAM(i) (context.GetParameter(i))

namespace svm::detail::stdlib {
	VirtualObject Assert(const VirtualObject& object, Type type) {
		if (object.GetType() == type) return object;
		else throw SVM_IEC_STDLIB_TYPEASSERTFAIL;
	}
	void Assert(bool pass, int code) {
		if (!pass) throw code;
	}
}

#define ASSERT_BEGIN try
#define ASSERT_END catch (int code) { context.OccurException(code); return; }

#define PDREF_A(p, t) (Assert(PDREF(p), t))
#define PARAM_A(i, t) (Assert(PARAM(i), t))

namespace svm::detail::stdlib::array {
	struct State {
		void Init(Loader& loader, std::vector<VirtualModule*>& modules) {
			auto& module = loader.Create("/std/array.sbf");
			modules.push_back(&module);

			module.AddFunction("copy", 5, false, [this](VirtualContext& context) {
				ASSERT_BEGIN {
					const auto dest = PDREF_A(PARAM_A(0, PointerType), ArrayType);
					const auto destBegin = PARAM_A(1, LongType).ToLong();
					const auto src = PDREF_A(PARAM_A(2, PointerType), ArrayType);
					const auto srcBegin = PARAM_A(3, LongType).ToLong();
					const auto count = PARAM_A(4, LongType).ToLong();

					Assert(dest.IsArray() == src.IsArray(), SVM_IEC_STDLIB_TYPEASSERTFAIL);
					Assert(destBegin + count <= dest.GetCount(), SVM_IEC_STDLIB_ARRAY_OUTOFRANGE);
					Assert(srcBegin + count <= src.GetCount(), SVM_IEC_STDLIB_ARRAY_OUTOFRANGE);

					context.CopyObjectUnsafe(PREF(ITEM(dest, destBegin)), PREF(ITEM(src, srcBegin)), count);
				} ASSERT_END;
			});
		}
	};
}

namespace svm::detail::stdlib::string {
	constexpr std::uint32_t StringData = 0;
	constexpr std::uint32_t StringLength = 1;
	constexpr std::uint32_t StringCapacity = 2;

	void Expand32(VirtualContext& context, const VirtualObject& string, std::uint64_t required) {
		auto capacity = FIELD(string, StringCapacity).ToLong();
		if (required <= capacity) return;

		if (capacity == 0) {
			capacity = required;
		} else {
			do {
				capacity <<= 1; // Doubling
			} while (required > capacity);
		}

		const auto data = FIELD(string, StringData);
		const auto newData = context.NewFundamental(IntObject(), capacity);
		if (data.ToPointer() != VPNULL) {
			context.CopyObject(newData, PDREF(data));
			context.DeleteObject(data);
		}

		data.SetPointer(PREF(newData));
		FIELD(string, StringCapacity).SetLong(capacity);
	}

	std::u32string ConvertToCppString32(VirtualContext& context, const VirtualObject& string) {
		const auto length = static_cast<std::size_t>(FIELD(string, StringLength).ToLong());
		const auto data = PDREF(FIELD(string, StringData));

		std::u32string result(length, 0);
		for (std::size_t i = 0; i < length; ++i) {
			result[i] = ITEM(data, i).ToInt();
		}
		return result;
	}
	void ConvertFromCppString32(VirtualContext& context, const std::u32string& cppString, const VirtualObject& string) {
		const auto size = static_cast<std::uint64_t>(cppString.size());
		Expand32(context, string, size);
		FIELD(string, StringLength).SetLong(size);

		const auto data = PDREF(FIELD(string, StringData));
		for (std::size_t i = 0; i < static_cast<std::size_t>(size); ++i) {
			ITEM(data, i).SetInt(cppString[i]);
		}
	}

	struct State {
		VirtualModule::StructureIndex VirtualString32;

		void Init(Loader& loader, std::vector<VirtualModule*>& modules) {
			auto& module = loader.Create("/std/string.sbf");
			modules.push_back(&module);

			VirtualString32 = module.AddStructure("String32", {
				{ svm::PointerType, 0 }, // data
				{ svm::LongType, 0 }, // length
				{ svm::LongType, 0 }, // capacity
			});

			module.AddFunction("create32", 0, true, [this](VirtualContext& context) {
				const auto result = context.PushStructure(STRUCT(VirtualString32));
				FIELD(result, StringData).SetPointer(VPNULL);
				FIELD(result, StringLength).SetLong(0);
				FIELD(result, StringCapacity).SetLong(0);
			});

			module.AddFunction("push", 2, false, [this](VirtualContext& context) {
				ASSERT_BEGIN {
					const auto string = PDREF_A(PARAM_A(0, PointerType), STRUCT(VirtualString32)->Type);
					const auto stringLength = FIELD(string, StringLength).ToLong();
					const auto value = PARAM_A(1, IntType).ToInt();

					Expand32(context, string, stringLength + 1);
					ITEM(PDREF(FIELD(string, StringData)), stringLength).SetInt(value);
					FIELD(string, StringLength).SetLong(stringLength + 1);
				} ASSERT_END;
			});
			module.AddFunction("concat", 2, false, [this](VirtualContext& context) {
				ASSERT_BEGIN {
					const auto dest = PDREF_A(PARAM_A(0, PointerType), STRUCT(VirtualString32)->Type);
					const auto destLength = FIELD(dest, StringLength).ToLong();
					const auto src = PDREF_A(PARAM_A(1, PointerType), STRUCT(VirtualString32)->Type);
					const auto srcLength = FIELD(src, StringLength).ToLong();

					Expand32(context, dest, destLength + srcLength);
					context.CopyObjectUnsafe(
						PREF(ITEM(PDREF(FIELD(dest, StringData)), destLength)),
						PREF(ITEM(PDREF(FIELD(src, StringData)), 0)), srcLength);
					FIELD(dest, StringLength).SetLong(destLength + srcLength);
				} ASSERT_END;
			});

			module.AddFunction("destroy", 1, false, [this](VirtualContext& context) {
				ASSERT_BEGIN {
					const auto string = PDREF_A(PARAM_A(0, PointerType), STRUCT(VirtualString32)->Type);
					const auto data = FIELD(string, StringData);
					if (data.ToPointer() != VPNULL) {
						context.DeleteObject(data);
					}
				} ASSERT_END;
			});
		}
	};
}

namespace svm::detail::stdlib::io {
	constexpr std::uint32_t StreamHandle = 0;

	struct Stream {
		bool IsReadable = false;
		bool IsWriteable = false;

		virtual ~Stream() = default;

		virtual std::uint32_t ReadInt() = 0;
		virtual std::uint32_t ReadSignedInt() = 0;
		virtual std::uint64_t ReadLong() = 0;
		virtual std::uint64_t ReadSignedLong() = 0;
		virtual double ReadDouble() = 0;
		virtual std::uint32_t ReadChar32() = 0;
		virtual std::u32string ReadString32() = 0;

		virtual void WriteInt(std::uint32_t value) = 0;
		virtual void WriteSignedInt(std::uint32_t value) = 0;
		virtual void WriteLong(std::uint64_t value) = 0;
		virtual void WriteSignedLong(std::uint64_t value) = 0;
		virtual void WriteDouble(double value) = 0;
		virtual void WriteChar32(std::uint32_t value) = 0;
		virtual void WriteString32(const std::u32string& value) = 0;

	protected:
		template<typename T>
		static std::uint32_t ReadChar32(T& stream) {
			char utf8[4];
			stream >> utf8[0];

			if (static_cast<std::uint8_t>(utf8[0]) < 0x80) {
				return utf8[0];
			} else if (static_cast<std::uint8_t>(utf8[0]) >= 0xF0) {
				stream >> utf8[1] >> utf8[2] >> utf8[3];
				return ((utf8[0] & 0x07) << 18) + ((utf8[1] & 0x3F) << 12) + ((utf8[2] & 0x3F) << 6) + ((utf8[3] & 0x3F) << 0);
			} else if (static_cast<std::uint8_t>(utf8[0]) >= 0xE0) {
				stream >> utf8[1] >> utf8[2];
				return ((utf8[0] & 0x0F) << 12) + ((utf8[1] & 0x3F) << 6) + ((utf8[2] & 0x3F) << 0);
			} else {
				stream >> utf8[1];
				return ((utf8[0] & 0x1F) << 6) + ((utf8[1] & 0x3F) << 0);
			}
		}
		template<typename T>
		static std::u32string ReadString32(T& stream) {
			std::string utf8;
			stream >> utf8;

			std::u32string utf32;
			utf8::utf8to32(utf8.begin(), utf8.end(), std::back_inserter(utf32));
			return utf32;
		}

		template<typename T>
		static void WriteChar32(T& stream, std::uint32_t value) {
			const char32_t utf32[2] = { value, 0 };
			char utf8[5] = {};
			utf8::utf32to8(std::begin(utf32), std::end(utf32), std::begin(utf8));
			stream << utf8;
		}
		template<typename T>
		static void WriteString32(T& stream, const std::u32string& value) {
			std::string utf8;
			utf8::utf32to8(value.begin(), value.end(), std::back_inserter(utf8));
			stream << utf8;
		}
	};

	struct ReadonlyStream : Stream {
		ReadonlyStream() {
			IsReadable = true;
		}

		virtual void WriteInt(std::uint32_t) override {
			throw std::bad_function_call();
		}
		virtual void WriteSignedInt(std::uint32_t) override {
			throw std::bad_function_call();
		}
		virtual void WriteLong(std::uint64_t) override {
			throw std::bad_function_call();
		}
		virtual void WriteSignedLong(std::uint64_t) override {
			throw std::bad_function_call();
		}
		virtual void WriteDouble(double) override {
			throw std::bad_function_call();
		}
		virtual void WriteChar32(std::uint32_t) override {
			throw std::bad_function_call();
		}
		virtual void WriteString32(const std::u32string&) override {
			throw std::bad_function_call();
		}
	};

	struct WriteonlyStream : Stream {
		WriteonlyStream() {
			IsWriteable = true;
		}

		virtual std::uint32_t ReadInt() override {
			throw std::bad_function_call();
		}
		virtual std::uint32_t ReadSignedInt() override {
			throw std::bad_function_call();
		}
		virtual std::uint64_t ReadLong() override {
			throw std::bad_function_call();
		}
		virtual std::uint64_t ReadSignedLong() override {
			throw std::bad_function_call();
		}
		virtual double ReadDouble() override {
			throw std::bad_function_call();
		}
		virtual std::uint32_t ReadChar32() override {
			throw std::bad_function_call();
		}
		virtual std::u32string ReadString32() override {
			throw std::bad_function_call();
		}
	};

	struct StdinStream : ReadonlyStream {
		virtual std::uint32_t ReadInt() override {
			std::uint32_t value;
			std::cin >> value;
			return value;
		}
		virtual std::uint32_t ReadSignedInt() override {
			std::int32_t value;
			std::cin >> value;
			return value;
		}
		virtual std::uint64_t ReadLong() override {
			std::uint64_t value;
			std::cin >> value;
			return value;
		}
		virtual std::uint64_t ReadSignedLong() override {
			std::int64_t value;
			std::cin >> value;
			return value;
		}
		virtual double ReadDouble() override {
			double value;
			std::cin >> value;
			return value;
		}
		virtual std::uint32_t ReadChar32() override {
			return Stream::ReadChar32(std::cin);
		}
		virtual std::u32string ReadString32() override {
			return Stream::ReadString32(std::cin);
		}
	};

	struct StdoutStream : WriteonlyStream {
		virtual void WriteInt(std::uint32_t value) override {
			std::cout << value;
		}
		virtual void WriteSignedInt(std::uint32_t value) override {
			std::cout << static_cast<std::int32_t>(value);
		}
		virtual void WriteLong(std::uint64_t value) override {
			std::cout << value;
		}
		virtual void WriteSignedLong(std::uint64_t value) override {
			std::cout << static_cast<std::int64_t>(value);
		}
		virtual void WriteDouble(double value) override {
			std::cout << value;
		}
		virtual void WriteChar32(std::uint32_t value) override {
			Stream::WriteChar32(std::cout, value);
		}
		virtual void WriteString32(const std::u32string& value) override {
			Stream::WriteString32(std::cout, value);
		}
	};

	struct FileStream : Stream {
		std::fstream Stream;

		explicit FileStream(std::fstream stream)
			: Stream(std::move(stream)) {
			IsReadable = true;
			IsWriteable = true;
		}

		virtual std::uint32_t ReadInt() override {
			std::uint32_t value;
			Stream >> value;
			return value;
		}
		virtual std::uint32_t ReadSignedInt() override {
			std::int32_t value;
			Stream >> value;
			return value;
		}
		virtual std::uint64_t ReadLong() override {
			std::uint64_t value;
			Stream >> value;
			return value;
		}
		virtual std::uint64_t ReadSignedLong() override {
			std::int64_t value;
			Stream >> value;
			return value;
		}
		virtual double ReadDouble() override {
			double value;
			Stream >> value;
			return value;
		}
		virtual std::uint32_t ReadChar32() override {
			return Stream::ReadChar32(Stream);
		}
		virtual std::u32string ReadString32() override {
			return Stream::ReadString32(Stream);
		}

		virtual void WriteInt(std::uint32_t value) override {
			Stream << value;
		}
		virtual void WriteSignedInt(std::uint32_t value) override {
			Stream << static_cast<std::int32_t>(value);
		}
		virtual void WriteLong(std::uint64_t value) override {
			Stream << value;
		}
		virtual void WriteSignedLong(std::uint64_t value) override {
			Stream << static_cast<std::int64_t>(value);
		}
		virtual void WriteDouble(double value) override {
			Stream << value;
		}
		virtual void WriteChar32(std::uint32_t value) override {
			Stream::WriteChar32(Stream, value);
		}
		virtual void WriteString32(const std::u32string& value) override {
			Stream::WriteString32(Stream, value);
		}
	};

	struct StreamManager {
		std::list<std::unique_ptr<Stream>> Streams;
		std::uint64_t Stdin, Stdout;

		StreamManager() {
			Stdin = AddStream(std::make_unique<StdinStream>());
			Stdout = AddStream(std::make_unique<StdoutStream>());
		}

		Stream& GetStream(std::uint64_t stream) {
			return *reinterpret_cast<std::unique_ptr<Stream>*>(stream)->get();
		}
		std::uint64_t AddStream(std::unique_ptr<Stream>&& stream) {
			return reinterpret_cast<std::uint64_t>(&Streams.emplace_back(std::move(stream)));
		}
		bool RemoveStream(std::uint64_t stream) {
			const auto iter = FindStream(stream);
			if (iter == Streams.end()) return false;

			Streams.erase(iter);
			return true;
		}
		bool IsValidStream(std::uint64_t stream) {
			return FindStream(stream) != Streams.end();
		}

	private:
		decltype(Streams)::iterator FindStream(std::uint64_t stream) {
			return std::find_if(Streams.begin(), Streams.end(), [stream](const auto& streamPtr) {
				return stream == reinterpret_cast<std::uint64_t>(&streamPtr);
			});
		}
	};

	struct State {
		io::StreamManager StreamManager;

		VirtualModule::StructureIndex VirtualStream;

		VirtualModule::DependencyIndex StringModule;
		VirtualModule::MappedStructureIndex VirtualString32;

		void Init(Loader& loader, std::vector<VirtualModule*>& modules) {
			auto& module = loader.Create("/std/io.sbf");
			modules.push_back(&module);

			VirtualStream = module.AddStructure("Stream", {
				{ svm::LongType, 0 }, // _handle
			});

			module.AddFunction("getStdin", 0, true, [this](VirtualContext& context) {
				const auto result = context.PushStructure(STRUCT(VirtualStream));
				FIELD(result, 0).SetLong(StreamManager.Stdin);
			});
			module.AddFunction("getStdout", 0, true, [this](VirtualContext& context) {
				const auto result = context.PushStructure(STRUCT(VirtualStream));
				FIELD(result, 0).SetLong(StreamManager.Stdout);
			});

			module.AddFunction("openReadonlyFile", 1, true, [this](VirtualContext& context) {
				ASSERT_BEGIN {
					const auto path = PDREF_A(PARAM_A(0, PointerType), STRUCT(VirtualString32)->Type);
					const auto cppPath = string::ConvertToCppString32(context, path);

					const auto streamHandle = StreamManager.AddStream(std::make_unique<FileStream>(
						std::fstream(std::filesystem::path(cppPath), std::fstream::in)));
					const auto result = context.PushStructure(STRUCT(VirtualStream));
					FIELD(result, StreamHandle).SetLong(streamHandle);
				} ASSERT_END;
			});
			module.AddFunction("openWriteonlyFile", 1, true, [this](VirtualContext& context) {
				ASSERT_BEGIN {
					const auto path = PDREF_A(PARAM_A(0, PointerType), STRUCT(VirtualString32)->Type);
					const auto cppPath = string::ConvertToCppString32(context, path);

					const auto streamHandle = StreamManager.AddStream(std::make_unique<FileStream>(
						std::fstream(std::filesystem::path(cppPath), std::fstream::out)));
					const auto result = context.PushStructure(STRUCT(VirtualStream));
					FIELD(result, StreamHandle).SetLong(streamHandle);
				} ASSERT_END;
			});
			module.AddFunction("closeFile", 1, false, [this](VirtualContext& context) {
				ASSERT_BEGIN {
					const auto stream = PARAM_A(0, STRUCT(VirtualStream)->Type);
					const auto streamHandle = FIELD(stream, StreamHandle).ToLong();

					Assert(StreamManager.IsValidStream(streamHandle), SVM_IEC_STDLIB_IO_INVALIDSTREAM);

					auto& cppStream = static_cast<FileStream&>(StreamManager.GetStream(streamHandle));
					cppStream.Stream.close();
					StreamManager.RemoveStream(streamHandle);
				} ASSERT_END;
			});

			module.AddFunction("readInt", 1, true, [this](VirtualContext& context) {
				ASSERT_BEGIN {
					const auto stream = PARAM_A(0, STRUCT(VirtualStream)->Type);
					const auto streamHandle = FIELD(stream, StreamHandle).ToLong();

					Assert(StreamManager.IsValidStream(streamHandle), SVM_IEC_STDLIB_IO_INVALIDSTREAM);

					auto& cppStream = static_cast<FileStream&>(StreamManager.GetStream(streamHandle));
					context.PushFundamental(IntObject(cppStream.ReadInt()));
				} ASSERT_END;
			});
			module.AddFunction("writeInt", 2, false, [this](VirtualContext& context) {
				ASSERT_BEGIN {
					const auto stream = PARAM_A(0, STRUCT(VirtualStream)->Type);
					const auto streamHandle = FIELD(stream, StreamHandle).ToLong();
					const auto value = PARAM_A(1, IntType).ToInt();

					Assert(StreamManager.IsValidStream(streamHandle), SVM_IEC_STDLIB_IO_INVALIDSTREAM);

					auto& cppStream = static_cast<FileStream&>(StreamManager.GetStream(streamHandle));
					cppStream.WriteInt(value);
				} ASSERT_END;
			});
			module.AddFunction("readSignedInt", 1, true, [this](VirtualContext& context) {
				ASSERT_BEGIN {
					const auto stream = PARAM_A(0, STRUCT(VirtualStream)->Type);
					const auto streamHandle = FIELD(stream, StreamHandle).ToLong();

					Assert(StreamManager.IsValidStream(streamHandle), SVM_IEC_STDLIB_IO_INVALIDSTREAM);

					auto& cppStream = static_cast<FileStream&>(StreamManager.GetStream(streamHandle));
					context.PushFundamental(IntObject(cppStream.ReadSignedInt()));
				} ASSERT_END;
			});
			module.AddFunction("writeSignedInt", 2, false, [this](VirtualContext& context) {
				ASSERT_BEGIN {
					const auto stream = PARAM_A(0, STRUCT(VirtualStream)->Type);
					const auto streamHandle = FIELD(stream, StreamHandle).ToLong();
					const auto value = PARAM_A(1, IntType).ToInt();

					Assert(StreamManager.IsValidStream(streamHandle), SVM_IEC_STDLIB_IO_INVALIDSTREAM);

					auto& cppStream = static_cast<FileStream&>(StreamManager.GetStream(streamHandle));
					cppStream.WriteSignedInt(value);
				} ASSERT_END;
			});

			module.AddFunction("readLong", 1, true, [this](VirtualContext& context) {
				ASSERT_BEGIN {
					const auto stream = PARAM_A(0, STRUCT(VirtualStream)->Type);
					const auto streamHandle = FIELD(stream, StreamHandle).ToLong();

					Assert(StreamManager.IsValidStream(streamHandle), SVM_IEC_STDLIB_IO_INVALIDSTREAM);

					auto& cppStream = static_cast<FileStream&>(StreamManager.GetStream(streamHandle));
					context.PushFundamental(LongObject(cppStream.ReadLong()));
				} ASSERT_END;
			});
			module.AddFunction("writeLong", 2, false, [this](VirtualContext& context) {
				ASSERT_BEGIN {
					const auto stream = PARAM_A(0, STRUCT(VirtualStream)->Type);
					const auto streamHandle = FIELD(stream, StreamHandle).ToLong();
					const auto value = PARAM_A(1, LongType).ToLong();

					Assert(StreamManager.IsValidStream(streamHandle), SVM_IEC_STDLIB_IO_INVALIDSTREAM);

					auto& cppStream = static_cast<FileStream&>(StreamManager.GetStream(streamHandle));
					cppStream.WriteLong(value);
				} ASSERT_END;
			});
			module.AddFunction("readSignedLong", 1, true, [this](VirtualContext& context) {
				ASSERT_BEGIN {
					const auto stream = PARAM_A(0, STRUCT(VirtualStream)->Type);
					const auto streamHandle = FIELD(stream, StreamHandle).ToLong();

					Assert(StreamManager.IsValidStream(streamHandle), SVM_IEC_STDLIB_IO_INVALIDSTREAM);

					auto& cppStream = static_cast<FileStream&>(StreamManager.GetStream(streamHandle));
					context.PushFundamental(LongObject(cppStream.ReadSignedLong()));
				} ASSERT_END;
			});
			module.AddFunction("writeSignedLong", 2, false, [this](VirtualContext& context) {
				ASSERT_BEGIN {
					const auto stream = PARAM_A(0, STRUCT(VirtualStream)->Type);
					const auto streamHandle = FIELD(stream, StreamHandle).ToLong();
					const auto value = PARAM_A(1, LongType).ToLong();

					Assert(StreamManager.IsValidStream(streamHandle), SVM_IEC_STDLIB_IO_INVALIDSTREAM);

					auto& cppStream = static_cast<FileStream&>(StreamManager.GetStream(streamHandle));
					cppStream.WriteSignedLong(value);
				} ASSERT_END;
			});

			module.AddFunction("readDouble", 1, true, [this](VirtualContext& context) {
				ASSERT_BEGIN {
					const auto stream = PARAM_A(0, STRUCT(VirtualStream)->Type);
					const auto streamHandle = FIELD(stream, StreamHandle).ToLong();

					Assert(StreamManager.IsValidStream(streamHandle), SVM_IEC_STDLIB_IO_INVALIDSTREAM);

					auto& cppStream = static_cast<FileStream&>(StreamManager.GetStream(streamHandle));
					context.PushFundamental(DoubleObject(cppStream.ReadDouble()));
				} ASSERT_END;
			});
			module.AddFunction("writeDouble", 2, false, [this](VirtualContext& context) {
				ASSERT_BEGIN {
					const auto stream = PARAM_A(0, STRUCT(VirtualStream)->Type);
					const auto streamHandle = FIELD(stream, StreamHandle).ToLong();
					const auto value = PARAM_A(1, DoubleType).ToDouble();

					Assert(StreamManager.IsValidStream(streamHandle), SVM_IEC_STDLIB_IO_INVALIDSTREAM);

					auto& cppStream = static_cast<FileStream&>(StreamManager.GetStream(streamHandle));
					cppStream.WriteDouble(value);
				} ASSERT_END;
			});

			module.AddFunction("readChar32", 1, true, [this](VirtualContext& context) {
				ASSERT_BEGIN {
					const auto stream = PARAM_A(0, STRUCT(VirtualStream)->Type);
					const auto streamHandle = FIELD(stream, StreamHandle).ToLong();

					Assert(StreamManager.IsValidStream(streamHandle), SVM_IEC_STDLIB_IO_INVALIDSTREAM);

					auto& cppStream = static_cast<FileStream&>(StreamManager.GetStream(streamHandle));
					context.PushFundamental(IntObject(cppStream.ReadChar32()));
				} ASSERT_END;
			});
			module.AddFunction("writeChar32", 2, false, [this](VirtualContext& context) {
				ASSERT_BEGIN {
					const auto stream = PARAM_A(0, STRUCT(VirtualStream)->Type);
					const auto streamHandle = FIELD(stream, StreamHandle).ToLong();
					const auto value = PARAM_A(1, IntType).ToInt();

					Assert(StreamManager.IsValidStream(streamHandle), SVM_IEC_STDLIB_IO_INVALIDSTREAM);

					auto& cppStream = static_cast<FileStream&>(StreamManager.GetStream(streamHandle));
					cppStream.WriteChar32(value);
				} ASSERT_END;
			});

			module.AddFunction("readString32", 1, true, [this](VirtualContext& context) {
				ASSERT_BEGIN {
					const auto stream = PARAM_A(0, STRUCT(VirtualStream)->Type);
					const auto streamHandle = FIELD(stream, StreamHandle).ToLong();

					Assert(StreamManager.IsValidStream(streamHandle), SVM_IEC_STDLIB_IO_INVALIDSTREAM);

					auto& cppStream = static_cast<FileStream&>(StreamManager.GetStream(streamHandle));
					const auto result = context.PushStructure(STRUCT(VirtualString32));
					string::ConvertFromCppString32(context, cppStream.ReadString32(), result);
				} ASSERT_END;
			});
			module.AddFunction("writeString32", 2, false, [this](VirtualContext& context) {
				ASSERT_BEGIN {
					const auto stream = PARAM_A(0, STRUCT(VirtualStream)->Type);
					const auto streamHandle = FIELD(stream, StreamHandle).ToLong();
					const auto value = PDREF_A(PARAM_A(1, PointerType), STRUCT(VirtualString32)->Type);

					Assert(StreamManager.IsValidStream(streamHandle), SVM_IEC_STDLIB_IO_INVALIDSTREAM);

					auto& cppStream = static_cast<FileStream&>(StreamManager.GetStream(streamHandle));
					cppStream.WriteString32(string::ConvertToCppString32(context, value));
				} ASSERT_END;
			});

			StringModule = module.AddDependency("/std/string.sbf");
			VirtualString32 = module.AddStructureMapping(StringModule, "String32");
		}
	};
}

namespace svm {
	namespace detail {
		struct StdModuleState {
			stdlib::array::State ArrayState;
			stdlib::io::State IOState;
			stdlib::string::State StringState;
		};
	}

	StdModule InitStdModule(Loader& loader) {
		StdModule module = std::make_shared<detail::StdModuleState>();
		std::vector<VirtualModule*> modules;

		module->ArrayState.Init(loader, modules);
		module->IOState.Init(loader, modules);
		module->StringState.Init(loader, modules);

		for (const auto& module : modules) {
			loader.Build(*module);
		}

		return module;
	}
}