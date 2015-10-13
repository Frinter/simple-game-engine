SUB_DIRS := $(shell find src -type d -print)
OBJ_DIRS := $(foreach dir,$(SUB_DIRS),$(patsubst src%,build%,$(dir)))
SRC := $(foreach dir,$(SUB_DIRS),$(wildcard $(dir)/*.cc))

bin/bandit-camp.exe: $(patsubst src/%.cc,build/%.o,$(SRC))
	clang++ -o $@ -Iinclude -std=c++11 $^ -lmingw32 lib/platform.MINGW.64.a -lopengl32 -lgdi32 -lwinmm

$(OBJ_DIRS):
	@mkdir -p $@

build/%.d: src/%.cc | $(OBJ_DIRS)
	@clang++ -MM -MT build/$*.o -Iinclude -MF $@ $<

build/%.o: src/%.cc build/%.d | $(OBJ_DIRS)
	clang++ -c -o $@ -std=c++11 -Iinclude $<

clean:
	rm -R build/*

include $(wildcard build/*.d)
