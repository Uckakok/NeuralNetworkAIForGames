#include"Renderer.h"
#include<iostream>
#include <Windows.h>

void GLClearError() 
{
	for (int i = 0; i < GLAD_GET_ERROR_MAX; ++i)
	{
		while (glGetError() == GL_NO_ERROR)
		{
			break;
		}
	}
}

bool GLLogCall(const char* Function, const char* File, int Line) 
{
	while (GLenum Error = glGetError()) 
	{
		char ErrorMessage[512];
		sprintf_s(ErrorMessage, "OpenGL error in file %s at line %d: %x", File, Line, Error);
		MessageBoxA(NULL, ErrorMessage, "OpenGL Error", MB_OK | MB_ICONERROR);
		return false;
	}
	return true;
}

void Renderer::Clear() const 
{
	GLCall(glClear(GL_COLOR_BUFFER_BIT));
}

void Renderer::Draw(const VertexArray& Va, const IndexBuffer& Ib, const Shader& Shader) const
{
	Shader.Bind();
	Va.Bind();
	Ib.Bind();
	GLCall(glDrawElements(GL_TRIANGLES, Ib.GetCount(), GL_UNSIGNED_INT, nullptr));
}