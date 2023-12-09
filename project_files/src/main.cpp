#include <iostream>
#include <memory>
#include <filesystem>
#include <algorithm>
#include "glstate.hpp"
#include <GL/freeglut.h>


/*####################
####    Globals   ####
####################*/

// OpenGL state
std::unique_ptr<GLState>	glState;
float timeWhenMouseClicked;

/*####################
####  Prototypes  ####
####################*/

// Initialization functions
void	initGLUT(int* argc, char** argv);

// Callback functions
void	display();
void	reshape(GLint width, GLint height);
void	keyRelease(unsigned char key, int x, int y);
void	mouseBtn(int button, int state, int x, int y);
void	mouseMove(int x, int y);
void	idle();
void	cleanup();


/*####################
####     Main     ####
####################*/

// Program entry point
int main(int argc, char** argv) {
	try {
		// Create the window
		initGLUT(&argc, argv);
		// Initialize OpenGL (buffers, shaders, etc.)
		glState = std::unique_ptr<GLState>(new GLState());
		glState->initializeGL();

	} catch (const std::exception& e) {
		// Handle any errors
		std::cerr << "Fatal error: " << e.what() << std::endl;
		cleanup();
		return -1;
	}

	std::cout << "\n######################################## -- Controls: -- ########################################\n" << std::endl;
	std::cout << "\n	Mouse:\n" << std::endl;
	std::cout << "		- Press and hold RIGHT MOUSE BUTTON to ORBIT around the PLANET\n" << std::endl;
	std::cout << "		- Press and hold LEFT MOUSE BUTTON to ROTATE the PLANET\n" << std::endl;
	std::cout << "		- Use the mouse SCROLL WHEEL to ZOOM in/out\n" << std::endl;
	//std::cout << "		- Hold LSHIFT and RIGHT MOUSE BUTTON to ROTATE the SUN\n" << std::endl;
	std::cout << "\n	Keyboard:\n" << std::endl;
	std::cout << "		- Tap 's' to reload shaders\n" << std::endl;
	std::cout << "		- Tap 'p' to toggle performance mode\n" << std::endl;
	std::cout << "			Default ON. Turns off soft shadows + reflections + water normals\n" << std::endl;
	std::cout << "			- Tap 'h' to toggle hard shadows in performance mode\n" << std::endl;
	std::cout << "		- Tap 'f' to toggle placement mode, enables terrain editing\n" << std::endl;
	std::cout << "			Edits here do not save automatically\n" << std::endl;
	std::cout << "			- Tap the LEFT MOUSE BUTTON to generate terrain at the mouse cursor's location on the planet \n" << std::endl;
	std::cout << "			- Tap 'CTRL' + 'z' to remove last edit (including those loaded from the config file) \n" << std::endl;
	std::cout << "			- Tap 'CTRL' + 's' to save all changes to config file \n" << std::endl;
	std::cout << "			Control the radius: \n" << std::endl;
	std::cout << "				- Tap 'c' to increment the radius by 0.05\n" << std::endl;
	std::cout << "				- Tap 'z' to decrement the radius by 0.05\n" << std::endl;
	std::cout << "			Control the height: \n" << std::endl;
	std::cout << "				- Tap 'e' to increment the height by 0.05\n" << std::endl;
	std::cout << "				- Tap 'q' to decrement the height by 0.05\n" << std::endl;
	std::cout << "		- Tap 'r' to load/reload the terrain edits saved in the config file\n" << std::endl;
	std::cout << "		- Tap ESC to exit the application\n" << std::endl;
	std::cout << "		Control amount of detail:\n" << std::endl;
	std::cout << "			- Tap 'c' to increment the fractal brownian motion (FBM) iterations\n" << std::endl;
	std::cout << "			- Tap 'z' to decrement the fractal brownian motion (FBM) iterations\n" << std::endl;
	std::cout << "		Control seed: \n" << std::endl;
	std::cout << "			- Tap 'e' to increment the procedural world seed\n" << std::endl;
	std::cout << "			- Tap 'q' to decrement the procedural world seed\n" << std::endl;
	std::cout << "\n#################################################################################################\n\n" << std::endl;

	// Execute main loop
	glutMainLoop();

	return 0;
}


/*####################
####     Init     ####
####################*/

// Setup window and callbacks
void initGLUT(int* argc, char** argv) {
	// Set window and context settings
	glutInit(argc, argv);
	glutInitWindowSize(GLState::width, GLState::height);
	glutInitContextVersion(3, 3);
	glutInitContextProfile(GLUT_CORE_PROFILE);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);
	// Create the window
	glutCreateWindow("Procedural Terrain");

	// GLUT callbacks
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutKeyboardUpFunc(keyRelease);
	glutMouseFunc(mouseBtn);
	glutMotionFunc(mouseMove);
	glutIdleFunc(idle);
	glutCloseFunc(cleanup);
}


/*####################
####   Callbacks  ####
####################*/

// Called whenever a screen redraw is requested
void display() {
	// Tell the GLState to render the scene
	glState->paintGL();

	// Scene is rendered to the back buffer, so swap the buffers to display it
	glutSwapBuffers();
}

// Called when the window is resized
void reshape(GLint width, GLint height) {
	// Tell OpenGL the new window size
	glState->resizeGL(width, height);
}

// Called when a mouse button is pressed or released
void mouseBtn(int button, int state, int x, int y) {
	// Press left mouse button
	if (state == GLUT_DOWN && button == GLUT_LEFT_BUTTON) {
		// Start camera rotation
		timeWhenMouseClicked = glState->currentTime;
		glState->planet.startRotation(glm::vec2(x, y));
	}
	// Release left mouse button
	if (state == GLUT_UP && button == GLUT_LEFT_BUTTON) {
		if (glState->placementMode) {
			// Check if planet is not moving and mouse was clicked not dragged, then call planet click function
			if (((glState->currentTime - timeWhenMouseClicked) < 100.0f) && (glState->planet.rotationVelocity == glm::vec2(0.0f))) {
				glState->onPlanetClicked(glm::vec2(x, y));
			}
		}
		// Stop camera rotation
		glState->planet.endRotation();
	}

	// Press right mouse button
	if (state == GLUT_DOWN && button == GLUT_RIGHT_BUTTON) {
		// Start camera rotation
		glState->cam.startRotation(glm::vec2(x, y));
	}
	// Release right mouse button
	if (state == GLUT_UP && button == GLUT_RIGHT_BUTTON) {
		// Stop camera rotation
		glState->cam.endRotation();
	}

	// Scroll wheel up
	if (button == 3) {
		glState->cam.zoom(-0.05f);
		glutPostRedisplay();
	}
	// Scroll wheel down
	if (button == 4) {
		glState->cam.zoom(0.05f);
		glutPostRedisplay();
	}
}

// Called when the mouse moves
void mouseMove(int x, int y) {
	if (glState->cam.isRotating) {
		// Rotate the camera if currently rotating
		glState->cam.rotate(glm::vec2(x, y));
		glutPostRedisplay();

	}
	else if (glState->planet.isRotating) {
		glState->planet.rotate(glm::vec2(x, y));
		glutPostRedisplay();
	}
}

static auto start = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now());	// record start time

// Called when there are no events to process
void idle() {
	// NOTE: anything that happens every frame (e.g. movement) should be done here
	// Be sure to call glutPostRedisplay() if the screen needs to update as well
	auto	finish	= std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now());	// record end time
	auto	elapsed = static_cast<float>((finish - start).count());

	if ((int)elapsed % (int)(1000.0/60.0) == 0) {
		glState->updateTime(elapsed);
		glutPostRedisplay();
	}
}

// Called when the window is closed or the event loop is otherwise exited
void cleanup() {
	// Delete the GLState object, calling its destructor,
	// which releases the OpenGL objects
	glState.reset(nullptr);
}


void keyRelease(unsigned char key, int x, int y) {
	int keyModifier = glutGetModifiers();

	switch (key) {
		case 27:  // escape
			glutLeaveMainLoop();
			break;
		case 26:  // 26 for ctrl+z
			if ((glState->placementMode) && (keyModifier == GLUT_ACTIVE_CTRL)) {
				glState->planet.terrain.undoAddTerrain();
				printf("Removed an edit (if any)\n");
			}
			break;
		case 19:  // 19 for ctrl+s
			if ((glState->placementMode) && (keyModifier == GLUT_ACTIVE_CTRL)) {
				glState->planet.terrain.save("config.txt");
				printf("Saved edits to config.txt \n");
			}
			break;
		case 'H':
		case 'h':
			glState->hardShadows = !glState->hardShadows;
			printf("Hard shadows turned %s. \n", glState->hardShadows ? "ON" : "OFF");
			break;
		case 'P': // performance mode
		case 'p':
			glState->performanceMode = !glState->performanceMode;
			printf("Performance mode turned %s. \n", glState->performanceMode ? "ON" : "OFF");
			break;
		case 'R':
		case 'r':
			glState->planet.terrain.load("config.txt");
			printf("Parsed and loaded user config. \n");
			break;
		case 'F':
		case 'f':
			glState->placementMode = !glState->placementMode;
			printf("Placement mode turned %s. \n", glState->placementMode ? "ON" : "OFF");
			break;
		case 'S': // shaders
		case 's':
			if (!((glState->placementMode) && (keyModifier == GLUT_ACTIVE_CTRL))) {
				glState->initShaders();
				printf("Reloaded Shaders. \n");
			}
			break;
		case 'Q': // procedural generation seed settings / terrain edit height settings
		case 'q':
			// dec height in placement mode
			if (glState->placementMode) {
				glState->planet.terrain.clickTCHeight -= 0.01f;
				printf("Decrrmented edit height. Height: %0.3f \n", glState->planet.terrain.clickTCHeight);
			}
			// dec seed otherwise
			else {
				glState->planet.terrain.decSeed();
				printf("Loaded Procedural World. SEED: %d \n", glState->planet.terrain.getSeed());
			}
			break;
		case 'E': 
		case 'e':
			// inc height in placement mode
			if (glState->placementMode) {
				glState->planet.terrain.clickTCHeight += 0.01f;
				printf("Incremented edit height. Height: %0.3f \n", glState->planet.terrain.clickTCHeight);
			}
			// // inc seed otherwise
			else {
				glState->planet.terrain.incSeed();
				printf("Loaded Procedural World. SEED: %d \n", glState->planet.terrain.getSeed());
			}
			break;
		case 'Z': // fractal brownian motion settings / terrain edit radius settings
		case 'z':
			if (glState->placementMode) {
				// dec radius
				glState->planet.terrain.clickTCRadius -= 0.01f;
				printf("Decremented edit radius. Radius: %0.3f \n", glState->planet.terrain.clickTCRadius);
			}
			// dec fbm iterations otherwise
			else {
				glState->planet.terrain.decDetail();
				printf("Decremented FBM iterations. DETAIL LEVEL: %d \n", glState->planet.terrain.getDetailLevel());
			}
			break;
		case 'C': 
		case 'c':
			// inc radius in placement mode
			if (glState->placementMode) {
				glState->planet.terrain.clickTCRadius += 0.01f;
				printf("Incremented edit radius. Radius: %0.3f \n", glState->planet.terrain.clickTCRadius);
			}
			// inc fbm iterations otherwise
			else {
				glState->planet.terrain.incDetail();
				printf("Incremented FBM iterations. DETAIL LEVEL: %d \n", glState->planet.terrain.getDetailLevel());
			}
			break;
		default:
			break;
	}
}