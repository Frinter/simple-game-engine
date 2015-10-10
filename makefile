bin/bandit-camp.exe: $(wildcard src/*)
	clang++ -o $@ -Iinclude -std=c++11 $(wildcard src/*.cc) -lmingw32 lib/platform.MINGW.64.a -lopengl32 -lgdi32 -lwinmm
