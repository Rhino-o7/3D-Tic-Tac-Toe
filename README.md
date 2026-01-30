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

1. Launch the server program
2. When prompted, enter the port number you want the server to run on (e.g., `9003`)
3. The server will start and wait for client connections

### Running the Client

1. Launch the client program
2. Connect to the server using the WebSocket address format: `ws://<server-address>:<port>`
   - Example: `ws://ryno.com:9003`
   - **Note**: The `ws://` prefix is required
3. Once connected, select whether you want to play as **X** or **O**
   - X always goes first

## How to Play

### Game Rules

This is a 3D version of Tic-Tac-Toe played on a 3x3x3 cube. The objective is to get three of your marks in a row, which can be:
- Horizontal (on any plane)
- Vertical (through any plane)
- Diagonal (across the cube)

### Controls

- **Left Click**: Place your move on the selected position
- **Right Click**: Rotate the board to view from different angles
- **Scroll Wheel**: Zoom in/out

## Gameplay

1. Choose to play as X or O (X always starts)
2. Wait for your turn
3. Click on an empty position to place your mark
4. The AI will automatically make its move
5. Continue until someone wins or the board is full

---

Enjoy playing 3D Tic-Tac-Toe!
