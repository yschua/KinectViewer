#include "ShaderLoader.h"

using namespace Core;

std::string ShaderLoader::readShader(char *filename)
{
  std::ifstream filestream(filename);
  if (!filestream.good())
    std::cout << "Failed to read " << filename << std::endl;

  return std::string(std::istreambuf_iterator<char>(filestream),
                     std::istreambuf_iterator<char>());
}

GLuint ShaderLoader::createShader(GLenum shaderType, std::string shaderSource, char* shaderName)
{
  GLuint shader = glCreateShader(shaderType);
  const char *shaderCodePtr = shaderSource.c_str();
  const int shaderCodeSize = shaderSource.size();

  glShaderSource(shader, 1, &shaderCodePtr, &shaderCodeSize);
  glCompileShader(shader);

  // Compile error check
  GLint compileStatus;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &compileStatus);
  if (compileStatus != GL_TRUE) {
    GLint infoLogLength;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLength);
    GLchar *buffer = new GLchar[infoLogLength];
    glGetShaderInfoLog(shader, infoLogLength, NULL, buffer);
    std::cout << "ERROR compiling " << shaderName << std::endl << buffer << std::endl;
    delete[] buffer;
  }

  return shader;
}

GLuint ShaderLoader::createProgram(char* vertexShaderFilename, char* fragmentShaderFilename)
{
  // Parse shader source code
  std::string vertexShaderSource = readShader(vertexShaderFilename);
  std::string fragmentShaderSource = readShader(fragmentShaderFilename);

  // Compile shader
  GLuint vertexShader = createShader(GL_VERTEX_SHADER, vertexShaderSource, "VertexShader");
  GLuint fragmentShader = createShader(GL_FRAGMENT_SHADER, fragmentShaderSource, "FragmentShader");

  // Create program and link shaders
  GLuint program = glCreateProgram();
  glAttachShader(program, vertexShader);
  glAttachShader(program, fragmentShader);
  glLinkProgram(program);
  
  // Link error check
  GLint linkStatus;
  glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
  if (linkStatus != GL_TRUE) {
    GLint infoLogLength;
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLogLength);
    GLchar *buffer = new GLchar[infoLogLength];
    glGetProgramInfoLog(program, infoLogLength, NULL, buffer);
    std::cout << "ERROR linking" << std::endl << buffer << std::endl;
    delete[] buffer;
  }

  glDeleteShader(vertexShader);
  glDeleteShader(fragmentShader);
  return program;
}