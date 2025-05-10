#pragma once

#include<string>
#include<unordered_map>
#include"glm/glm.hpp"

struct ShaderProgramSource 
{
	std::string VertexSource;
	std::string FragmentSource;
};

class Shader {
private:
	std::string m_filePath;
	unsigned int m_rendererId;
	std::unordered_map<std::string, int> m_uniformLocationCache;

	int GetUniformLocation(const std::string& Name);
	unsigned int CompileShader(unsigned int Type, const std::string& Source);
	unsigned int CreateShader(const std::string& VertexShader, const std::string& FragmentShader);
	ShaderProgramSource ParseShader(const std::string& Filepath);
public:
	Shader(const std::string& Filepath);
	~Shader();
	void Bind() const;
	void Unbind();
	void SetUniform1i(const std::string& Name, int Value);
	void SetUniformMat4f(const std::string& Name, const glm::mat4& Matrix);
};