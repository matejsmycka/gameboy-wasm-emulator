# Gameboy Pokemon WASM Emulator

This is a Gameboy emulator[1] written by David Jolly in C.
I ported, modified, and compiled it to WebAssembly.

Check it out: https://525073.gitlab-pages.ics.muni.cz/wasm-pokemon/

Pokemon Red ROM is included in the repository, bundled as an `xxd` output file.
I had to modify the emulator to load in-compile time instead of reading it from a file, which is impossible in `WASM` runtime. I tried using `Raylib` for rendering, but even with native compilation, I could not get it working. I switched to `SDL2`. The `emcc` supports it both, making it easier to understand.

## Controls

- Arrow keys: D-Pad
- J: A
- K: B
- Space: Start
- L: Select

## Requirements

- sdl2
- libc
- glib2
- emscripten SDK with a functional environment
- emscripten compiler

## Build

```bash
make
python3 -m http.server
```

## Disclaimer

This project is POC, and many features are not implemented.
Missing features:

- Sound
- Save/Load
- The screen could be bigger.
- Add PokemonRed.gb to the wasm environment, not as an array of bytes.
- Optimize memory and CPU resources.
- Code quality

## Sources

- [1]: Based on [GameBoy Emulator by David Jolly](https://sr.ht/~dajolly/dmgl/). I chose this emulator because it is lightweight and uses SDL2 or Raylib for rendering. I might switch to another emulator in the future.
- 2: Good resource: [Cpp-Retro-Snake-Game-with-raylib](https://github.com/educ8s/Cpp-Retro-Snake-Game-with-raylib)
- 3: [Article about SDL WASM support](https://www.jamesfmackenzie.com/2019/12/01/webassembly-graphics-with-sdl/)
- 4: [WASMBOY, Similar project, but written in AssemblyScript](https://github.com/torch2424/wasmboy)
