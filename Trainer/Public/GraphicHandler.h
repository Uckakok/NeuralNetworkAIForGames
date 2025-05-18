#pragma once

#include "texture.h"
#include "shader.h"
#include "indexBuffer.h"
#include "vertexBuffer.h"
#include "vertexArray.h"
#include "vertexBufferLayout.h"
#include "glm/glm.hpp"
#include "glfw3.h"
#include "Renderer.h"
#include "IGame.h"

#include <unordered_map>
#include <vector>
#include <string>

class GraphicalInterface {
private:
    struct CachedDraw 
    {
        Texture* texture;
        glm::mat4 model;
    };
    std::vector<CachedDraw> m_cachedDrawCalls;

    float m_tileWidth = 0.0f;
    float m_tileHeight = 0.0f;
    std::unordered_map<int, Texture*> m_spriteTextures;
    std::vector<Texture*> m_loadedTextures;
    IGame* m_currentGame = nullptr;

    GLFWwindow* m_window;
    GLuint m_vao;
    VertexArray* m_va;
    VertexBuffer* m_vb;
    IndexBuffer* m_ib;
    VertexBufferLayout m_layout;
    Shader* m_shader;
    Renderer m_renderer;

    Texture* m_background = nullptr;

    int m_viewportWidth = 640;
    int m_viewportHeight = 640;

    int m_boardWidth = 8;
    int m_boardHeight = 8;

    void CalculateBoardLayout();

public:
    /// <summary>Update board state to be rendered.</summary>
    void SubmitEntitiesFromGrid(const std::vector<std::vector<int>>& grid);
    GraphicalInterface(IGame& game);
    GraphicalInterface();

    void MakeContextCurrent();
    void EnableBlending() const;
    void PrepareVertexArray();
    void PrepareVertexBuffer();
    void PrepareShaders();

    void LoadSprite(int spriteID, const std::string& filepath);

    void UnbindObjects() const; //Can be deleted, but might be useful in future
    bool WindowUpdate();

    void SetBoardSize(int width, int height);

    void HandleClick(int x, int y);
    void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);

    ~GraphicalInterface();
};

