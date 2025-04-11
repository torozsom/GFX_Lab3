//=============================================================================================
// OpenGL keretrendszer: GLFW �s GLAD alap� implement�ci�
//=============================================================================================
#include "framework.h"
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

// Keretrendszer �llapota
static int minorNumber = 3, majorNumber = 3;
static int windowWidth = 600, windowHeight = 600;
static const char * windowCaption = "Grafika";
static GLFWwindow* window;
static bool screenRefresh = true;
static glApp * pApp = nullptr;

// Esem�nykezel�k
static void error_callback(int error, const char* description) {
	fprintf(stderr, "Error: %s\n", description);
}

static void window_refresh_callback(GLFWwindow* window) {
	screenRefresh = true;
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, GLFW_TRUE);
		return;
	}
	if ((mods & GLFW_MOD_SHIFT) == 0) key += 'a' - 'A';
	if (action == GLFW_PRESS || action == GLFW_REPEAT) pApp->onKeyboard(key);
	if (action == GLFW_RELEASE) pApp->onKeyboardUp(key);
}

void character_callback(GLFWwindow* window, unsigned int codepoint) {
	pApp->onKeyboard(codepoint);
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
	double pX, pY;
	glfwGetCursorPos(window, &pX, &pY);
	if (action == GLFW_PRESS) pApp->onMousePressed((button == GLFW_MOUSE_BUTTON_LEFT) ? MOUSE_LEFT : MOUSE_RIGHT, (int)pX, (int)pY);
	else				      pApp->onMouseReleased((button == GLFW_MOUSE_BUTTON_LEFT) ? MOUSE_LEFT : MOUSE_RIGHT, (int)pX, (int)pY);
}

static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos) {
	pApp->onMouseMotion((int)xpos, (int)ypos);
}

// Applik�ci� konstruktora
glApp::glApp(unsigned int _majorNumber, unsigned int _minorNumber, unsigned int _windowWidth, unsigned int _windowHeight, const char * _windowCaption) {
	majorNumber = _majorNumber;
	minorNumber = _minorNumber;
	windowWidth = _windowWidth;
	windowHeight = _windowHeight;
	windowCaption = _windowCaption;
	pApp = this;
}

// Applik�ci� konstruktora
glApp::glApp(const char * _windowCaption) {
	majorNumber = 3;
	minorNumber = 3;
	windowWidth = 600;
	windowHeight = 600;
	windowCaption = _windowCaption;
	pApp = this;
}

// Rajzold �jra az alkalmaz�si ablakot
void glApp::refreshScreen() {
	screenRefresh = true;
}

// Lek�rdez�ses klaviat�ra kezel�s
bool pollKey(int key) {
	return (glfwGetKey(window, key) == GLFW_PRESS);
}


int main(void) {
	// Alkalmaz�i ablak l�trehoz�sa
	glfwSetErrorCallback(error_callback);
	if (!glfwInit()) exit(EXIT_FAILURE);

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, majorNumber);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, minorNumber);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	window = glfwCreateWindow(windowWidth, windowHeight, windowCaption, NULL, NULL);
	if (!window) {
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	// Esem�nykezel�k regisztr�l�sa
	//glfwSetKeyCallback(window, key_callback);
	glfwSetCharCallback(window, character_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwSetCursorPosCallback(window, cursor_position_callback);
	glfwSetWindowRefreshCallback(window, window_refresh_callback);

	glfwMakeContextCurrent(window);
	gladLoadGL();
	glfwSwapInterval(1);

	// Applik�ci� inicializ�l�sa
	pApp->onInitialization();
	float startTime = 0;

	// �zenetkezel� hurok
	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents(); // esem�nyek lek�rdez�se �s reakci�

		float endTime = (float)glfwGetTime();    // id� lek�rdez�se
		pApp->onTimeElapsed(startTime, endTime); // anim�ci�
		startTime = endTime;

		if (screenRefresh) {
			pApp->onDisplay();       // rajzol�s
			glfwSwapBuffers(window); // buffercsere
			screenRefresh = false;
		}
	}
	glfwDestroyWindow(window);
	glfwTerminate();
	exit(EXIT_SUCCESS);
}
