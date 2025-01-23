//Prevent glfw from importing opengl header
#define GLFW_INCLUDE_NONE


#include "app.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "../Helpers/Shader.h"

App::App(int windowWidth, int windowHeight)
{
	this->window_width = windowWidth;
	this->window_height = windowHeight;
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

	//Compile Shader
	Shader testShader("../../../src/Shaders/Test/test.vert", "../../../src/Shaders/Test/test.frag");

	
	// set up vertex data (and buffer(s)) and configure vertex attributes
   // ------------------------------------------------------------------
	float vertices[] = {
		//positions									//colors
	 0.5f,  0.5f, 0.0f,   1.0f, 0.0f, 0.0f,// top right
	 0.5f, -0.5f, 0.0f,  0.0f, 1.0f, 0.0f,// bottom right
	-0.5f, -0.5f, 0.0f,  0.0f, 0.0f, 1.0f// bottom left
	-0.5f,  0.5f, 0.0f,   1.0f, 1.0f, 1.0f// top left 
	};

	//float vertices[] = {
	// 0.5f,  0.5f, 0.0f,  // top right
	// 0.5f, -0.5f, 0.0f,  // bottom right
	//-0.5f, -0.5f, 0.0f,  // bottom left
	//-0.5f,  0.5f, 0.0f   // top left 
	//};
	unsigned int indices[] = {  // note that we start from 0!
		0, 1, 3,   // first triangle
		1, 2, 3    // second triangle
	};

	//Generate vertex array and vertex buffer for triangle render
	unsigned int VBO, VAO, EBO;
	//Generate Vertex Array Object
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	//Generate Element Buffer Object
	glGenBuffers(1, &EBO);

	//Bind VAO
	glBindVertexArray(VAO);

	// 0. copy our vertices array in a buffer for OpenGL to use
	//Binds a buffer object to the current buffer type, only 1 can be set at one time
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	//Copy data to the buffer
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	// 1. Copy index array in an element buffer for OpenGL to use.
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
	// 2. then set the vertex attributes pointers
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	//Now we can unbind the buffer
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	//Next unbind the VAO
	glBindVertexArray(0);

	//Render frames
	while(!glfwWindowShouldClose(window))
	{
		// Set window background color and clear previous image
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		//~~ Handle Input~~ 
		processInput(window);

		//~~ Handle Rendering ~~
		testShader.use();

		//Rotate render
		glm::mat4 transform = glm::mat4(1.f);
		transform = glm::translate(transform, glm::vec3(0.25f, 0.5f, 0.f));
		transform = glm::rotate(transform, static_cast<float>(glfwGetTime()), glm::vec3(0.f, 0.f, 1.f));
		testShader.setMat4("transform", transform);

		glBindVertexArray(VAO);
		//glDrawArrays(GL_TRIANGLES, 0, 3);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

		//~~ Handle Events Polling ~~ 
		//Swap the back buffer with the front buffer (to show new image)
		glfwSwapBuffers(window);
		//Check if any events have been triggered (keyboard inputs, mouse movements, etc)
		glfwPollEvents();
	}

	// optional: de-allocate all resources once they've outlived their purpose:
	// ------------------------------------------------------------------------
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);

	//Terminate the window
	glfwTerminate();
}

/*
 * Resize the viewport of the OpenGL window to new width and height
 */
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
}

