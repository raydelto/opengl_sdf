# Simple Signed Distance Fields (SDF) font Implementation in OpenGL

Simple Signed Distance Fields (SDF) font implementation written in C++ with OpenGL.

This projects provides a simple way of rendering text in OpenGL.

## Sample usage:

`RenderText(shader, "ABC abc", gWindowWidth / 2, gWindowHeight / 2, 1.0f, glm::vec3(1.0, 0.0, 0.0));` 

Where the function signature is:

`RenderText(ShaderProgram *shader, std::string text, float x, float y, float scale, glm::vec3 color)` 

![Font Test](https://www.raydelto.org/img/font_test.png)

## Dependencies
1. glfw3
2. libfreetype
3. GLEW (For Linux & Windows Build) ; GLAD (for MacOS builds).

## Build

We provide a simple Makefile that provides support for builindg the project in the following environments:
1. Windows (MinGW)
2. Linux
3. MacOS

Should you have any question you can contact us at info@raycasters.com .
