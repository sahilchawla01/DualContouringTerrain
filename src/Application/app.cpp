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
#include "Components/USDFComponent.h"
#include "Enums/EShaderOption.h"
#include "Helpers/imgui/imgui.h"
#include "Helpers/imgui/imgui_impl_glfw.h"
#include "Helpers/imgui/imgui_impl_opengl3.h"
#include "Helpers/SDFs/BoxSDF.h"
#include "Helpers/SDFs/SphereSDF.h"

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

	//Create Sphere Actor
	std::shared_ptr<AActor> terrainActor = std::make_shared<AActor>("Terrain", m_currentCamera);
	terrainActor->SetupSDFComponent();

	//Attach the sdf component and add relevant sdfs within it
	const std::weak_ptr<USDFComponent> terrainSDFComponent = terrainActor->GetSDFComponent();
	if (!terrainSDFComponent.expired())
	{
		//terrainSDFComponent.lock()->AddSDF<BoxSDF>(glm::vec3(-3.2f, 4.8f, -3.2f), glm::vec3(1.0f));
		terrainSDFComponent.lock()->AddSDF<SphereSDF>(glm::vec3(0), 3.0f);
	}

	// Setup dual contouring grid
	std::vector<float> terrainVertices, terrainNormals, terrainDebugColors;
	std::vector<unsigned int> terrainIndices;

	const int gridSize = 15;

	DualContouring dualContouring(gridSize, gridSize, gridSize, 0.5f);


	//Create the user-brush depth plane
	std::shared_ptr<AActor> userBrushDepthPlane = std::make_shared<AActor>("User-brush Depth Plane", m_currentCamera);

	//Setup the mesh for the plane
	{
		std::vector<float> planeVertices =
		{
			-0.5f, 0.0f, 0.5f,
			-0.5f, 0.0f, -0.5f,
			0.5f, 0.0f, 0.5f,
			0.5f, 0.0f, -0.5f,
		};

		std::vector<float> planeNormals =
		{
			0.0f, 1.0f, 0.0f,
			0.0f, 1.0f, 0.0f,
			0.0f, 1.0f, 0.0f,
			0.0f, 1.0f, 0.0f,
		};

		std::vector<unsigned int> planeIndices = {
			0, 1, 2,
			1, 3, 2
		};

		userBrushDepthPlane->SetupMeshComponent(EShaderOption::unlit, planeVertices, planeNormals, planeIndices);
	}


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

		// IMGUI Frame
		{

			ImGui_ImplOpenGL3_NewFrame();
			ImGui_ImplGlfw_NewFrame();
			ImGui::NewFrame();
			ImGui::Begin("Dual Contouring Settings");                        

			ImGui::Checkbox("Enable Cursor", &settings.bIsCursorEnabled);      // Edit bools storing our window open/close state
			if (ImGui::CollapsingHeader("Debug"))
			{
				ImGui::Checkbox("Enable Debug", &settings.bIsDebugEnabled);
				ImGui::Checkbox("Enable Voxel Debug", &settings.bIsVoxelDebugEnabled);
				ImGui::Checkbox("Enable SDF Mesh Rendering", &settings.bViewMesh);
			}

			//Only show individual SDF settings if the app state is in modelling
			if (m_currentAppState == EAppState::Modelling && ImGui::CollapsingHeader("Terrain SDFs"))
			{
				if (!terrainSDFComponent.expired())
				{
					bool bSDFChanged = false;

					std::vector<std::shared_ptr<ISignedDistanceField>> sdfList = terrainSDFComponent.lock()->GetSDFList();

					//Lambda function that returns an Input::Float and sets the sdf changed flag if any change occurs
					auto SDFInputVector3WithCallback = [](const char* label, float* v, float v_step, float v_speedStep, const char* format, bool& bSDFChanged)
						{
							float original_value = *v;

							if (ImGui::InputFloat(label, v, v_step, v_speedStep, format)) {
								if (*v != original_value) {

									//If SDF value has changed, set flag 
									bSDFChanged = true;
									return true;
								}
							}
							return false;
						};

					for(size_t sdfIndex = 0; sdfIndex < sdfList.size(); ++sdfIndex)
					{
						const std::shared_ptr<ISignedDistanceField> sdfElement = sdfList[sdfIndex];

						switch (sdfElement->GetType())
						{
							case SDFType::Box:
							{
								ImGui::Text("Box Center:");
								ImGui::Spacing();

								std::shared_ptr<BoxSDF> boxSDF = std::dynamic_pointer_cast<BoxSDF>(sdfElement);

								//IMPORTANT: The complement strings are something needed for ImGUI to have UNIQUE IDs for components as I create them in a for loop
								std::string centerComplement = "##center" + std::to_string(sdfIndex);
;								SDFInputVector3WithCallback(("X"+centerComplement).c_str(), &boxSDF->center.x, 0.01f, 1.0f, "%.3f", bSDFChanged);
								SDFInputVector3WithCallback(("Y" + centerComplement).c_str(), &boxSDF->center.y, 0.01f, 1.0f, "%.3f", bSDFChanged);
								SDFInputVector3WithCallback(("Z" + centerComplement).c_str(), &boxSDF->center.z, 0.01f, 1.0f, "%.3f", bSDFChanged);

								ImGui::Text("Half-Extents:");
								ImGui::Spacing();

								std::string halfExtentComplement = "##halfExtent" + std::to_string(sdfIndex);
								SDFInputVector3WithCallback(("X" + halfExtentComplement).c_str(), &boxSDF->halfExtents.x, 0.01f, 1.0f, "%.3f", bSDFChanged);
								SDFInputVector3WithCallback(("Y" + halfExtentComplement).c_str(), &boxSDF->halfExtents.y, 0.01f, 1.0f, "%.3f", bSDFChanged);
								SDFInputVector3WithCallback(("Z" + halfExtentComplement).c_str(), &boxSDF->halfExtents.z, 0.01f, 1.0f, "%.3f", bSDFChanged);

								break;
							}
							case SDFType::Sphere:
							{
								ImGui::Text("Sphere Center");
								ImGui::Spacing();

								std::shared_ptr<SphereSDF> sphereSDF = std::dynamic_pointer_cast<SphereSDF>(sdfElement);
								std::string centerComplement = "##s_center" + std::to_string(sdfIndex);								SDFInputVector3WithCallback(("X" + centerComplement).c_str(), &sphereSDF->center.x, 0.01f, 1.0f, "%.3f", bSDFChanged);
								SDFInputVector3WithCallback(("Y" + centerComplement).c_str(), &sphereSDF->center.y, 0.01f, 1.0f, "%.3f", bSDFChanged);
								SDFInputVector3WithCallback(("Z" + centerComplement).c_str(), &sphereSDF->center.z, 0.01f, 1.0f, "%.3f", bSDFChanged);

								ImGui::Spacing();
								SDFInputVector3WithCallback("Sphere Radius", &sphereSDF->radius, 0.01f, 1.0f, "%.3f", bSDFChanged);

								break;
							}
							default:
							{
									
							}
						}
					}

					//If SDF changed at any value, set flag to regenerate mesh
					if (bSDFChanged) terrainSDFComponent.lock()->SetShouldRegenerateMesh(true);
				}

			}

			if (ImGui::Button("Begin Editing"))
			{
				m_currentAppState = EAppState::Editing;
			}

			ImGui::End();
			
		}

		//~~ Handle Rendering ~~

		//Render the SDF sphere
		{
			//Regenerate mesh behavior based on app state
			if (m_currentAppState == EAppState::Modelling)
			{
				//If any changes occur in the SDF, regenerate the mesh
				if (!terrainSDFComponent.expired() && terrainSDFComponent.lock()->GetShouldRegenerateMesh())
				{
					//Generate the mesh based on the new SDF
					dualContouring.InitGenerateMesh(terrainVertices, terrainNormals, terrainIndices, terrainDebugColors, terrainSDFComponent);

					//Set up the mesh component after generating the mesh
					terrainActor->SetupMeshComponent((Settings::bIsDuplicateVerticesDebugEnabled ? EShaderOption::flat_shade : EShaderOption::lit), terrainVertices, terrainNormals, terrainIndices, terrainDebugColors);

					//Unset flag to regenerate mesh
					terrainSDFComponent.lock()->SetShouldRegenerateMesh(false);
				}
			} else if (m_currentAppState == EAppState::Editing)
			{
				//Utilize the voxel field to edit the mesh

				//Render a spherical brush

				//Render a visual helper in the Z-axis of the camera used for getting point of brush

				//Keep brush depth plane at a distance from the camera 

				userBrushDepthPlane->Render();

			}

			//Render the terrain mesh
			terrainActor->Render();
			
		}

		//Render dual contouring vertices
		dualContouring.DebugDrawVertices(terrainActor->GetVertices(), m_currentCamera, std::make_shared<Settings>(settings));

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
