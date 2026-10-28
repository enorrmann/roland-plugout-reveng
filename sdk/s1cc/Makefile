# s1cc: C / C++ -> Roland BMC DSP microcode through LLVM (docs/S1CC.md). Needs LLVM 19 (llvm-config).
LLVM_CONFIG ?= $(shell command -v llvm-config-19 || echo /usr/lib/llvm-19/bin/llvm-config)
CXXFLAGS = $(shell $(LLVM_CONFIG) --cxxflags) -O2 -Wall -Wno-unused-parameter
LDFLAGS = $(shell $(LLVM_CONFIG) --ldflags) $(shell $(LLVM_CONFIG) --libs core irreader support analysis passes transformutils) -Wl,-rpath,$(shell $(LLVM_CONFIG) --libdir)
SRC = src/main.cpp src/graph.cpp src/prof.cpp src/lower.cpp src/kernel.cpp src/fit.cpp src/loops.cpp src/devirt.cpp
OBJ = $(patsubst src/%.cpp,build/%.o,$(SRC))

build/s1cc: $(OBJ)
	$(CXX) $(OBJ) -o $@ $(LDFLAGS)

build/%.o: src/%.cpp src/s1cc.h
	@mkdir -p build
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf build
