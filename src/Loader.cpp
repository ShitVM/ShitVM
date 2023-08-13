#include <svm/Loader.hpp>

#include <svm/Interpreter.hpp>
#include <svm/virtual/VirtualContext.hpp>
#include <svm/virtual/VirtualModule.hpp>

#include <cstdint>
#include <fstream>
#include <iostream>
#include <vector>

#include <cstdint>
#include <tuple>
#include <ios>
#include <unordered_map>

namespace svm {
	void Loader::LoadStdLibraries() {
		LoadStdIoLibrary();
	}
}

namespace svm {
	namespace stdlib::io {
		struct Stream {
			virtual ~Stream() = default;

			virtual std::uint32_t ReadSignedInt() = 0;
			virtual std::uint32_t ReadUnsignedInt() = 0;

			virtual void WriteSignedInt(std::uint32_t value) = 0;
			virtual void WriteUnsignedInt(std::uint32_t value) = 0;
		};

		struct StdioStream : Stream {
			virtual std::uint32_t ReadSignedInt() override {
				std::int32_t value;
				std::cin >> value;
				return value;
			}
			virtual std::uint32_t ReadUnsignedInt() override {
				std::uint32_t value;
				std::cin >> value;
				return value;
			}

			virtual void WriteSignedInt(std::uint32_t value) override {
				std::cout << value;
			}
			virtual void WriteUnsignedInt(std::uint32_t value) override {
				std::cout << value;
			}
		};

		struct FileStream : Stream {
			std::fstream CppStream;

			virtual std::uint32_t ReadSignedInt() override {
				std::int32_t value;
				CppStream >> value;
				return value;
			}
			virtual std::uint32_t ReadUnsignedInt() override {
				std::uint32_t value;
				CppStream >> value;
				return value;
			}

			virtual void WriteSignedInt(std::uint32_t value) override {
				CppStream << value;
			}
			virtual void WriteUnsignedInt(std::uint32_t value) override {
				CppStream << value;
			}
		};

		struct StreamManager {
			std::vector<std::unique_ptr<Stream>> Streams;
		};
	}

	void Loader::LoadStdIoLibrary() {
		auto& io = static_cast<VirtualModule&>(Create("/std/io.sbf"));

		static const auto stream = io.AddStructure("Stream", {
			{ LongType, 0 }
		});

		static stdlib::io::StreamManager streamManager;
		streamManager.Streams.push_back(std::unique_ptr<stdlib::io::Stream>(new stdlib::io::StdioStream));

		io.AddFunction("getStdio", 0, true, [](VirtualContext& context) {
			auto result = context.PushStructure(context.GetStructure(stream));
			context.GetField(result, 0).SetLong(reinterpret_cast<std::uint64_t>(streamManager.Streams[0].get()));
		});

		io.AddFunction("readInt", 1, true, [](VirtualContext& context) {
			auto stream = context.GetParameter(0); // TODO: 타입 검사
			auto cppStream = reinterpret_cast<stdlib::io::Stream*>(context.GetField(stream, 0).ToLong()); // TODO: 유효성 검사

			context.PushFundamental(IntObject(cppStream->ReadSignedInt()));
		});
		io.AddFunction("writeInt", 2, false, [](VirtualContext& context) {
			auto stream = context.GetParameter(0); // TODO: 타입 검사
			auto cppStream = reinterpret_cast<stdlib::io::Stream*>(context.GetField(stream, 0).ToLong()); // TODO: 유효성 검사

			auto value = context.GetParameter(1).ToInt(); // TODO: 타입 검사
			cppStream->WriteSignedInt(value);
		});

		Build(io);
	}
}