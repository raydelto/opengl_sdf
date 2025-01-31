#include "ShaderProgram.h"
#include <fstream>
#include <iostream>
#include <sstream>

#include <glm/gtc/type_ptr.hpp>

ShaderProgram::ShaderProgram(const char *vsFilename, const char *fsFilename)
	: mHandle(0)
{
	LoadShaders(vsFilename, fsFilename);
}

ShaderProgram::~ShaderProgram()
{
	// Delete the program
	glDeleteProgram(mHandle);
}

//-----------------------------------------------------------------------------
// Loads vertex and fragment shaders
//-----------------------------------------------------------------------------
bool ShaderProgram::LoadShaders(const char *vsFilename, const char *fsFilename)
{
	string vsString = FileToString(vsFilename);
	string fsString = FileToString(fsFilename);
	const GLchar *vsSourcePtr = vsString.c_str();
	const GLchar *fsSourcePtr = fsString.c_str();

	GLuint vs = glCreateShader(GL_VERTEX_SHADER);
	GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);

	glShaderSource(vs, 1, &vsSourcePtr, NULL);
	glShaderSource(fs, 1, &fsSourcePtr, NULL);

	glCompileShader(vs);
	CheckCompileErrors(vs, VERTEX);

	glCompileShader(fs);
	CheckCompileErrors(fs, FRAGMENT);

	mHandle = glCreateProgram();
	if (mHandle == 0)
	{
		std::cerr << "Unable to create shader program!" << std::endl;
		return false;
	}

	glAttachShader(mHandle, vs);
	glAttachShader(mHandle, fs);

	glLinkProgram(mHandle);
	CheckCompileErrors(mHandle, PROGRAM);

	glDeleteShader(vs);
	glDeleteShader(fs);

	mUniformLocations.clear();

	return true;
}

string ShaderProgram::FileToString(const string &filename)
{
	std::stringstream ss;
	std::ifstream file;

	try
	{
		file.open(filename, std::ios::in);

		if (!file.fail())
		{
			// Using a std::stringstream is easier than looping through each line of the file
			ss << file.rdbuf();
		}

		file.close();
	}
	catch (std::exception ex)
	{
		std::cerr << "Error reading shader filename!" << std::endl;
	}

	return ss.str();
}

void ShaderProgram::Use()
{
	if (mHandle > 0)
	{
		glUseProgram(mHandle);
	}
}

void ShaderProgram::CheckCompileErrors(GLuint shader, ShaderType type)
{
	int status = 0;

	if (type == PROGRAM)
	{
		glGetProgramiv(shader, GL_LINK_STATUS, &status);
		if (status == GL_FALSE)
		{
			GLint length = 0;
			glGetProgramiv(shader, GL_INFO_LOG_LENGTH, &length);

			// The length includes the NULL character
			string errorLog(length, ' '); // Resize and fill with space character
			glGetProgramInfoLog(shader, length, &length, &errorLog[0]);
			std::cerr << "Error! Shader program failed to link. " << errorLog << std::endl;
		}
	}
	else
	{
		glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
		if (status == GL_FALSE)
		{
			GLint length = 0;
			glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);

			// The length includes the NULL character
			string errorLog(length, ' '); // Resize and fill with space character
			glGetShaderInfoLog(shader, length, &length, &errorLog[0]);
			std::cerr << "Error! Shader failed to compile. " << errorLog << std::endl;
		}
	}
}

GLuint ShaderProgram::GetProgram() const
{
	return mHandle;
}

//-----------------------------------------------------------------------------
// Sets a float shader uniform
//-----------------------------------------------------------------------------

void ShaderProgram::SetUniform(const GLchar *name, const float &f)
{
	GLint loc = GetUniformLocation(name);
	glUniform1f(loc, f);
}

//-----------------------------------------------------------------------------
// Sets a glm::vec2 shader uniform
//-----------------------------------------------------------------------------
void ShaderProgram::SetUniform(const GLchar *name, const glm::vec2 &v)
{
	GLint loc = GetUniformLocation(name);
	glUniform2f(loc, v.x, v.y);
}

//-----------------------------------------------------------------------------
// Sets a glm::vec3 shader uniform
//-----------------------------------------------------------------------------
void ShaderProgram::SetUniform(const GLchar *name, const glm::vec3 &v)
{
	GLint loc = GetUniformLocation(name);
	glUniform3f(loc, v.x, v.y, v.z);
}

//-----------------------------------------------------------------------------
// Sets a glm::vec4 shader uniform
//-----------------------------------------------------------------------------
void ShaderProgram::SetUniform(const GLchar *name, const glm::vec4 &v)
{
	GLint loc = GetUniformLocation(name);
	glUniform4f(loc, v.x, v.y, v.z, v.w);
}

//-----------------------------------------------------------------------------
// Sets a glm::mat4 shader uniform
//-----------------------------------------------------------------------------
void ShaderProgram::SetUniform(const GLchar *name, const glm::mat4 &m)
{
	GLint loc = GetUniformLocation(name);

	// loc = location of uniform in shader
	// count = how many matrices (1 if not an array of mats)
	// transpose = False for opengl because column major
	// value = the matrix to set for the uniform
	glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(m));
}

//-----------------------------------------------------------------------------
// Returns the uniform identifier given it's string name.
// NOTE: Shader must be currently active first.
//-----------------------------------------------------------------------------
GLint ShaderProgram::GetUniformLocation(const GLchar *name)
{
	std::map<string, GLint>::iterator it = mUniformLocations.find(name);

	// Only need to query the shader program IF it doesn't already exist.
	if (it == mUniformLocations.end())
	{
		// Find it and add it to the map
		mUniformLocations[name] = glGetUniformLocation(mHandle, name);
	}

	// Return it
	return mUniformLocations[name];
}
