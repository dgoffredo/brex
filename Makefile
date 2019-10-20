BUILD_TYPE ?= Release

SOURCES := $(shell find . -name '*.cpp' '!' -name '*.t.cpp')
OBJECTS := $(SOURCES:.cpp=.o)

# Note for the unaccustomed: CPPFLAGS are for the C pre-processor, while
# CXXFLAGS are for the C++ compiler.  In general, CPP -> C pre-processor, and
# CXX -> C++ compiler.  Except when they don't (e.g. `.cpp` files are for the
# C++ compiler, though only after having gone through the C pre-processor...).
CPPFLAGS += -I src/
ifeq ($(BUILD_TYPE), Release)
     CPPFLAGS += -DNDEBUG
endif

WARNINGFLAGS += -Wall -Wextra -Wpedantic -Werror
ifeq ($(BUILD_TYPE), Release)
    OPTIMIZATIONFLAGS += -O3 -flto
else
    OPTIMIZATIONFLAGS += -O0
endif
DEBUGFLAGS += -g
CXXFLAGS += $(WARNINGFLAGS) $(OPTIMIZATIONFLAGS) $(DEBUGFLAGS) --std=c++11

brex: $(OBJECTS)
	$(CXX) -o brex $(CXXFLAGS) $(OBJECTS)

.PHONY: clean
clean:
	find src/ -type f \( -name '*.d' -o -name '*.o' -o -name '*.a' \) \
	          -exec rm {} \;
	if [ -f brex ]; then rm brex; fi

.PHONY: test
test: brex
	python3.7 -m unittest discover --start-directory test

# Use the `-MM` option of the C++ compiler to produce makefile dependencies
# corresponding to the headers included (even transitively) by each C++ file.
# The `sed` command then adds the resulting `.d` file as an additional target,
# so if any of the headers change, the makefile (`.d` file) will be remade
# (along with the `.o`).
%.d: %.cpp
	$(CXX) -MM $(CPPFLAGS) $< | sed '1 s,^.*:,$@ $(patsubst %.d,%.o,$@): ,' >$@

# Include all of the `.d` files, which are makefiles containing the rules that
# say how `.o` files depend on `.h` and `.cpp` files.
-include $(SOURCES:.cpp=.d)
