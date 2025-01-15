//Prevent glfw from importing opengl header
#define GLFW_INCLUDE_NONE


#include "app.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

App::App()
{
}

void App::init()
{
	std::cout << "App started!";

	//Setup the GLFW window context
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	////Instantiate the GLFW window
	GLFWwindow* window = glfwCreateWindow(window_width, window_height, "Terrain Editor", NULL, NULL);

	if(!window)
	{
		std::cout << "Failed to create the GLFW window!";
		glfwTerminate();
		return;
	}
	glfwMakeContextCurrent(window);

	//Load GLAD before any OpenGL calls, (to find function pointers for OpenGL)
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return;
	}

	//Tell window dimensions to OpenGL
	glViewport(0, 0, window_width, window_height);

	//Tell glfw to call the function when window size changes
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);


	//Start render loop
	while(!glfwWindowShouldClose(window))
	{
		//Swap the back buffer with the front buffer (to show new image)
		glfwSwapBuffers(window);
		//Check if any events have been triggered (keyboard inputs, mouse movements, etc)
		glfwPollEvents();
	}

	//Terminate the window
	glfwTerminate();
	return;
}

/*
 * Resize the viewport of the OpenGL window to new width and height
 */
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

