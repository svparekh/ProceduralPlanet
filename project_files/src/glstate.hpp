#ifndef GLSTATE_HPP
#define GLSTATE_HPP

#include <string>
#include <vector>
#include <memory>
#include <glm/glm.hpp>
#include "gl_core_3_3.h"
#include "camera.hpp"
#include "planet.hpp"

/*####################
####     Class    ####
####################*/

// Manages OpenGL state, e.g. camera transform, objects, shaders
class GLState {
public:
	GLState();
	~GLState();
	// Disallow copy, move, & assignment
	GLState(const GLState& other) = delete;
	GLState& operator=(const GLState& other) = delete;
	GLState(GLState&& other) = delete;
	GLState& operator=(GLState&& other) = delete;

	// Initialization
	void initLineGeometry();
	void initShaders();

	// Callbacks
	void initializeGL();
	void paintGL();
	void resizeGL(int w, int h);

	void updateTime(float time);
	void onPlanetClicked(glm::vec2 mousePos);

	// Camera
	Camera cam;

	// Planet
	PlanetSphere planet;

	// Per-vertex attributes
	struct Vertex {
		glm::vec3 pos;		// Position
		glm::vec3 norm;		// Normal
	};

	// window size:
	static int width, height;  // 600 by default

	// time
	float currentTime;

	// performance mode
	bool performanceMode;
	bool hardShadows;

	// terrain editing mode (maybe implement)
	bool placementMode;

protected:
	// OpenGL states of the platform
	GLuint lineShader;		// GPU shader program
	GLuint lineVao;			// Vertex array object
	GLuint lineVbuf;		// Vertex buffer
	GLuint lineIbuf;		// Index buffer

	GLuint camPosLoc;
	GLuint camTBNMatLoc;
	GLuint planetRotRadLoc;
	GLuint noiseOffsetLoc;
	GLuint fbmIterationsLoc;
	GLuint performanceModeLoc;
	GLuint pArrayLoc;
	GLuint rArrayLoc;
	GLuint hArrayLoc;
	GLuint numPointsLoc;
	GLuint hardShadLoc;

	glm::ivec2 iResolution;
	GLuint iResolution_uniform_loc;
};

#endif
