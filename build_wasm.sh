#!/bin/bash

# Build script for compiling C++ game engines to WebAssembly
# Requires Emscripten SDK (emsdk) to be installed and activated

set -e

echo "Building WebAssembly game module..."

# Check if emcc is available
if ! command -v emcc &> /dev/null; then
    echo "emcc not found in PATH, searching for emsdk..."
    
    # Possible emsdk locations
    POSSIBLE_EMSDK_PATHS=(
        "$HOME/emsdk"
        "$(dirname "$(pwd)")/emsdk"
        "/Users/alun/code/github/emsdk"
    )
    
    for path in "${POSSIBLE_EMSDK_PATHS[@]}"; do
        if [ -f "$path/emsdk_env.sh" ]; then
            echo "Found emsdk at $path, sourcing environment..."
            source "$path/emsdk_env.sh"
            break
        fi
    done
fi

# Re-check if emcc is available
if ! command -v emcc &> /dev/null; then
    echo "Error: Emscripten compiler (emcc) not found!"
    echo "Please install Emscripten SDK and source emsdk_env.sh"
    exit 1
fi

# Compile to WebAssembly
emcc cpp/wasm_bindings.cpp \
     cpp/gomoku_engine.cpp \
     cpp/ttt_engine.cpp \
     cpp/minimax.cpp \
     -o docs/game_module.js \
     -std=c++17 \
     -s WASM=1 \
     -s EXPORTED_RUNTIME_METHODS='["cwrap"]' \
     -s ALLOW_MEMORY_GROWTH=1 \
     -s MODULARIZE=1 \
     -s EXPORT_NAME='createGameModule' \
     -s ENVIRONMENT='web' \
     -O3 \
     --bind

echo "Build completed successfully!"
echo "Output files:"
echo "  - docs/game_module.js"
echo "  - docs/game_module.wasm"
