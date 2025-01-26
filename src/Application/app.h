#pragma once


#include <memory>
#include <GLFW/glfw3.h>
#include "Helpers/Settings.h"

class ACamera;

class App
{

public:

	Settings settings;

public:
	App(int windowWidth, int windowHeight);

	void init();
	float GetDeltaTime() const { return deltaTime; }
	float GetTimeElapsedSinceLaunch() const { return elapsedTimeSinceLaunch; }
	float GetLastFrameTime() const { return lastFrameTime; }

	~App();

private:
	std::unique_ptr<ACamera> m_currentCamera;

	int window_width = 800;
	int window_height = 600;

	float deltaTime = 0.f;
	float lastFrameTime = 0.f;
	float elapsedTimeSinceLaunch = 0.f;

private:
	void CreateInitActors();
	void ProcessInput(GLFWwindow* window);
	static void MouseCallback(GLFWwindow* window, double xposIn, double yposIn);
	static void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
};

// ~~ GLFW Functions ~~ 
void framebuffer_size_callback(GLFWwindow* window, int width, int height);

