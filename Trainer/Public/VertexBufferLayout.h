#pragma once

#include <vector>
#include "glew.h"
#include "Renderer.h"

struct VertexBufferElement 
{
	unsigned int Type;
	unsigned int Count;
	unsigned char Normalized;

	static unsigned int GetSizeOfType(unsigned int type) 
	{
		switch (type)
		{
		case GL_FLOAT:	return 4;
		case GL_UNSIGNED_INT:	return 4;
		case GL_UNSIGNED_BYTE:	return 1;
		}
		ASSERT(false);
		return 0;
	}
};

class VertexBufferLayout
{
private:
	std::vector<VertexBufferElement> m_elements;
	unsigned int m_stride;
public:
	VertexBufferLayout()
		: m_stride(0) {}

	void Push(unsigned int Count) 
	{
		m_elements.push_back({ GL_FLOAT, Count, GL_FALSE });
		m_stride += Count * VertexBufferElement::GetSizeOfType(GL_FLOAT);
	}

	inline const std::vector<VertexBufferElement> GetElements() const { return m_elements; }
	inline unsigned int GetStride() const { return m_stride; }
};