#include <iostream>

#ifdef __APPLE__
#include <glad/glad.h>
#else
#define GLEW_STATIC
#include "GL/glew.h"
#endif

#include <GLFW/glfw3.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#include "ShaderProgram.h"
#include <map>
#include <glm/gtc/matrix_transform.hpp>
#include <memory> // Add this line

namespace
{
    std::unique_ptr<ShaderProgram> shader; // Change to unique_ptr
    unsigned int gWindowWidth = 800;
    unsigned int gWindowHeight = 600;
    const char *APP_TITLE = "OpenGL Window";
    GLFWwindow *gWindow = nullptr;

    // Structure to hold character glyph data
    struct Character
    {
        GLuint textureID; // SDF font texture
        glm::ivec2 size;  // Glyph size
        glm::ivec2 bearing;
        GLuint advance;
    };

    bool gWireframe = false;
    std::map<char, Character> Characters;
    GLuint VAO, VBO;
}

// GLFW function pointer declarations
void OnKeyPressed(GLFWwindow *window, int key, int scancode, int action, int mode);
void OnFramebufferResize(GLFWwindow *window, int width, int height);

// Initialize FreeType and generate Signed Distance Fields (SDF) font textures.
void LoadFont(const char *fontPath)
{
    FT_Library ft;
    if (FT_Init_FreeType(&ft))
    {
        std::cerr << "ERROR::FREETYPE: Could not initialize FreeType Library" << std::endl;
        return;
    }

    FT_Face face;
    if (FT_New_Face(ft, fontPath, 0, &face))
    {
        std::cerr << "ERROR::FREETYPE: Failed to load font" << std::endl;
        return;
    }

    FT_Set_Pixel_Sizes(face, 0, 48); // Set font size

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // Disable byte-alignment restriction

    for (unsigned char c = 0; c < 128; c++)
    {
        if (FT_Load_Char(face, c, FT_LOAD_RENDER))
        {
            std::cerr << "ERROR::FREETYPE: Failed to load glyph " << c << std::endl;
            continue;
        }

        GLuint texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RED,
            face->glyph->bitmap.width,
            face->glyph->bitmap.rows,
            0,
            GL_RED,
            GL_UNSIGNED_BYTE,
            face->glyph->bitmap.buffer);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        Character character = {
            texture,
            glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
            glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
            static_cast<GLuint>(face->glyph->advance.x)};
        Characters.insert(std::pair<char, Character>(c, character));
    }

    FT_Done_Face(face);
    FT_Done_FreeType(ft);
    glBindTexture(GL_TEXTURE_2D, 0);
}

// Initialize OpenGL buffers
void InitBuffers()
{
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

// Render text using Signed Distance Fields (SDF) shader
void RenderText(ShaderProgram *shader, std::string text, float x, float y, float scale, glm::vec3 color)
{
    shader->SetUniform("textColor", color);
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(VAO);

    for (auto c : text)
    {
        Character ch = Characters[c];

        float xpos = x + ch.bearing.x * scale;
        float ypos = y - (ch.size.y - ch.bearing.y) * scale;
        float w = ch.size.x * scale;
        float h = ch.size.y * scale;

        float vertices[6][4] = {
            {xpos, ypos + h, 0.0f, 0.0f},
            {xpos, ypos, 0.0f, 1.0f},
            {xpos + w, ypos, 1.0f, 1.0f},
            {xpos, ypos + h, 0.0f, 0.0f},
            {xpos + w, ypos, 1.0f, 1.0f},
            {xpos + w, ypos + h, 1.0f, 0.0f}};

        glBindTexture(GL_TEXTURE_2D, ch.textureID);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid *)(0));
        glEnableVertexAttribArray(0);

        // Next  Frame Position attribute
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid *)(2 * sizeof(GLfloat)));
        glEnableVertexAttribArray(1);

        glDrawArrays(GL_TRIANGLES, 0, 6);

        x += (ch.advance >> 6) * scale;
    }

    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

bool InitGL()
{
    // Intialize GLFW
    // GLFW is configured.  Must be called before calling any GLFW functions
    if (!glfwInit())
    {
        // An error occured
        std::cerr << "GLFW initialization failed" << std::endl;
        return false;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // forward compatible with newer versions of OpenGL as they become available but not backward compatible (it will not run on devices that do not support OpenGL 3.3

    // Create an OpenGL 3.3 core, forward compatible context window
    gWindow = glfwCreateWindow(gWindowWidth, gWindowHeight, APP_TITLE, NULL, NULL);
    if (gWindow == NULL)
    {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return false;
    }

    // Make the window's context the current one
    glfwMakeContextCurrent(gWindow);
    glfwSetKeyCallback(gWindow, OnKeyPressed);
    glfwSetFramebufferSizeCallback(gWindow, OnFramebufferResize);

#ifdef __APPLE__
    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return -1;
    }
#else
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
    {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        return false;
    }
#endif

    glClearColor(0.3, 0.3f, 0.3f, 1.0f);

    // Define the viewport dimensions
    glViewport(0, 0, gWindowWidth, gWindowHeight);
    return true;
}

void OnKeyPressed(GLFWwindow *window, int key, int scancode, int action, int mode)
{

    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);

    if (key == GLFW_KEY_F1 && action == GLFW_PRESS)
    {
        gWireframe = !gWireframe;
        if (gWireframe)
        {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        }
        else
        {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }
    }
}

void OnFramebufferResize(GLFWwindow *window, int width, int height)
{
    gWindowWidth = width;
    gWindowHeight = height;
    glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(gWindowWidth), 0.0f, static_cast<float>(gWindowHeight));
    shader->SetUniform("projection", projection);

    glViewport(0, 0, gWindowWidth, gWindowHeight);
}

int main()
{
    InitGL();

    shader = std::make_unique<ShaderProgram>("shaders/sdf.vert", "shaders/sdf.frag");
    shader->Use();
    glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(gWindowWidth), 0.0f, static_cast<float>(gWindowHeight));
    shader->SetUniform("projection", projection);

    LoadFont("fonts/roboto.ttf");
    InitBuffers();

    while (!glfwWindowShouldClose(gWindow))
    {
        glClear(GL_COLOR_BUFFER_BIT);
        RenderText(shader.get(), "ABC abc", gWindowWidth / 2, gWindowHeight / 2, 1.0f, glm::vec3(1.0, 0.0, 0.0));
        glfwSwapBuffers(gWindow);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
