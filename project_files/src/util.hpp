#ifndef UTIL_HPP
#define UTIL_HPP

#include <string>
#include <vector>
#include "gl_core_3_3.h"

GLuint compileShader(GLenum type, const std::string& filename);
GLuint linkProgram(std::vector<GLuint>& shaders);

#endif
