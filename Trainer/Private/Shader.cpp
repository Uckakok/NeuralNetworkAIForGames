#include"Shader.h"
#include<iostream>
#include<fstream>
#include<string>
#include<sstream>
#include <Windows.h>
#include"Renderer.h"


int Shader::GetUniformLocation(const std::string& Name)
{
	if (m_uniformLocationCache.find(Name) != m_uniformLocationCache.end())
		return m_uniformLocationCache[Name];
	GLCall(int Location = glGetUniformLocation(m_rendererId, Name.c_str()));
	if (Location == -1)
		return Location;
		//MessageBox(NULL, L"Shader doesn't exist!", L"Error", MB_OK | MB_ICONERROR);

	m_uniformLocationCache[Name] = Location;
	return Location;
}

Shader::Shader(const std::string& Filepath) : m_filePath(Filepath), m_rendererId(0)
{
	ShaderProgramSource source = ParseShader(Filepath);
	m_rendererId = CreateShader(source.VertexSource, source.FragmentSource);
}

Shader::~Shader()
{
	GLCall(glDeleteProgram(m_rendererId));
}

void Shader::Bind() const
{
	GLCall(glUseProgram(m_rendererId));
}

void Shader::Unbind()
{
	GLCall(glUseProgram(0));
}

void Shader::SetUniform1i(const std::string& Name, int Value)
{
	GLCall(glUniform1i(GetUniformLocation(Name), Value));
}

void Shader::SetUniformMat4f(const std::string& Name, const glm::mat4& Matrix)
{
	GLCall(glUniformMatrix4fv(GetUniformLocation(Name), 1, GL_FALSE, &Matrix[0][0]));
}

unsigned int Shader::CreateShader(const std::string& VertexShader, const std::string& FragmentShader) 
{
	GLCall(unsigned int Program = glCreateProgram());
	unsigned int Vs = CompileShader(GL_VERTEX_SHADER, VertexShader);
	unsigned int Fs = CompileShader(GL_FRAGMENT_SHADER, FragmentShader);

	GLCall(glAttachShader(Program, Vs));
	GLCall(glAttachShader(Program, Fs));
	GLCall(glLinkProgram(Program));
	GLCall(glValidateProgram(Program));
	GLCall(glDeleteShader(Vs));
	GLCall(glDeleteShader(Fs));

	return Program;
}

unsigned int Shader::CompileShader(unsigned int Type, const std::string& Source) 
{
	unsigned int Id = glCreateShader(Type);
	const char* Src = Source.c_str();
	GLCall(glShaderSource(Id, 1, &Src, nullptr));
	GLCall(glCompileShader(Id));

	int Result;
	GLCall(glGetShaderiv(Id, GL_COMPILE_STATUS, &Result));
	if (Result == GL_FALSE)
	{
		int Length;
		GLCall(glGetShaderiv(Id, GL_INFO_LOG_LENGTH, &Length));
		char* Message = (char*)alloca(Length * sizeof(char));
		GLCall(glGetShaderInfoLog(Id, Length, &Length, Message));

		std::string FullMessage = "Failed to compile the shader:\n";
		FullMessage += Message;

		MessageBoxA(NULL, FullMessage.c_str(), "Error", MB_OK | MB_ICONERROR);
		GLCall(glDeleteShader(Id));
		return 0;
	}
	return Id;
}

ShaderProgramSource Shader::ParseShader(const std::string& Filepath)
{
	std::ifstream Stream(Filepath);

	enum class ShaderType 
	{
		NONE = -1, VERTEX = 0, FRAGMENT = 1
	};

	std::string Line;
	std::stringstream Ss[2];
	ShaderType Type = ShaderType::NONE;
	while (getline(Stream, Line)) 
	{
		if (Line.find("#shader") != std::string::npos)
		{
			if (Line.find("vertex") != std::string::npos) 
			{
				Type = ShaderType::VERTEX;
			}
			else if (Line.find("fragment") != std::string::npos) 
			{
				Type = ShaderType::FRAGMENT;
			}
		}
		else 
		{
			Ss[(int)Type] << Line << '\n';
		}
	}

	return { Ss[0].str(), Ss[1].str() };
}