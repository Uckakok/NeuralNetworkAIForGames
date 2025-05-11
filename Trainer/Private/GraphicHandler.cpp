#pragma once
#include"GraphicHandler.h"
#include"glew.h"
#include<iostream>
#include<fstream>
#include<string>
#include<sstream>
#include"glm/gtc/matrix_transform.hpp"
#include<vector>
#include<cmath>
#include <Windows.h>


static void MouseButtonCallbackWrapper(GLFWwindow* window, int button, int action, int mods)
{
	GraphicalInterface* instance = static_cast<GraphicalInterface*>(glfwGetWindowUserPointer(window));
	if (instance) 
	{
		instance->MouseButtonCallback(window, button, action, mods);
	}
}


void GraphicalInterface::MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) 
	{
		double mouseX, mouseY;
		glfwGetCursorPos(window, &mouseX, &mouseY);

		float mouseXNormalized = static_cast<float>(mouseX) / m_viewportWidth;
		float mouseYNormalized = static_cast<float>(mouseY) / m_viewportHeight;

		int gridX = static_cast<int>(mouseXNormalized * m_boardWidth);
		int gridY = static_cast<int>(mouseYNormalized * m_boardHeight);

		if (gridX >= 0 && gridX < m_boardWidth && gridY >= 0 && gridY < m_boardHeight) 
		{
			HandleClick(gridX, gridY);
		}
	}
}

void GraphicalInterface::HandleClick(int x, int y)
{
	m_currentGame->MakeMove(y, x);/*
	m_currentGame->PrintBoard();
	for (auto& pos : m_currentGame->GetValidMoves())
	{
		int from =pos / 100;
		int to = pos % 100;
		int fr = from / 8, fc = from % 8;
		int tr = to / 8, tc = to % 8;
		std::cout << fr << " " << fc << ", " << tr << " " << tc << "\n";
	}*/
	SubmitEntitiesFromGrid(m_currentGame->GetSpriteGrid());
}

GraphicalInterface::GraphicalInterface(IGame& game)
{
	if (!glfwInit())
	{
		MessageBox(NULL, L"Can't open the window. Critical error", L"Error", MB_OK | MB_ICONERROR);
		__debugbreak();
		return;
	}
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_CORE_PROFILE, GLFW_TRUE);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	m_window = glfwCreateWindow(m_viewportWidth, m_viewportHeight, "Game", NULL, NULL);
	if (!m_window)
	{
		MessageBox(NULL, L"Can't open the window. Critical error", L"Error", MB_OK | MB_ICONERROR);
		glfwTerminate();
		__debugbreak();
		return;
	}

	MakeContextCurrent();
	EnableBlending();
	PrepareVertexArray();
	PrepareVertexBuffer();
	PrepareShaders();

	glfwSetWindowUserPointer(m_window, this);
	glfwSetMouseButtonCallback(m_window, MouseButtonCallbackWrapper);

	SetBoardSize(static_cast<int>(game.GetSpriteGrid()[0].size()),
		static_cast<int>(game.GetSpriteGrid().size()));
	CalculateBoardLayout();

	m_currentGame = &game;

	auto spritePaths = game.GetSpritePaths();
	for (const auto& pair : spritePaths)
	{
		LoadSprite(pair.first, pair.second);
	}

	SubmitEntitiesFromGrid(game.GetSpriteGrid());
}

void GraphicalInterface::SetBoardSize(int width, int height)
{
	m_boardHeight = height;
	m_boardWidth = width;
}

void GraphicalInterface::CalculateBoardLayout()
{
	m_tileWidth = static_cast<float>(m_viewportWidth) / m_boardWidth;
	m_tileHeight = static_cast<float>(m_viewportHeight) / m_boardHeight;
}

GraphicalInterface::GraphicalInterface() :m_va(NULL), m_ib(NULL) 
{
	if (!glfwInit()) 
	{
		MessageBox(NULL, L"Can't open the window. Critical error", L"Error", MB_OK | MB_ICONERROR);
		__debugbreak();
		return;
	}
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	m_window = glfwCreateWindow(m_viewportWidth, m_viewportHeight, "Roguelike", NULL, NULL);
	if (!m_window)
	{
		MessageBox(NULL, L"Can't open the window. Critical error", L"Error", MB_OK | MB_ICONERROR);
		glfwTerminate();
		__debugbreak();
		return;
	}
	
}

void GraphicalInterface::MakeContextCurrent()
{
	GLCall(glfwMakeContextCurrent(m_window));

	glfwSwapInterval(1);

	if (glewInit() != GLEW_OK) 
	{
		MessageBox(NULL, L"Can't open the window. Critical error", L"Error", MB_OK | MB_ICONERROR);
		glfwTerminate();
		__debugbreak();
		return;
	}
}

void GraphicalInterface::EnableBlending() const
{
	GLCall(glEnable(GL_BLEND));
	GLCall(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
}

void GraphicalInterface::PrepareVertexArray()
{
	GLCall(glGenVertexArrays(1, &m_vao));
	GLCall(glBindVertexArray(m_vao));
}

void GraphicalInterface::PrepareVertexBuffer()
{
	float squareVertices[] = {
		-0.5f, -0.5f, 0.0f, 0.0f,
		 0.5f, -0.5f, 1.0f, 0.0f,
		 0.5f,  0.5f, 1.0f, 1.0f,
		-0.5f,  0.5f, 0.0f, 1.0f
	};
	unsigned int squareIndices[] = {
		0, 1, 2,
		2, 3, 0
	};

	m_vb = new VertexBuffer(squareVertices, sizeof(squareVertices));
	m_layout.Push(2);
	m_layout.Push(2);
	m_va = new VertexArray();
	m_va->AddBuffer(*m_vb, m_layout);
	m_ib = new IndexBuffer(squareIndices, 6);
}

void GraphicalInterface::PrepareShaders()
{
	m_shader = new Shader("Resources/Shaders/basic.shader");
	m_shader->Bind();
	m_shader->SetUniform1i("u_Texture", 0);
}

void GraphicalInterface::UnbindObjects() const
{
	GLCall(glBindVertexArray(0));
	m_va->Unbind();
	m_shader->Unbind();
	m_vb->Unbind();
	m_ib->Unbind();
}

void GraphicalInterface::LoadSprite(int spriteID, const std::string& filepath)
{
	Texture* tex = new Texture(filepath);
	tex->Bind();
	m_loadedTextures.push_back(tex);     
	m_spriteTextures[spriteID] = tex;

	if (spriteID == 0) 
	{
		m_background = tex;
	}
}


bool GraphicalInterface::WindowUpdate()
{
	if (glfwWindowShouldClose(m_window))
	{
		return false;
	}

	if (glfwGetKey(m_window, GLFW_KEY_X) == GLFW_PRESS) 
	{
		glfwSetWindowShouldClose(m_window, 1);
		return false;
	}

	m_renderer.Clear();
	m_shader->Bind();
	m_va->Bind();
	m_ib->Bind();

	if (m_background) 
	{
		glm::mat4 model = glm::mat4(1.0f);
		glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(2.0f, 2.0f, 1.0f));
		glm::mat4 mvp = scale * model;

		m_background->Bind();
		m_shader->SetUniformMat4f("u_MVP", mvp);
		m_renderer.Draw(*m_va, *m_ib, *m_shader);
	}

	m_shader->Bind();

	glm::mat4 proj = glm::ortho(0.0f, static_cast<float>(m_viewportWidth),
		static_cast<float>(m_viewportHeight), 0.0f,
		-1.0f, 1.0f); 
	m_shader->SetUniformMat4f("u_Projection", proj);

	glm::mat4 view = glm::mat4(1.0f);
	m_shader->SetUniformMat4f("u_View", view);

	for (const auto& draw : m_cachedDrawCalls)
	{
		draw.texture->Bind();
		m_shader->SetUniformMat4f("u_MVP", proj * view * draw.model);
		m_renderer.Draw(*m_va, *m_ib, *m_shader);
	}

	GLCall(glfwSwapBuffers(m_window));
	GLCall(glfwPollEvents());

	return true;
}


void GraphicalInterface::SubmitEntitiesFromGrid(const std::vector<std::vector<int>>& grid)
{
	m_cachedDrawCalls.clear();
	for (int y = 0; y < static_cast<int>(grid.size()); ++y)
	{
		for (int x = 0; x < static_cast<int>(grid[y].size()); ++x)
		{
			int spriteID = grid[y][x];
			if (spriteID == 0) continue;

			auto it = m_spriteTextures.find(spriteID);
			if (it == m_spriteTextures.end()) continue;

			Texture* tex = it->second;

			float posX = x * m_tileWidth + m_tileWidth / 2.0f;
			float posY = y * m_tileHeight + m_tileHeight / 2.0f;

			glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(posX, posY, 0.0f));
			model = glm::scale(model, glm::vec3(m_tileWidth, m_tileHeight, 1.0f));

			m_cachedDrawCalls.push_back({ tex, model });
		}
	}
}


GraphicalInterface::~GraphicalInterface()
{
	for (auto* tex : m_loadedTextures)
	{
		delete tex;
	}

	glfwTerminate();
}