#define NOMINMAX
#include "glstate.hpp"
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include "util.hpp"
#include <iostream>
#include <chrono>  // for high_resolution_clock
#include <cmath>

int GLState::width = 800;
int GLState::height = 800;


/*####################
####  Constructor ####
####################*/

GLState::GLState() :
	// states of the platform:
	lineShader(0),
	camPosLoc(0),
	camTBNMatLoc(0),
	planetRotRadLoc(0),
	noiseOffsetLoc(0),
	fbmIterationsLoc(0),
	performanceModeLoc(0),
	pArrayLoc(0),
	rArrayLoc(0),
	hArrayLoc(0),
	numPointsLoc(0),
	lineVao(0),
	lineVbuf(0),
	lineIbuf(0),
	hardShadLoc(0),
	iResolution_uniform_loc(-1),
	iResolution(glm::ivec2(0)),
	performanceMode(true),
	hardShadows(true),
	placementMode(false),
	currentTime(0.0f)
{
}

/*####################
####  Destructor  ####
####################*/

GLState::~GLState() {
	// Release OpenGL resources
	if (lineShader)	glDeleteProgram(lineShader);
	if (lineVao)	glDeleteVertexArrays(1, &lineVao);
	if (lineVbuf)	glDeleteBuffers(1, &lineVbuf);
	if (lineIbuf)	glDeleteBuffers(1, &lineIbuf);
}


/*####################
####      Init    ####
####################*/
// Called when OpenGL context is created (some time after construction)
void GLState::initializeGL() {
	// General settings
	glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
	glClearDepth(1.0f);
	glEnable(GL_DEPTH_TEST);

	// Initialize OpenGL state
	initShaders();
	initLineGeometry();
}

/*####################
####    Shaders   ####
####################*/
// Called when window requests a screen redraw
void GLState::paintGL() {
	// Clear the color and depth buffers
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Set shader to draw with
	glUseProgram(lineShader);

	// Send resolution to shader
	glUniform2i(iResolution_uniform_loc, width, height);

	// Send camera position, tbn, and planet rotation to shader
	glm::vec3 camCoords = cam.getCoords();
	glm::mat3 tbn = cam.getTBNMatrix();
	glUniform3f(camPosLoc, camCoords.x, camCoords.y, camCoords.z);
	glUniformMatrix3fv(camTBNMatLoc, 1, GL_FALSE, glm::value_ptr(tbn));

	planet.updateRotation();
	glUniform2f(planetRotRadLoc, planet.rotationRad.x, planet.rotationRad.y);

	// Send noise settings
	glUniform1f(noiseOffsetLoc, (float)planet.terrain.getSeed());
	glUniform1i(fbmIterationsLoc, planet.terrain.getDetailLevel());

	// Send current operating mode
	glUniform1i(performanceModeLoc, (int)performanceMode);
	glUniform1i(hardShadLoc, (int)hardShadows);

	// Send user created terrain (if any)
	size_t numP = planet.terrain.getAddedTerrainArraySize();
	glUniform3fv(pArrayLoc, (GLsizei)numP, glm::value_ptr(planet.terrain.getAddedTerrainPointsArray()[0]));
	glUniform1fv(rArrayLoc, (GLsizei)numP, &planet.terrain.getAddedTerrainRadiusArray()[0]);
	glUniform1fv(hArrayLoc, (GLsizei)numP, &planet.terrain.getAddedTerrainHeightArray()[0]);
	glUniform1i(numPointsLoc, (int)numP);

	// Use our vertex format and buffers
	glBindVertexArray(lineVao);
	// Draw the geometry
	glLineWidth((GLfloat)10.0f);  // define the width of the bar
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL);  // NOTE: use GL_LINE will fail to draw the line
	// Cleanup state
	glBindVertexArray(0);

	glUseProgram(0);
}

// Create shaders and associated state
void GLState::initShaders() {
	// Compile and link shader files
	std::vector<GLuint> lineShaders;
	lineShaders.push_back(compileShader(GL_VERTEX_SHADER, "shaders/v.glsl"));
	lineShaders.push_back(compileShader(GL_FRAGMENT_SHADER, "shaders/f.glsl"));
	lineShader = linkProgram(lineShaders);
	for (auto s : lineShaders)
		glDeleteShader(s);
	lineShaders.clear();

	// Get locations of variables on GPU
	camPosLoc				= glGetUniformLocation(lineShader, "cameraPosition"); 
	camTBNMatLoc			= glGetUniformLocation(lineShader, "camTBNMat");
	planetRotRadLoc			= glGetUniformLocation(lineShader, "planetRotationAngleRadians");
	noiseOffsetLoc			= glGetUniformLocation(lineShader, "noiseOffset"); 
	fbmIterationsLoc		= glGetUniformLocation(lineShader, "fbmIterations");
	performanceModeLoc		= glGetUniformLocation(lineShader, "performanceMode");
	iResolution_uniform_loc = glGetUniformLocation(lineShader, "iResolution");
	pArrayLoc				= glGetUniformLocation(lineShader, "pArray");
	rArrayLoc				= glGetUniformLocation(lineShader, "rArray");
	hArrayLoc				= glGetUniformLocation(lineShader, "hArray");
	numPointsLoc			= glGetUniformLocation(lineShader, "numUserAddedPoints");
	hardShadLoc				= glGetUniformLocation(lineShader, "hardShadowsEnable");

	// Initialize user generated terrain array size to 0
	glUniform1i(numPointsLoc, 0);
}

void GLState::updateTime(float time) {
	currentTime = time;
}

/*###########################
####  Generate Geometry  ####
###########################*/

void GLState::onPlanetClicked(glm::vec2 mousePos) {
	/*
	glm::vec3 pixel;
	glReadPixels(mousePos.x, height - mousePos.y, 1, 1, GL_RGB, GL_FLOAT, &(pixel[0]));
	if (glm::length(pixel) > glm::length(glm::vec3(0.05f))) {
		if (pixel != (glm::vec3(255.0f, 211.0f, 92.0f) / 255.0f)) {
			printf("Worked");
		}
	}

	*/
	//mousePos = glm::vec2(mousePos.x, height - mousePos.y);
	glm::vec2 p = glm::vec2((2.0f * mousePos.x - (float)width) / (float)width, (2.0f * (height - mousePos.y) - (float)height) / (float)height);
	glm::vec3 ro = -2.0f * cam.getCoords();
	glm::vec3 rd = cam.getTBNMatrix() * glm::normalize(glm::vec3(p, -2.0f));
	// ray march to sphere (same process as in glsl rayMarch func)
	float hit = 0.0f;
	float item = 0.0f;
	for (int i = 0; i < 128; i++) {
		glm::vec3 pos = ro + item * rd;
		hit = glm::length(pos - glm::vec3(0.0f)) - 1.0f; // get sdf
		item += hit; // increment by distance
		
		// if hit
		if ((abs(hit) < 0.00001f) || (item > 100.0f)) {
			break;
		}
	}
	if (item < 100.0f) { // if hit, add point
		glm::vec3 center = ro + item * rd;
		float radius = planet.terrain.clickTCRadius;
		float height = planet.terrain.clickTCHeight;
		planet.terrain.addTerrain(center, radius, height);
		printf("Added terrain at (%0.3f, %0.3f, %0.3f) with radius %0.3f and height %0.3f\n", center.x, center.y, center.z, radius, height);
	}
	else {
		printf("Tried to edit, but you didn't click the planet\n");
	}
	



}

void GLState::initLineGeometry() {
	// Generate an icosphere
	
	// Vertices
	std::vector<Vertex> verts{
		// Position                 // Normal
	  { {  -1.0f,  -1.0f,  0.0f }, {  0.0f, 0.0f, 0.0f }, },	// v0
	  { {  1.0f,  -1.0f,  0.0f }, {  0.0f, 0.0f, 0.0f }, },	// v1
	  { {  -1.0f,  1.0f,  0.0f }, {  0.0f, 0.0f, 0.0f }, },	// v2
	  { {  1.0f,  1.0f,  0.0f }, {  0.0f, 0.0f, 0.0f }, },	// v3
	};

	// Triangle indices
	std::vector<GLuint> inds{
		0, 1, 2,
		2, 1, 3,
	};

	// Create vertex array object
	glGenVertexArrays(1, &lineVao);
	glBindVertexArray(lineVao);

	// Create OpenGL buffers for vertex and index data
	glGenBuffers(1, &lineVbuf);
	glBindBuffer(GL_ARRAY_BUFFER, lineVbuf);
	glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(Vertex), verts.data(), GL_STATIC_DRAW);
	glGenBuffers(1, &lineIbuf);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, lineIbuf);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, inds.size() * sizeof(GLuint), inds.data(), GL_STATIC_DRAW);

	// Specify vertex attributes
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)sizeof(glm::vec3));

	// Cleanup state
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}


/*####################
####    Helpers   ####
####################*/
// Called when window is resized
void GLState::resizeGL(int w, int h) {
	// Tell OpenGL the new dimensions of the window
	width = w;
	height = h;
	glViewport(0, 0, w, h);
}
