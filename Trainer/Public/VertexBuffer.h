#pragma once

class VertexBuffer 
{
private:
	unsigned int m_rendererId;
public:
	VertexBuffer(const void* Data, unsigned int Size);
	~VertexBuffer();

	void Bind() const;
	void Unbind() const;
};