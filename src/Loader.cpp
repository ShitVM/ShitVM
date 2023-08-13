#include <svm/Loader.hpp>

#include <svm/Interpreter.hpp>
#include <svm/virtual/VirtualModule.hpp>
#include <svm/virtual/VirtualObject.hpp>
#include <svm/virtual/VirtualStack.hpp>

#include <iostream>

namespace svm {
	void Loader::LoadStdLibraries() {
		auto& io = static_cast<VirtualModule&>(Create("/std/io.sbf"));

		io.AddStructure("Stream", {
			{ IntType, 0 }
		});

		io.AddFunction("getStdin", 0, false, [](Interpreter& interpreter, VirtualStack& stack) -> VirtualObject {
			std::cout << "std.io.getStdin called!\n";

			return VNULL; // TODO
		});
		io.AddFunction("getStdout", 0, false, [](Interpreter& interpreter, VirtualStack& stack) -> VirtualObject {
			std::cout << "std.io.getStdout called!\n";

			return VNULL; // TODO
		});

		io.AddFunction("readInt", 0, false, [](Interpreter& interpreter, VirtualStack& stack) -> VirtualObject {
			std::cout << "std.io.readInt called!\n";

			return VNULL; // TODO
		});
		io.AddFunction("writeInt", 0, false, [](Interpreter& interpreter, VirtualStack& stack) -> VirtualObject {
			std::cout << "std.io.writeInt called!\n";

			return VNULL; // TODO
		});

		Build(io);
	}
}