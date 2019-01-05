#include <iostream>
#include <sstream>

#define GLEW_STATIC
#include "GL/glew.h"
#include "GLFW/glfw3.h"

#include "glm/gtc/matrix_transform.hpp"

#include "ShaderProgram.h"
#include "Texture2D.h"
#include "Camera.h"


const char* APP_TITLE = "Introduction to Modern OpenGL - Camera";
int gWindowWidth = 1024;
int gWindowHeight = 768;
GLFWwindow* gWindow = NULL;
bool gFullscreen = false;
bool gWireframe = false;
const std::string texture1Filename = "textures/wooden_crate.jpg";
const std::string texture2Filename = "textures/grid.jpg";

FPSCamera fpsCamera(glm::vec3(0.0f, 0.0f, 5.0f));
const double ZOOM_SENSITIVITY = -3.0;
const float MOVE_SPEED = 5.0; // units per second
const float MOUSE_SENSITIVITY = 0.1f;

void glfw_onKey(GLFWwindow* window, int key, int scancode, int action, int mode);
void glfw_onFramebufferSize(GLFWwindow* window, int width, int height);
void glfw_onMouseScroll(GLFWwindow* window, double deltaX, double deltaY);
void update(double elapsedTime);
void showFPS(GLFWwindow* window);
bool initOpenGL();

int main()
{
	if (!initOpenGL())
	{
		std::cerr << "GLFW initialization failed" << std::endl;
		return -1;
	}

	GLfloat vertices[] = {
		// position		 // tex coords

	   // front face
	   -1.0f,  1.0f,  1.0f, 0.0f, 1.0f,
		1.0f, -1.0f,  1.0f, 1.0f, 0.0f,
		1.0f,  1.0f,  1.0f, 1.0f, 1.0f,
	   -1.0f,  1.0f,  1.0f, 0.0f, 1.0f,
	   -1.0f, -1.0f,  1.0f, 0.0f, 0.0f,
		1.0f, -1.0f,  1.0f, 1.0f, 0.0f,

		// back face
		-1.0f,  1.0f, -1.0f, 0.0f, 1.0f,
		 1.0f, -1.0f, -1.0f, 1.0f, 0.0f,
		 1.0f,  1.0f, -1.0f, 1.0f, 1.0f,
		-1.0f,  1.0f, -1.0f, 0.0f, 1.0f,
		-1.0f, -1.0f, -1.0f, 0.0f, 0.0f,
		 1.0f, -1.0f, -1.0f, 1.0f, 0.0f,

		 // left face
		 -1.0f,  1.0f, -1.0f, 0.0f, 1.0f,
		 -1.0f, -1.0f,  1.0f, 1.0f, 0.0f,
		 -1.0f,  1.0f,  1.0f, 1.0f, 1.0f,
		 -1.0f,  1.0f, -1.0f, 0.0f, 1.0f,
		 -1.0f, -1.0f, -1.0f, 0.0f, 0.0f,
		 -1.0f, -1.0f,  1.0f, 1.0f, 0.0f,

		 // right face
		  1.0f,  1.0f,  1.0f, 0.0f, 1.0f,
		  1.0f, -1.0f, -1.0f, 1.0f, 0.0f,
		  1.0f,  1.0f, -1.0f, 1.0f, 1.0f,
		  1.0f,  1.0f,  1.0f, 0.0f, 1.0f,
		  1.0f, -1.0f,  1.0f, 0.0f, 0.0f,
		  1.0f, -1.0f, -1.0f, 1.0f, 0.0f,

		  // top face
		 -1.0f,  1.0f, -1.0f, 0.0f, 1.0f,
		  1.0f,  1.0f,  1.0f, 1.0f, 0.0f,
		  1.0f,  1.0f, -1.0f, 1.0f, 1.0f,
		 -1.0f,  1.0f, -1.0f, 0.0f, 1.0f,
		 -1.0f,  1.0f,  1.0f, 0.0f, 0.0f,
		  1.0f,  1.0f,  1.0f, 1.0f, 0.0f,

		  // bottom face
		 -1.0f, -1.0f,  1.0f, 0.0f, 1.0f,
		  1.0f, -1.0f, -1.0f, 1.0f, 0.0f,
		  1.0f, -1.0f,  1.0f, 1.0f, 1.0f,
		 -1.0f, -1.0f,  1.0f, 0.0f, 1.0f,
		 -1.0f, -1.0f, -1.0f, 0.0f, 0.0f,
		  1.0f, -1.0f, -1.0f, 1.0f, 0.0f,
	};

	glm::vec3 cubePos = glm::vec3(0.0f, 0.0f, -5.0f);
	glm::vec3 floorPos = glm::vec3(0.0f, -1.0f, 5.0f);

	GLuint vbo, vao;

	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	
	// position
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(0));
	glEnableVertexAttribArray(0);

	// tex coord
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);

	glBindVertexArray(0);					// unbind to make sure other code doesn't change it
	
	ShaderProgram shaderProgram;
	shaderProgram.loadShaders("shaders/basic.vert", "shaders/basic.frag");

	Texture2D texture1;
	texture1.loadTexture(texture1Filename, true);

	Texture2D texture2;
	texture2.loadTexture(texture2Filename, true);

	double lastTime = glfwGetTime();
	float cubeAngle = 0.0f;

	while (!glfwWindowShouldClose(gWindow))
	{
		showFPS(gWindow);

		double currentTime = glfwGetTime();
		double deltaTime = currentTime - lastTime;

		glfwPollEvents();
		update(deltaTime);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		texture1.bind(0);

		glm::mat4 model(1.0f), view, projection;
		
		model = glm::translate(model, cubePos);
		
		view = fpsCamera.getViewMatrix();

		projection = glm::perspective(glm::radians(fpsCamera.getFOV()), (float)gWindowWidth / (float)gWindowHeight, 0.1f, 100.0f);

		shaderProgram.use();

		shaderProgram.setUniform("model", model);
		shaderProgram.setUniform("view", view);
		shaderProgram.setUniform("projection", projection);
		
		glBindVertexArray(vao);
		glDrawArrays(GL_TRIANGLES, 0, 36);

		texture2.bind(0);
		//floorPos.y = -1.0f;
		model = glm::translate(model, floorPos) * glm::scale(model, glm::vec3(10.f, 0.01f, 10.0f));
		shaderProgram.setUniform("model", model);
		glDrawArrays(GL_TRIANGLES, 0, 36);


		glBindVertexArray(0);

		glfwSwapBuffers(gWindow);

		lastTime = currentTime;
	}


	glDeleteVertexArrays(1, &vao);
	glDeleteBuffers(1, &vbo);

	glfwTerminate();
	return 0;
}

bool initOpenGL()
{
	if (!glfwInit())
	{
		std::cerr << "initOpenGL GLFW initialization failed" << std::endl;
		return false;
	}
	std::cout << "initOpenGL GLFW initialization successfull" << std::endl;

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	//glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	if (gFullscreen)
	{
		GLFWmonitor* pMonitor = glfwGetPrimaryMonitor();
		const GLFWvidmode* pVmode = glfwGetVideoMode(pMonitor);
		if (pVmode != NULL)
		{
			gWindow = glfwCreateWindow(pVmode->width, pVmode->height, APP_TITLE, pMonitor, NULL);
		}
	}
	else
	{
		gWindow = glfwCreateWindow(gWindowWidth, gWindowHeight, APP_TITLE, NULL, NULL); 
	}

	if (gWindow == NULL)
	{
		std::cerr << "initOpenGL Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return false;
	}
	
	glfwMakeContextCurrent(gWindow);
	//glfwSwapInterval(0);

	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK)
	{
		std::cerr << "initOpenGL GLEW initialization failed" << std::endl;
		glfwTerminate();
		return false;
	}
	const GLubyte* renderer = glGetString(GL_RENDERER);
	const GLubyte* version = glGetString(GL_VERSION);
	std::cout << "Renderer: " << renderer << std::endl;
	std::cout << "OpenGL version supported: " << version << std::endl;
	std::cout << "OpenGL Initialization Complete" << std::endl;

	// Set the required callback functions
	glfwSetKeyCallback(gWindow, glfw_onKey);
	glfwSetFramebufferSizeCallback(gWindow, glfw_onFramebufferSize);
	glfwSetScrollCallback(gWindow, glfw_onMouseScroll);
	
	glfwSetInputMode(gWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetCursorPos(gWindow, gWindowWidth / 2.0, gWindowHeight / 2.0);

	glClearColor(0.23f, 0.38f, 0.47f, 1.0f);

	glViewport(0, 0, gWindowWidth, gWindowHeight);
	glEnable(GL_DEPTH_TEST);

	return true;
}

void glfw_onKey(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) 
		glfwSetWindowShouldClose(window, GL_TRUE);
		
	if (key == GLFW_KEY_F1 && action == GLFW_PRESS)
	{
		gWireframe = !gWireframe;
		if (gWireframe)
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		else
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
	//std::cout << "glfw_onKey key:"<< key << std::endl;
}

void glfw_onFramebufferSize(GLFWwindow* window, int width, int height)
{
	gWindowWidth = width;
	gWindowHeight = height;
	glViewport(0, 0, gWindowWidth, gWindowHeight);
}

void glfw_onMouseScroll(GLFWwindow* window, double deltaX, double deltaY)
{
	double fov = fpsCamera.getFOV() + deltaY * ZOOM_SENSITIVITY;

	fov = glm::clamp(fov, 1.0, 120.0);

	fpsCamera.setFOV((float)fov);
}

void update(double elapsedTime)
{
	// Camera orientation
	double mouseX, mouseY;

	// Get the current mouse cursor position delta
	glfwGetCursorPos(gWindow, &mouseX, &mouseY);

	// Rotate the camera the difference in mouse distance from the center screen.  Multiply this delta by a speed scaler
	fpsCamera.rotate((float)(gWindowWidth / 2.0 - mouseX) * MOUSE_SENSITIVITY, (float)(gWindowHeight / 2.0 - mouseY) * MOUSE_SENSITIVITY);

	// Clamp mouse cursor to center of screen
	glfwSetCursorPos(gWindow, gWindowWidth / 2.0, gWindowHeight / 2.0);

	// Camera FPS movement

	// Forward/backward
	if (glfwGetKey(gWindow, GLFW_KEY_W) == GLFW_PRESS)
		fpsCamera.move(MOVE_SPEED * (float)elapsedTime * fpsCamera.getLook());
	else if (glfwGetKey(gWindow, GLFW_KEY_S) == GLFW_PRESS)
		fpsCamera.move(MOVE_SPEED * (float)elapsedTime * -fpsCamera.getLook());

	// Strafe left/right
	if (glfwGetKey(gWindow, GLFW_KEY_A) == GLFW_PRESS)
		fpsCamera.move(MOVE_SPEED * (float)elapsedTime * -fpsCamera.getRight());
	else if (glfwGetKey(gWindow, GLFW_KEY_D) == GLFW_PRESS)
		fpsCamera.move(MOVE_SPEED * (float)elapsedTime * fpsCamera.getRight());

	// Up/down
	if (glfwGetKey(gWindow, GLFW_KEY_Z) == GLFW_PRESS)
		fpsCamera.move(MOVE_SPEED * (float)elapsedTime * fpsCamera.getUp());
	else if (glfwGetKey(gWindow, GLFW_KEY_X) == GLFW_PRESS)
		fpsCamera.move(MOVE_SPEED * (float)elapsedTime * -fpsCamera.getUp());
}

void showFPS(GLFWwindow* window)
{
	static double previousSeconds = 0.0;
	static int frameCount = 0;
	double elapsedSeconds;
	double currentSeconds = glfwGetTime(); // returns number of seconds since GLFW started, as double float

	elapsedSeconds = currentSeconds - previousSeconds;

	// Limit text updates to 4 times per second
	if (elapsedSeconds > 0.25)
	{
		previousSeconds = currentSeconds;
		double fps = (double)frameCount / elapsedSeconds;
		double msPerFrame = 1000.0 / fps;

		// The C++ way of setting the window title
		std::ostringstream outs;
		outs.precision(3);	// decimal places
		outs << std::fixed
			<< APP_TITLE << "    "
			<< "FPS: " << fps << "    "
			<< "Frame Time: " << msPerFrame << " (ms)";
		glfwSetWindowTitle(window, outs.str().c_str());

		// Reset for next average.
		frameCount = 0;
	}

	frameCount++;
}