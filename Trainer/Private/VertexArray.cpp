#include"VertexArray.h"

#include"VertexBufferLayout.h"
#include"Renderer.h"

VertexArray::VertexArray()
{
	GLCall(glGenVertexArrays(1, &m_rendererId));
}

VertexArray::~VertexArray()
{
	GLCall(glDeleteVertexArrays(1, &m_rendererId));
}

// Metoda dodaj¹ca bufor wierzcho³ków do tablicy wierzcho³ków
void VertexArray::AddBuffer(const VertexBuffer& Vb, const VertexBufferLayout& Layout)
{
	Bind();
	Vb.Bind();
	const auto& Elements = Layout.GetElements();
	unsigned int Offset = 0;
	for (unsigned int i = 0; i < Elements.size(); ++i) 
	{
		const auto& Element = Elements[i];
		GLCall(glEnableVertexAttribArray(i));
		GLCall(glVertexAttribPointer(i, Element.Count, Element.Type, Element.Normalized, Layout.GetStride(), (const void*)Offset));
		Offset += Element.Count * VertexBufferElement::GetSizeOfType(Element.Type);
	}
}

void VertexArray::Bind() const
{
	GLCall(glBindVertexArray(m_rendererId));
}

void VertexArray::Unbind() const
{
	GLCall(glBindVertexArray(0));
}
