#include"IndexBuffer.h"
#include"Renderer.h"


IndexBuffer::IndexBuffer(const unsigned int* Data, unsigned int Count)
	:m_count(Count)
{
	ASSERT(sizeof(unsigned int) == sizeof(GLuint));
	GLCall(glGenBuffers(1, &m_rendererId));
	GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_rendererId));
	GLCall(glBufferData(GL_ELEMENT_ARRAY_BUFFER, Count * sizeof(unsigned int), Data, GL_STATIC_DRAW));

}

IndexBuffer::~IndexBuffer()
{
	GLCall(glDeleteBuffers(1, &m_rendererId));
}

void IndexBuffer::Bind() const
{
	GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_rendererId));
}

void IndexBuffer::Unbind() const
{
	GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
}