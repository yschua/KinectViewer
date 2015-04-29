#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include <gl\glew.h>

namespace Core {

class ShaderLoader {
protected:
  std::string readShader(char *filename);
  GLuint createShader(GLenum shaderType, std::string shaderSource, char* shaderName);
public:
  GLuint createProgram(char* vertexShaderFilename, char* fragmentShaderFilename);
};

}