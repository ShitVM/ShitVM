#include <svm/Loader.hpp>

#include <svm/virtual/VirtualContext.hpp>
#include <svm/virtual/VirtualModule.hpp>

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <functional>
#include <iostream>
#include <utility>
#include <list>

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

namespace svm::detail::stdlib::string {
	constexpr std::uint32_t StringData = 0;
	constexpr std::uint32_t StringLength = 1;
	constexpr std::uint32_t StringCapacity = 2;

	void Expand32(VirtualContext& context, VirtualObject& string, std::uint64_t required) {
		auto capacity = FIELD(string, StringCapacity).ToLong();
		if (required <= capacity) return;

		if (capacity == 0) {
			capacity = required;
		} else {
			do {
				capacity <<= 1; // Doubling
			} while (required > capacity);
		}

		auto data = FIELD(string, StringData);
		auto newData = context.NewFundamental(IntObject(), capacity);
		if (data.ToPointer() != VPNULL) {
			context.CopyObject(newData, PDREF(data));
			context.DeleteObject(data);
		}

		data.SetPointer(PREF(newData));
		FIELD(string, StringCapacity).SetLong(capacity);
	}

	struct State {
		VirtualModule::StructureIndex VirtualString32;

		void Init(svm::Loader& loader) {
			auto& module = loader.Create("/std/string.sbf");

			VirtualString32 = module.AddStructure("String32", {
				{ svm::PointerType, 0 }, // data
				{ svm::LongType, 0 }, // length
				{ svm::LongType, 0 }, // capacity
			});

			module.AddFunction("createString", 0, true, [this](VirtualContext& context) {
				auto result = context.PushStructure(STRUCT(VirtualString32));
				FIELD(result, StringData).SetPointer(VPNULL);
				FIELD(result, StringLength).SetLong(0);
				FIELD(result, StringCapacity).SetLong(0);
			});

			module.AddFunction("push", 2, false, [this](VirtualContext& context) {
				auto string = PDREF(PARAM(0)); // TODO: 타입 검사
				auto stringLength = FIELD(string, StringLength).ToLong();

				Expand32(context, string, stringLength + 1);
				ITEM(PDREF(FIELD(string, StringData)), stringLength).SetInt(PARAM(1).ToInt()); // TODO: 타입 검사
				FIELD(string, StringLength).SetLong(stringLength + 1);
			});
			module.AddFunction("concat", 2, false, [this](VirtualContext& context) {
				auto dest = PDREF(PARAM(0)); // TODO: 타입 검사
				auto src = PDREF(PARAM(1)); // TODO: 타입 검사
				auto destLength = FIELD(dest, StringLength).ToLong();
				auto srcLength = FIELD(src, StringLength).ToLong();

				Expand32(context, dest, destLength + srcLength);
				context.CopyObjectUnsafe(
					PREF(ITEM(PDREF(FIELD(dest, StringData)), destLength)),
					PREF(ITEM(PDREF(FIELD(src, StringData)), 0)), srcLength);
				FIELD(dest, StringLength).SetLong(destLength + srcLength);
			});

			module.AddFunction("destroy", 1, false, [this](VirtualContext& context) {
				auto string = PDREF(PARAM(0)); // TODO: 타입 검사
				auto data = FIELD(string, StringData);
				if (data.ToPointer() != VPNULL) {
					context.DeleteObject(data);
					data.SetPointer(VPNULL);
				}
			});

			loader.Build(module);
		}
	};
}

namespace svm::detail::stdlib::io {
	constexpr std::uint32_t StreamHandle = 0;

	struct Stream {
		bool IsReadable = false;
		bool IsWriteable = false;

		virtual ~Stream() = default;

		virtual std::uint32_t ReadChar32() = 0;
		virtual std::int32_t ReadInt() = 0;

		virtual void WriteChar32(std::uint32_t value) = 0;
		virtual void WriteInt(std::int32_t value) = 0;
	};

	struct ReadonlyStream : Stream {
		ReadonlyStream() {
			IsReadable = true;
		}

		virtual void WriteChar32(std::uint32_t) override {
			throw std::bad_function_call();
		}
		virtual void WriteInt(std::int32_t) override {
			throw std::bad_function_call();
		}
	};

	struct WriteonlyStream : Stream {
		WriteonlyStream() {
			IsWriteable = true;
		}

		virtual std::uint32_t ReadChar32() override {
			throw std::bad_function_call();
		}
		virtual std::int32_t ReadInt() override {
			throw std::bad_function_call();
		}
	};

	struct StdinStream : ReadonlyStream {
		virtual std::uint32_t ReadChar32() override {
			char value;
			std::cin >> value;
			return value;
		}
		virtual std::int32_t ReadInt() override {
			std::int32_t value;
			std::cin >> value;
			return value;
		}
	};

	struct StdoutStream : WriteonlyStream {
		virtual void WriteChar32(std::uint32_t value) override {
			std::cout << static_cast<char>(value);
		}
		virtual void WriteInt(std::int32_t value) override {
			std::cout << value;
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
		StreamManager StreamManager;

		VirtualModule::StructureIndex VirtualStream;

		void Init(svm::Loader& loader) {
			auto& module = loader.Create("/std/io.sbf");

			VirtualStream = module.AddStructure("Stream", {
				{ svm::LongType, 0 }, // _handle
			});

			module.AddFunction("getStdin", 0, true, [this](VirtualContext& context) {
				auto result = context.PushStructure(STRUCT(VirtualStream));
				FIELD(result, 0).SetLong(StreamManager.Stdin);
			});
			module.AddFunction("getStdout", 0, true, [this](VirtualContext& context) {
				auto result = context.PushStructure(STRUCT(VirtualStream));
				FIELD(result, 0).SetLong(StreamManager.Stdout);
			});

			module.AddFunction("readChar32", 1, true, [this](VirtualContext& context) {
				auto stream = PARAM(0); // TODO: 타입 검사
				auto streamHandle = FIELD(stream, StreamHandle).ToLong();
				auto& cppStream = StreamManager.GetStream(streamHandle); // TODO: 유효성 검사

				context.PushFundamental(IntObject(cppStream.ReadChar32()));
			});
			module.AddFunction("writeChar32", 2, false, [this](VirtualContext& context) {
				auto stream = PARAM(0); // TODO: 타입 검사
				auto streamHandle = FIELD(stream, StreamHandle).ToLong();
				auto& cppStream = StreamManager.GetStream(streamHandle); // TODO: 유효성 검사

				auto value = PARAM(1).ToInt(); // TODO: 타입 검사
				cppStream.WriteChar32(value);
			});

			module.AddFunction("readInt", 1, true, [this](VirtualContext& context) {
				auto stream = PARAM(0); // TODO: 타입 검사
				auto streamHandle = FIELD(stream, StreamHandle).ToLong();
				auto& cppStream = StreamManager.GetStream(streamHandle); // TODO: 유효성 검사

				context.PushFundamental(IntObject(cppStream.ReadInt()));
			});
			module.AddFunction("writeInt", 2, false, [this](VirtualContext& context) {
				auto stream = PARAM(0); // TODO: 타입 검사
				auto streamHandle = FIELD(stream, StreamHandle).ToLong();
				auto& cppStream = StreamManager.GetStream(streamHandle); // TODO: 유효성 검사

				auto value = PARAM(1).ToInt(); // TODO: 타입 검사
				cppStream.WriteInt(value);
			});

			loader.Build(module);
		}
	};
}

namespace svm {
	namespace detail {
		struct StdModuleState {
			stdlib::io::State IOState;
			stdlib::string::State StringState;
		};
	}

	StdModule InitStdModule(Loader& loader) {
		StdModule module = std::make_shared<detail::StdModuleState>();

		module->IOState.Init(loader);
		module->StringState.Init(loader);

		return module;
	}
}