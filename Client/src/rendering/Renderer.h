#pragma once

#include <GL/glew.h>
#include "VertexArray.h"
#include "IndexBuffer.h"
#include "Shader.h"

//#define ASSERT(x) if(!(x)) __debugbreak();

static class Renderer {
private:

public: 
	static void Clear() ;
	static void Draw(const VertexArray& va, const IndexBuffer& ib, const Shader& shader) ;
};

