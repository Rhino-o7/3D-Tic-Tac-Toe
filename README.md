# 3D Tic-Tac-Toe (3x3x3)

A three-dimensional 3x3x3 Tic-Tac-Toe game built with C++ and OpenGL, featuring a client-server architecture where the server hosts the game and plays as an AI opponent.

## Overview

This project consists of two programs:
- **Server**: Hosts the game and acts as the AI player
- **Client**: Provides the 3D visual interface for human players to compete against the AI

## Technologies

- **C++**: Core game logic and networking
- **OpenGL**: 3D graphics rendering
- **WebSocket**: Client-server communication

## Getting Started

### Running the Server

1. Launch the server program inside the `Builds` folder
2. When prompted, enter the port number you want the server to run on (e.g., `9003`)
3. The server will start and wait for client connections

### Running the Client

1. Launch the client program or load the webpage inside the `Builds` folder
2. Connect to the server using the WebSocket address format: `ws://<server-address>:<port>`
   - Example: `ws://ryno.com:9003`
   - **Note**: The `ws://` prefix is required
3. Once connected, select whether you want to play as **X** or **O**
   - X will always goes first
**Note**: To run the web version of the build you must run it through a server envirement
   - Example: In directory of the index.html enter the command `python -m http.server 8080` and go to `http://localhost:8080`

## How to Play

### Game Rules

This is a 3D version of Tic-Tac-Toe played on a 3x3x3 cube. The goal is to get three marks in a row, which can be:
- Horizontal (on any plane)
- Vertical (through any plane)
- Diagonal (across the cube or any plane)

### Controls

- **Left Click**: Place your move on the selected position
- **Right Click**: Rotate the board to view from different angles
- **Scroll Wheel**: Zoom in/out

## Gameplay

1. Choose to play as X or O (X always starts)
2. Wait for your turn
3. Click on an empty box to place your mark
4. The AI will automatically make its move
5. Continue until someone wins or the board is full


## Building the Projects

The `Builds` folder in the root directory contains the current builds for:
- Server
- Client
- Client-Web

### Emscripten Build (WebAssembly)

To compile the Emscripten version of the build:
1. Navigate to the `Client` folder
2. Run the `emccBuild.bat` file

### Ubuntu Build (WSL)

To build the server for Ubuntu using WSL, use the CMAKE project included in the repository.


