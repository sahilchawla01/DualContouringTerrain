#pragma once
#include <GLFW/glfw3.h>

class App
{

public:

	int window_width = 800;
	int window_height = 600;

public:
	App();

	void init();

};

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
