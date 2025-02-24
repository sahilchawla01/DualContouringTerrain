//Prevent glfw from importing opengl header
#define GLFW_INCLUDE_NONE


#include "app.h"

#include <iostream>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "../Helpers/Shader.h"
#include "Actors/ACamera.h"
#include "Helpers/DualContouring.h"


//IMGUI INCLUDES

#include "Actors/AActor.h"
#include "Enums/EShaderOption.h"
#include "Helpers/imgui/imgui.h"
#include "Helpers/imgui/imgui_impl_glfw.h"
#include "Helpers/imgui/imgui_impl_opengl3.h"

App::App(int windowWidth, int windowHeight)
{
	this->window_width = windowWidth;
	this->window_height = windowHeight;
}

void App::init()
{

	//Init settings
	settings = Settings();

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
	glfwSetCursorPosCallback(window, App::MouseCallback);
	glfwSetScrollCallback(window, App::ScrollCallback);
	glfwSetKeyCallback(window, App::KeyCallback);
	glfwSetInputMode(window, GLFW_CURSOR, settings.bIsCursorEnabled ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);

	//Load GLAD before any OpenGL calls, (to find function pointers for OpenGL)
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD\n";
		return;
	}

	//Tell window dimensions to OpenGL
	glViewport(0, 0, window_width, window_height);

	//Store pointer to app on window
	glfwSetWindowUserPointer(window, this);


	//Enable depth buffer and depth testing
	glEnable(GL_DEPTH_TEST);

	//Tell glfw to call the function when window size changes
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);


	CreateInitActors();


	//Setup IMGUI
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	// Setup Platform/Renderer backends
	ImGui_ImplGlfw_InitForOpenGL(window, true);          // Second param install_callback=true will install GLFW callbacks and chain to existing ones.
	ImGui_ImplOpenGL3_Init();


	// Setup Grid
	std::vector<float> terrainVertices, terrainNormals, terrainDebugColors;
	std::vector<unsigned int> terrainIndices;

	DualContouring dualContouring(15, 15, 15, 1);
	dualContouring.GenerateMesh(terrainVertices, terrainNormals, terrainIndices, terrainDebugColors);

	//Create Sphere Actor
	std::shared_ptr<AActor> sphereSDFActor = std::make_shared<AActor>("TestSphere", m_currentCamera);
	sphereSDFActor->SetupMeshComponent(EShaderOption::lit, terrainVertices, terrainNormals, terrainIndices, terrainDebugColors);

	//Render frames
	while(!glfwWindowShouldClose(window))
	{
		// Set window background color and clear previous image
		glClearColor(0.f, 0.f, 0.f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//Store time variables
		float currentTime = static_cast<float>(glfwGetTime());
		elapsedTimeSinceLaunch = currentTime;
		deltaTime = currentTime - lastFrameTime;
		lastFrameTime = currentTime;

		//~~ Handle Input~~ 
		ProcessInput(window);
		PollSettings(window);

		// Start the Dear ImGui frame
		{

			ImGui_ImplOpenGL3_NewFrame();
			ImGui_ImplGlfw_NewFrame();
			ImGui::NewFrame();
			ImGui::Begin("Dual Contouring Settings!");                        

			ImGui::Checkbox("Enable Cursor", &settings.bIsCursorEnabled);      // Edit bools storing our window open/close state
			if (ImGui::CollapsingHeader("Debug"))
			{
				ImGui::Checkbox("Enable Debug", &settings.bIsDebugEnabled);
				ImGui::Checkbox("Enable Voxel Debug", &settings.bIsVoxelDebugEnabled);
				ImGui::Checkbox("Enable SDF Mesh Rendering", &settings.bViewMesh);
			}
			//ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)

			//ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
			ImGui::End();
			
		}

		//~~ Handle Rendering ~~

		//Render the SDF sphere
		sphereSDFActor->Render();
		//Render dual contouring vertices
		dualContouring.DebugDrawVertices(sphereSDFActor->GetVertices(), m_currentCamera, std::make_shared<Settings>(settings));

		//Render the UI
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		//~~ Handle Events Polling ~~ 
		//Swap the back buffer with the front buffer (to show new image)
		glfwSwapBuffers(window);
		//Check if any events have been triggered (keyboard inputs, mouse movements, etc)
		glfwPollEvents();

	}

	//Clean up code
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	//Terminate the window
	glfwTerminate();
}

void App::PollSettings(GLFWwindow* window) const
{
	glfwSetInputMode(window, GLFW_CURSOR, settings.bIsCursorEnabled ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);

}

//Creates all required actors for the scene
void App::CreateInitActors()
{

	//Create camera
	m_currentCamera = std::make_shared<ACamera>(glm::vec3(0.f, 0.f, 10.f), glm::vec3(0.f, 1.f, 0.f), settings.mouse_yaw, settings.mouse_pitch, static_cast<float>(this->window_width) / static_cast<float>(this->window_height));

}

int App::GetUniqueIndexForGrid(const int x, const int y, const int z, const int gridWidth, const int gridHeight)
{
	return x + gridWidth * (y + gridHeight * z);
}

void App::ProcessInput(GLFWwindow* window)
{
	//-- Handle WASD movement for camera  --
	if (m_currentCamera)
	{
		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		{
			m_currentCamera->ProcessKeyboardInput(ECameraMoveDirection::FORWARD, deltaTime);
			/*glm::vec3 newCamPos = currentCameraPosition + (cameraSpeed * deltaTime) * cameraFront;
			SetCameraPosition(newCamPos);*/
		}
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		{
			m_currentCamera->ProcessKeyboardInput(ECameraMoveDirection::BACKWARD, deltaTime);
			/*glm::vec3 newCamPos = currentCameraPosition - (cameraSpeed * deltaTime) * cameraFront;
			SetCameraPosition(newCamPos);*/
		}
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		{
			m_currentCamera->ProcessKeyboardInput(ECameraMoveDirection::LEFT, deltaTime);
			/*glm::vec3 newCamPos = currentCameraPosition - glm::normalize(glm::cross(cameraFront, cameraUp)) * (cameraSpeed * deltaTime);
			SetCameraPosition(newCamPos);*/
		}
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		{
			m_currentCamera->ProcessKeyboardInput(ECameraMoveDirection::RIGHT, deltaTime);
			/*glm::vec3 newCamPos = currentCameraPosition + glm::normalize(glm::cross(cameraFront, cameraUp)) * (cameraSpeed * deltaTime);
			SetCameraPosition(newCamPos);*/
		}
	} else
	{
		std::cout << "\nError: Camera not present.";
	}

	////Handle cursor display
	//if (glfwGetKey(window, GLFW_KEY_C) == GLFW_REPEAT)
	//{
	//	//Flip flag
	//	settings.bIsCursorEnabled = !settings.bIsCursorEnabled;
	//	glfwSetInputMode(window, GLFW_CURSOR, settings.bIsCursorEnabled ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
	//}
}


/*
 * Resize the viewport of the OpenGL window to new width and height
 */
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

void App::MouseCallback(GLFWwindow* window, double xposIn, double yposIn)
{
	//Get app pointer
	App* appPtr = static_cast<App*>(glfwGetWindowUserPointer(window));

	appPtr->m_currentCamera->ProcessMouseInput(static_cast<float>(xposIn), static_cast<float>(yposIn), appPtr->settings.bIsCursorEnabled);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void App::ScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
	//Get app pointer
	App* appPtr = static_cast<App*>(glfwGetWindowUserPointer(window));

	appPtr->m_currentCamera->ProcessScrollInput(static_cast<float>(xoffset), static_cast<float>(yoffset));
}

void App::KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	//Get app pointer
	App* appPtr = static_cast<App*>(glfwGetWindowUserPointer(window));

	if (key == GLFW_KEY_C && action == GLFW_PRESS)
	{
		//Flip flag
		appPtr->settings.bIsCursorEnabled = !appPtr->settings.bIsCursorEnabled;
		glfwSetInputMode(window, GLFW_CURSOR, appPtr->settings.bIsCursorEnabled ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
	}
}

App::~App()
{
}
