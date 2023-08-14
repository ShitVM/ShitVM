#include <svm/Loader.hpp>

#include <svm/virtual/VirtualContext.hpp>
#include <svm/virtual/VirtualModule.hpp>

#include <algorithm>
#include <cstdint>
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

namespace svm::detail::stdlib::io {
	struct Stream {
		bool IsReadable = false;
		bool IsWriteable = false;

		virtual ~Stream() = default;

		virtual std::int32_t ReadInt() = 0;
		virtual void WriteInt(std::int32_t value) = 0;
	};

	struct ReadonlyStream : Stream {
		ReadonlyStream() {
			IsReadable = true;
		}

		virtual void WriteInt(std::int32_t) override {
			throw std::bad_function_call();
		}
	};

	struct WriteonlyStream : Stream {
		WriteonlyStream() {
			IsWriteable = true;
		}

		virtual std::int32_t ReadInt() override {
			throw std::bad_function_call();
		}
	};

	struct StdinStream : ReadonlyStream {
		virtual std::int32_t ReadInt() override {
			std::int32_t value;
			std::cin >> value;
			return value;
		}
	};

	struct StdoutStream : WriteonlyStream {
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
				{ svm::LongType, 0 }
			});

			module.AddFunction("getStdin", 0, true, [this](VirtualContext& context) {
				auto result = context.PushStructure(context.GetStructure(VirtualStream));
				context.GetField(result, 0).SetLong(StreamManager.Stdin);
			});
			module.AddFunction("getStdout", 0, true, [this](VirtualContext& context) {
				auto result = context.PushStructure(context.GetStructure(VirtualStream));
				context.GetField(result, 0).SetLong(StreamManager.Stdout);
			});

			module.AddFunction("readInt", 1, true, [this](VirtualContext& context) {
				auto stream = context.GetParameter(0); // TODO: 타입 검사
				auto streamHandle = context.GetField(stream, 0).ToLong();
				auto& cppStream = StreamManager.GetStream(streamHandle); // TODO: 유효성 검사

				context.PushFundamental(IntObject(cppStream.ReadInt()));
			});
			module.AddFunction("writeInt", 2, false, [this](VirtualContext& context) {
				auto stream = context.GetParameter(0); // TODO: 타입 검사
				auto streamHandle = context.GetField(stream, 0).ToLong();
				auto& cppStream = StreamManager.GetStream(streamHandle); // TODO: 유효성 검사

				auto value = context.GetParameter(1).ToInt(); // TODO: 타입 검사
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
		};
	}

	StdModule InitStdModule(Loader& loader) {
		StdModule module = std::make_shared<detail::StdModuleState>();

		module->IOState.Init(loader);

		return module;
	}
}