#include <iostream>
#include <sstream>
#include <fstream>
#include "util.hpp"

// Compile a single shader stage
GLuint compileShader(GLenum type, const std::string& filename) {
	// Read the file
	std::ifstream file(filename);
	if (!file.is_open()) {
		std::stringstream ss;
		ss << "Failed to open " << filename << std::endl;
		throw std::runtime_error(ss.str());
	}

	// Read the shader source
	std::stringstream buffer;
	buffer << file.rdbuf();
	std::string bufStr = buffer.str();
	const char* bufCStr = bufStr.c_str();
	GLint length = (GLint)bufStr.length();

	// Compile the shader
	GLuint shader = glCreateShader(type);
	glShaderSource(shader, 1, &bufCStr, &length);
	glCompileShader(shader);

	// Make sure compilation succeeded
	GLint status;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
	if (status == GL_FALSE) {
		// Compilation failed, get the info log
		GLint logLength;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);
		std::vector<GLchar> logText(logLength);
		glGetShaderInfoLog(shader, logLength, NULL, logText.data());

		// Construct an error message with the compile log
		std::stringstream ss;
		ss << "Error compiling " << filename << ":" << std::endl << std::endl;
		ss << logText.data() << std::endl;

		// Cleanup shader and throw an exception
		glDeleteShader(shader);
		throw std::runtime_error(ss.str());
	}

	return shader;
}

// Link compiled shader stages into a single program
GLuint linkProgram(std::vector<GLuint>& shaders) {
	GLuint program = glCreateProgram();

	// Attach the shaders and link the program
	for (auto it = shaders.begin(); it != shaders.end(); ++it)
		glAttachShader(program, *it);
	glLinkProgram(program);

	// Detach shaders
	for (auto it = shaders.begin(); it != shaders.end(); ++it)
		glDetachShader(program, *it);

	// Make sure link succeeded
	GLint status;
	glGetProgramiv(program, GL_LINK_STATUS, &status);
	if (status == GL_FALSE) {
		// Link failed, get the info log
		GLint logLength;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);
		std::vector<GLchar> logText(logLength);
		glGetProgramInfoLog(program, logLength, NULL, logText.data());

		// Construct an error message with the compile log
		std::stringstream ss;
		ss << "Error linking shader program:" << std::endl << std::endl;
		ss << logText.data() << std::endl;

		// Cleanup program and throw an exception
		glDeleteProgram(program);
		throw std::runtime_error(ss.str());
	}

	return program;
}
