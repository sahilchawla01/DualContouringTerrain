#pragma once


#include <memory>
#include <glm/glm.hpp>
#include <GLFW/glfw3.h>
#include "Helpers/Settings.h"
#include "Enums/AppEnums.h"
#include "Helpers/Brushes/SphereBrush.h"

class ACamera;
class AActor;
class Settings;


struct RayCastResult
{
	glm::vec3 hitWorldPos = glm::vec3(0);
	bool bHit = false;
};

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
	std::shared_ptr<ACamera> m_currentCamera;
	std::shared_ptr<AActor> m_terrainActor;
	SphereBrush m_sphereBrush; 
	RayCastResult m_userBrushRaycastResult;
	EBrushType m_brushType = EBrushType::HardBrushAdd;

	int window_width = 1200;
	int window_height = 1080;

	float deltaTime = 0.f;
	float lastFrameTime = 0.f;
	float elapsedTimeSinceLaunch = 0.f;

	// -- EDITING MODE VARIABLES --
	std::shared_ptr<AActor> m_userBrushDepthPlane;
	float distanceToUserBrushPlane = 10.f;

private:
	RayCastResult RaycastForBrushPlane(double xPos, double yPos);

	//If any changes have occurred in settings, reflect changes
	void PollSettings(GLFWwindow* window) const;
	void CreateInitActors();
	void ProcessInput(GLFWwindow* window);
	static glm::vec2 GetCursorPosNDC(GLFWwindow* window);
	static int GetUniqueIndexForGrid(const int x, const int y, const int z, const int gridWidth, const int gridHeight);
	static void MouseCallback(GLFWwindow* window, double xposIn, double yposIn);
	static void MouseClickCallback(GLFWwindow* window, int button, int action, int mods);
	static void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
	static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

	//Initially, the app state is modelling 
	EAppState m_currentAppState = EAppState::Modelling;
};

// ~~ GLFW Functions ~~ 
void framebuffer_size_callback(GLFWwindow* window, int width, int height);

