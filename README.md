# Physics Engine

A simple 2D physics simulation written in C using SDL3. The simulation demonstrates efficient ball collision detection using spatial grid partitioning.

## Features
- Real-time simulation of up to 10,000 balls
- Efficient collision detection using a 2D grid (spatial partitioning)
- Gravity toggle
- Adjustable simulation speed
- Ball spawning with random colors and velocities
- Window resizing support
- Visual grid overlay for cell partitions

## Controls
- **Left Mouse Button:** Spawn 100 balls at mouse position
- **Up Arrow:** Increase simulation speed
- **Down Arrow:** Decrease simulation speed
- **Backspace:** Remove 100 balls
- **G:** Toggle gravity on/off
- **P:** Print ball count per cell to console
- **ESC:** Quit simulation

## Build Instructions
1. The `src` folder (with SDL3 headers and libraries) is included in this repository.
2. Compile with a C compiler, using the provided SDL3 headers and libraries. Example (Windows, MinGW):
   ```sh
   gcc main.c -o main.exe -Isrc/include -Lsrc/lib -lSDL3
   ```
3. Ensure `SDL3.dll` is in the executable directory.

## How It Works
- The simulation space is divided into a grid of cells.
- Each ball is assigned to a cell based on its position.
- Collisions are only checked between balls within the same cell, greatly improving performance.
- The grid is redrawn each frame for visualization. q

## License
MIT License
