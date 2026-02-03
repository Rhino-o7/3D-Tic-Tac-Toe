@echo off
setlocal

REM =========================
REM Paths
REM =========================
set BUILD_DIR=..\Builds\Client-Web
set IMGUI_DIR=src\vendor\imgui
set SRC_DIR=src
set CORE_DIR=..\Core

REM =========================
REM Output directory
REM =========================
if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%"

REM =========================
REM Build with Emscripten
REM =========================
emcc ^
%SRC_DIR%\main.cpp ^
%SRC_DIR%\Application.cpp ^
%SRC_DIR%\VoxelClient.cpp ^
%SRC_DIR%\WebsocketClient.cpp ^
%SRC_DIR%\rendering\Renderer.cpp ^
%SRC_DIR%\rendering\Shader.cpp ^
%SRC_DIR%\rendering\Texture.cpp ^
%SRC_DIR%\rendering\VertexBuffer.cpp ^
%SRC_DIR%\rendering\IndexBuffer.cpp ^
%SRC_DIR%\rendering\VertexArray.cpp ^
%SRC_DIR%\rendering\Camera.cpp ^
%SRC_DIR%\tests\Test.cpp ^
%SRC_DIR%\tests\TestClearColor.cpp ^
%SRC_DIR%\tests\TestTexture.cpp ^
%SRC_DIR%\tests\Test3D.cpp ^
%SRC_DIR%\tests\TestChunk.cpp ^
%SRC_DIR%\voxel\Chunk.cpp ^
%SRC_DIR%\voxel\ChunkBuilder.cpp ^
%SRC_DIR%\voxel\ChunkRenderer.cpp ^
%SRC_DIR%\voxel\ChunkMesh.cpp ^
%SRC_DIR%\voxel\TextureAtlas.cpp ^
%SRC_DIR%\vendor\stb\stb_image.cpp ^
%CORE_DIR%\src\board.cpp ^
%CORE_DIR%\src\game.cpp ^
%CORE_DIR%\src\network_message.cpp ^
%IMGUI_DIR%\imgui.cpp ^
%IMGUI_DIR%\imgui_draw.cpp ^
%IMGUI_DIR%\imgui_tables.cpp ^
%IMGUI_DIR%\imgui_widgets.cpp ^
%IMGUI_DIR%\imgui_impl_glfw.cpp ^
%IMGUI_DIR%\imgui_impl_opengl3.cpp ^
-o "%BUILD_DIR%\index.html" ^
-DIMGUI_IMPL_OPENGL_ES3 ^
-s USE_GLFW=3 ^
-s USE_WEBGL2=1 ^
-s FULL_ES3=1 ^
-s ALLOW_MEMORY_GROWTH=1 ^
-s WASM=1 ^
-s ASSERTIONS=1 ^
-lwebsocket.js ^
-Isrc ^
-I%CORE_DIR%\\include ^
-I%IMGUI_DIR% ^
-Isrc\\vendor\\stb ^
-Isrc\\vendor\\glm ^
-std=c++17 --preload-file res@/res ^

if errorlevel 1 (
    echo.
    echo BUILD FAILED
    exit /b 1
)

echo.
echo BUILD SUCCESSFUL
