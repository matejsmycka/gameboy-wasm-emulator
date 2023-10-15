#!MAKE
#emcc -o build/index.html $(find src -name "*.c" && find tool -name "*.c") -Wall -std=c99 -D_DEFAULT_SOURCE -Wno-missing-braces -Wunused-result -Os -I. -Itool  -I src/ -I src/system -s USE_GLFW=3 -s ASYNCIFY -s TOTAL_MEMORY=67108864 -s FORCE_FILESYSTEM=1 --shell-file /usr/lib/emscripten/src/shell_minimal.html -DPLATFORM_WEB -s "EXPORTED_FUNCTIONS=["_free","_malloc","_main"]" -s EXPORTED_RUNTIME_METHODS=ccall -DCLIENT_SDL2 -sUSE_SDL=2 -s ALLOW_MEMORY_GROWTH=1 -s TOTAL_STACK=32MB
#AUTHOR: Matej Smycka

CC=emcc
OUT=build/index.html
C_FILES=$(shell find src -name "*.c" && find tool -name "*.c")
CFLAGS=-Wall -std=c99 -D_DEFAULT_SOURCE -Wno-missing-braces -Wunused-result -Os 
EMSFLAGS= -s USE_GLFW=3 -s ASYNCIFY -s TOTAL_MEMORY=67108864 -s FORCE_FILESYSTEM=1 --shell-file /usr/lib/emscripten/src/shell_minimal.html -DPLATFORM_WEB -s "EXPORTED_FUNCTIONS=["_free","_malloc","_main"]" -s EXPORTED_RUNTIME_METHODS=ccall -DCLIENT_SDL2 -sUSE_SDL=2 -s ALLOW_MEMORY_GROWTH=1 -s TOTAL_STACK=32MB
H_FILES=-I. -Itool  -I src/ -I src/system

all:
	$(CC) -o $(OUT) $(C_FILES) $(CFLAGS) $(H_FILES) $(EMSFLAGS)

clean:
	rm -rf build/*