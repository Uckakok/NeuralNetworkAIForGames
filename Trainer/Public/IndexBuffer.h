#pragma once

class IndexBuffer {
private:
	unsigned int m_rendererId;
	unsigned int m_count;
public:
	IndexBuffer(const unsigned int* Data, unsigned int Count);
	~IndexBuffer();

	void Bind() const;
	void Unbind() const;

	inline unsigned int GetCount() const { return m_count; }
};