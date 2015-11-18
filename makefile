SUB_DIRS := $(shell find src -type d -print)
OBJ_DIRS := $(foreach dir,$(SUB_DIRS),$(patsubst src%,build%,$(dir)))
SRC := $(foreach dir,$(SUB_DIRS),$(wildcard $(dir)/*.cc))
OBJ := $(patsubst src/%.cc,build/%.o,$(SRC))
MAINS := $(wildcard src/*entry.cc)
MAIN_OBJ := $(patsubst src/%.cc,build/%.o,$(MAINS))
LIBS := lib/lodepng.cpp -Llib -lmingw32 lib/platform.MINGW.64.a -ljpeg -lopengl32 -lgdi32 -lwinmm -static -lstdc++
INCLUDE := -Iinclude -Isrc

TEST_SUB_DIRS := $(shell find tests -type d -print)
TEST_OBJ_DIRS := $(foreach dir,$(TEST_SUB_DIRS),$(patsubst tests%,test-build%,$(dir)))
TEST_SRC := $(foreach dir,$(TEST_SUB_DIRS),$(wildcard $(dir)/*.cc))
TEST_OBJ := $(patsubst tests/%.cc,test-build/%.o,$(TEST_SRC))

.PRECIOUS: build/%.d
.PHONY: clean test release debug tests-run
.DEFAULT_GOAL := debug

debug: test bin/debug-build.exe
release: test bin/rendering-engine.exe

test: tests-run
	@rm bin/tests.exe

tests-run: bin/tests.exe
	@echo
	@$< #-d yes

bin/debug-build.exe: $(OBJ)
	clang++ -o $@ $(INCLUDE) -std=c++11 $^ $(LIBS)

bin/rendering-engine.exe: $(OBJ)
	clang++ -o $@ $(INCLUDE) -std=c++11 $^ $(LIBS) -Wl,-subsystem,windows

bin/tests.exe: $(TEST_OBJ) $(filter-out $(MAIN_OBJ) build/imageloader.o build/objimporter/objimporter.o,$(OBJ))
	clang++ -o $@ $(INCLUDE) -std=c++11 $^ lib/lodepng.cpp -Llib -lmingw32 lib/platform.MINGW.64.a -ljpeg -lopengl32 -lgdi32 -lwinmm

$(OBJ_DIRS) $(TEST_OBJ_DIRS):
	@mkdir -p $@

build/%.d: src/%.cc | $(OBJ_DIRS)
	@clang++ -std=c++11 -MM -MT build/$*.o $(INCLUDE) -MF $@ $<

build/%.o: src/%.cc build/%.d | $(OBJ_DIRS)
	clang++ -c -o $@ -std=c++11 $(INCLUDE) $<

test-build/%.d: tests/%.cc | $(TEST_OBJ_DIRS)
	@clang++ -std=c++11 -MM -MT test-build/$*.o $(INCLUDE) -Isrc -MF $@ $<

test-build/%.o: tests/%.cc test-build/%.d | $(TEST_OBJ_DIRS)
	clang++ -c -o $@ -std=c++11 $(INCLUDE) -Isrc $<

clean:
	rm -R build
	rm -R test-build

include $(wildcard build/*.d)
include $(wildcard test-build/*.d)
