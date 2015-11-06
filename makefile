SUB_DIRS := $(shell find src -type d -print)
OBJ_DIRS := $(foreach dir,$(SUB_DIRS),$(patsubst src%,build%,$(dir)))
SRC := $(foreach dir,$(SUB_DIRS),$(wildcard $(dir)/*.cc))
OBJ := $(patsubst src/%.cc,build/%.o,$(SRC))
MAINS := $(wildcard src/*entry.cc)
MAIN_OBJ := $(patsubst src/%.cc,build/%.o,$(MAINS))

TEST_SUB_DIRS := $(shell find tests -type d -print)
TEST_OBJ_DIRS := $(foreach dir,$(TEST_SUB_DIRS),$(patsubst tests%,test-build%,$(dir)))
TEST_SRC := $(foreach dir,$(TEST_SUB_DIRS),$(wildcard $(dir)/*.cc))
TEST_OBJ := $(patsubst tests/%.cc,test-build/%.o,$(TEST_SRC))

.PRECIOUS: build/%.d
.PHONY: clean test release debug tests-run
.DEFAULT_GOAL := debug

debug: bin/debug-build.exe
release: bin/bandit-camp.exe
test: tests-run
	@rm bin/tests.exe

tests-run: bin/tests.exe
	@echo
	@$< #-d yes

bin/debug-build.exe: $(OBJ)
	clang++ -o $@ -Iinclude -std=c++11 $^ -lmingw32 lib/platform.MINGW.64.a -lopengl32 -lgdi32 -lwinmm -static -lstdc++

bin/bandit-camp.exe: $(OBJ)
	clang++ -o $@ -Iinclude -std=c++11 $^ -lmingw32 lib/platform.MINGW.64.a -lopengl32 -lgdi32 -lwinmm -static -lstdc++ -Wl,-subsystem,windows

$(OBJ_DIRS) $(TEST_OBJ_DIRS):
	@mkdir -p $@

build/%.d: src/%.cc | $(OBJ_DIRS)
	@clang++ -std=c++11 -MM -MT build/$*.o -Iinclude -MF $@ $<

build/%.o: src/%.cc build/%.d | $(OBJ_DIRS)
	clang++ -c -o $@ -std=c++11 -Iinclude $<

test-build/%.d: tests/%.cc | $(TEST_OBJ_DIRS)
	@clang++ -std=c++11 -MM -MT test-build/$*.o -Iinclude -Isrc -MF $@ $<

test-build/%.o: tests/%.cc test-build/%.d | $(TEST_OBJ_DIRS)
	clang++ -c -o $@ -std=c++11 -Iinclude -Isrc $<

bin/tests.exe: $(TEST_OBJ) $(filter-out $(MAIN_OBJ),$(OBJ))
	clang++ -o $@ -Iinclude -std=c++11 $^

clean:
	rm -R build
	rm -R test-build

include $(wildcard build/*.d)
include $(wildcard test-build/*.d)
