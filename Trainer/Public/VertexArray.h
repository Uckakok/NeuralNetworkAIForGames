#pragma once

#include"VertexBuffer.h"

class VertexBufferLayout;

class VertexArray 
{
private:
	unsigned int m_rendererId;
public:
	VertexArray();
	~VertexArray();

	void AddBuffer(const VertexBuffer& Vb, const VertexBufferLayout& Layout);

	void Bind() const;
	void Unbind() const;
};