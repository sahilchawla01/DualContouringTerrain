//Prevent glfw from importing opengl header
#define GLFW_INCLUDE_NONE


#include "app.h"

#include <iostream>
#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "../Helpers/Shader.h"
#include "Actors/ACamera.h"
#include "Helpers/DualContouring.h"


//IMGUI INCLUDES

#include "Actors/AActor.h"
#include "Components/UMeshComponent.h"
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
	glfwSetMouseButtonCallback(window, App::MouseClickCallback);

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

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	//Tell glfw to call the function when window size changes
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	//Create important actors like the camera
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
	m_terrainActor = std::make_shared<AActor>("Terrain", m_currentCamera);
	m_terrainActor->SetupSDFComponent();

	//Attach the sdf component and add relevant sdfs within it
	const std::weak_ptr<USDFComponent> terrainSDFComponent = m_terrainActor->GetSDFComponent();
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
	m_userBrushDepthPlane = std::make_shared<AActor>("User-brush Depth Plane", m_currentCamera, m_currentCamera->GetCameraWorldPosition(), glm::vec3(6.0), glm::vec3(90, 0, 0));

	distanceToUserBrushPlane = 10.f;

	//Setup the user brush depth plane
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

		m_userBrushDepthPlane->SetupMeshComponent(EShaderOption::unlit, planeVertices, planeNormals, planeIndices);
		m_userBrushDepthPlane->GetMeshComponent().lock()->SetObjectColor(glm::vec4(1.0f, 1.0f, 1.0f, 0.3f));
	}


	//Create the user brush (sphere)
	std::shared_ptr<AActor> userBrushSphere= std::make_shared<AActor>("User Brush(Sphere)", m_currentCamera, glm::vec3(0));

	//By default, don't update sphere
	m_sphereBrush.bUpdateSDF = false;


	//Setup the user brush (sphere)
	{
		std::vector<float> vertices;
		std::vector<float> normals;
		std::vector<unsigned int> indices;
		float radius = 1.f;
		int sectorCount = 16;
		int stackCount = 16;

		float x, y, z, xy;                              // vertex position
		float nx, ny, nz, lengthInv = 1.0f / radius;    // vertex normal

		float sectorStep = 2.f * glm::pi<float>() / sectorCount;
		float stackStep = glm::pi<float>() / stackCount;
		float sectorAngle, stackAngle;

		for (int i = 0; i <= stackCount; ++i)
		{
			stackAngle = glm::pi<float>() / 2 - i * stackStep;        // starting from pi/2 to -pi/2
			xy = radius * cosf(stackAngle);             // r * cos(u)
			z = radius * sinf(stackAngle);              // r * sin(u)

			// add (sectorCount+1) vertices per stack
			// first and last vertices have same position and normal, but different tex coords
			for (int j = 0; j <= sectorCount; ++j)
			{
				sectorAngle = j * sectorStep;           // starting from 0 to 2pi

				// vertex position (x, y, z)
				x = xy * cosf(sectorAngle);             // r * cos(u) * cos(v)
				y = xy * sinf(sectorAngle);             // r * cos(u) * sin(v)
				vertices.push_back(x);
				vertices.push_back(y);
				vertices.push_back(z);

				// normalized vertex normal (nx, ny, nz)
				nx = x * lengthInv;
				ny = y * lengthInv;
				nz = z * lengthInv;
				normals.push_back(nx);
				normals.push_back(ny);
				normals.push_back(nz);

			}
		}

		for (int i = 0; i < stackCount; ++i)
		{
			int k1 = i * (sectorCount + 1);     // beginning of current stack
			int k2 = k1 + sectorCount + 1;      // beginning of next stack

			for (int j = 0; j < sectorCount; ++j, ++k1, ++k2)
			{
				// 2 triangles per quad (not for top and bottom stacks)
				if (i != 0) {
					indices.push_back(k1);
					indices.push_back(k2);
					indices.push_back(k1 + 1);
				}

				if (i != (stackCount - 1)) {
					indices.push_back(k1 + 1);
					indices.push_back(k2);
					indices.push_back(k2 + 1);
				}
			}
		}


		userBrushSphere->SetupMeshComponent(EShaderOption::lit, vertices, normals, indices);
		userBrushSphere->GetMeshComponent().lock()->SetObjectColor(glm::vec3(0.5f, 0.5f, 1.0f));
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
				if(ImGui::Button("Switch Shading Mode"))
				{
					//Flip shading flag
					settings.bShouldFlatShade = !settings.bShouldFlatShade;

					//Update the mesh shader 
					terrainSDFComponent.lock()->SetShouldRegenerateMesh(true);
				}
				//ImGui::Checkbox("Enable SDF Mesh Rendering", &settings.bViewMesh);
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

			//Only show begin editing option if app state is currently modelling
			if (m_currentAppState == EAppState::Modelling && ImGui::Button("Begin Editing"))
			{
				m_currentAppState = EAppState::Editing;
			}

			if (m_currentAppState == EAppState::Editing)
			{
				ImGui::Text("Distance to Brush Depth Plane");
				ImGui::InputFloat(":", &distanceToUserBrushPlane, 0.25f, 1.0f);
			}


			ImGui::End();
			
		}

		//~~ Handle Rendering ~~

		//Render the terrain
		{
			//Render dual contouring vertices
			dualContouring.DebugDrawVertices(m_terrainActor->GetVertices(), m_currentCamera, settings);

			//Regenerate mesh behavior based on app state
			if (m_currentAppState == EAppState::Modelling)
			{
				//If any changes occur in the SDF, regenerate the mesh
				if (!terrainSDFComponent.expired() && terrainSDFComponent.lock()->GetShouldRegenerateMesh())
				{
					//Generate the mesh based on the new SDF
					dualContouring.InitGenerateMesh(terrainVertices, terrainNormals, terrainIndices, terrainDebugColors, terrainSDFComponent, settings);

					//Set up the mesh component after generating the mesh
					m_terrainActor->SetupMeshComponent((settings.bShouldFlatShade ? EShaderOption::flat_shade : EShaderOption::lit), terrainVertices, terrainNormals, terrainIndices, terrainDebugColors);
					//Set object color
					m_terrainActor->GetMeshComponent().lock()->SetObjectColor(glm::vec3(0.5f, 1.0f, 0.75f));

					//Unset flag to regenerate mesh
					terrainSDFComponent.lock()->SetShouldRegenerateMesh(false);
				}

				//Render the terrain mesh
				m_terrainActor->Render();

			} else if (m_currentAppState == EAppState::Editing)
			{

				//Utilize the voxel field to edit the mesh

				//Setup a visual helper in the Z-axis of the camera used for getting point of brush
				{
					
					glm::vec3 directionToCamera = glm::normalize(m_currentCamera->GetCameraWorldPosition() - m_userBrushDepthPlane->GetWorldPosition());

					// +Y is the model's "forward" direction
					glm::vec3 forward = directionToCamera; // model's +Y points here
					glm::vec3 right = glm::normalize(glm::cross(glm::vec3(0, 1, 0), forward)); // model's +X
					glm::vec3 up = glm::normalize(glm::cross(forward, right)); // model's +Z (orthogonal up)

					glm::mat4 rotationMatrix(1.0f);
					rotationMatrix[0] = glm::vec4(right, 0.0f);   // X-axis
					rotationMatrix[1] = glm::vec4(forward, 0.0f); // Y-axis (your normal)
					rotationMatrix[2] = glm::vec4(up, 0.0f);      // Z-axis
					rotationMatrix[3] = glm::vec4(0, 0, 0, 1);

					//Plane always faces the camera
					m_userBrushDepthPlane->SetWorldRotation(rotationMatrix);

					//Plane always stays in front of the camera at a distance 'X'
					m_userBrushDepthPlane->SetWorldPosition(m_currentCamera->GetCameraWorldPosition() + (m_currentCamera->GetCameraForwardDirVector() * distanceToUserBrushPlane));

					
				}

				//Setup and render a spherical brush
				{

					if (settings.bIsCursorEnabled)
					{
						glm::vec2 ndcCoords = GetCursorPosNDC(window);
						m_userBrushRaycastResult = RaycastForBrushPlane(ndcCoords.x, ndcCoords.y);

						if (m_userBrushRaycastResult.bHit)
						{
							userBrushSphere->SetWorldPosition(m_userBrushRaycastResult.hitWorldPos);
							userBrushSphere->Render();
						}

						if (m_sphereBrush.bUpdateSDF == true)
						{
							m_sphereBrush.bUpdateSDF = false;

							//Update voxel field based on brush
							dualContouring.ApplyBrushToVoxels(1.f, userBrushSphere->GetWorldPosition());

							//Update the mesh based on the updated field
							dualContouring.UpdateMesh(terrainVertices, terrainNormals, terrainIndices, terrainDebugColors, settings);

							//Set up the mesh component after generating the mesh
							m_terrainActor->SetupMeshComponent((settings.bShouldFlatShade ? EShaderOption::flat_shade : EShaderOption::lit), terrainVertices, terrainNormals, terrainIndices, terrainDebugColors);
							//Set object color
							m_terrainActor->GetMeshComponent().lock()->SetObjectColor(glm::vec3(0.5f, 1.0f, 0.75f));
						}

						//Otherwise, if any changes occur in the regenerate the mesh (such as switching between shading model)
						if (!terrainSDFComponent.expired() && terrainSDFComponent.lock()->GetShouldRegenerateMesh())
						{
							//Update the mesh based on the updated field
							dualContouring.UpdateMesh(terrainVertices, terrainNormals, terrainIndices, terrainDebugColors, settings);

							//Set up the mesh component after generating the mesh
							m_terrainActor->SetupMeshComponent((settings.bShouldFlatShade ? EShaderOption::flat_shade : EShaderOption::lit), terrainVertices, terrainNormals, terrainIndices, terrainDebugColors);
							//Set object color
							m_terrainActor->GetMeshComponent().lock()->SetObjectColor(glm::vec3(0.5f, 1.0f, 0.75f));


							//Unset flag to regenerate mesh
							terrainSDFComponent.lock()->SetShouldRegenerateMesh(false);
						}
					}

				}

				//Render the terrain mesh
				m_terrainActor->Render();
				//Render transparent object last
				m_userBrushDepthPlane->Render();

			}
			
		}


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

RayCastResult App::RaycastForBrushPlane(double xPos, double yPos)
{
	//std::cout << "Current cursor x and y pos:" << xPos<<","<<yPos<<"\n";

	RayCastResult hitResult;

	glm::vec4 rayClipSpace = glm::vec4(xPos, yPos, -1.0, 1.0);

	glm::vec4 rayViewSpace = glm::inverse(m_currentCamera->GetProjectionMatrix()) * rayClipSpace;
	rayViewSpace = glm::vec4(rayViewSpace.x, rayViewSpace.y, -1.0, 0.0);

	glm::vec3 rayWorldSpace = glm::normalize(glm::inverse(m_currentCamera->GetViewMatrix()) * rayViewSpace);

	//Ray - plane intersection
	float denom = glm::dot(-m_currentCamera->GetCameraForwardDirVector(), rayWorldSpace);

	glm::vec3 planePoint = m_currentCamera->GetCameraWorldPosition() + m_currentCamera->GetCameraForwardDirVector() * distanceToUserBrushPlane;

	if (glm::abs(denom) > 1e-6f) {
		float t = glm::dot((planePoint - m_currentCamera->GetCameraWorldPosition()), -m_currentCamera->GetCameraForwardDirVector()) / denom;
		hitResult.hitWorldPos = m_currentCamera->GetCameraWorldPosition() + t * rayWorldSpace;

		// Convert hitPoint to local space and scale it
		glm::vec3 localHit = glm::vec3(glm::inverse(m_userBrushDepthPlane->GetModelMatrix()) * glm::vec4(hitResult.hitWorldPos, 1.0f));

		/*std::cout << "Local Hit Point: " << localHit.x << ", " << localHit.y << ", " << localHit.z << "\n";
		std::cout << "Camera Position: " << m_currentCamera->GetCameraWorldPosition().x << ", " << m_currentCamera->GetCameraWorldPosition().y << ", " << m_currentCamera->GetCameraWorldPosition().z << "\n\n";
		std::cout << "Plane Size:" << planeSize.x << ", " << planeSize.y << "\n";*/

		// Check if hit is inside bounds
		if (glm::abs(localHit.x) <= 0.5f && glm::abs(localHit.z) <= 0.5f) {

			//std::cout << "Raycast hit the plane!\n";
			//std::cout << "Global Hit Point: " << hitResult.hitWorldPos.x << ", " << hitResult.hitWorldPos.y << ", " << hitResult.hitWorldPos.z << "\n";
	
			hitResult.bHit = true;
		}
		else
		{
			//std::cout << "Raycast did not hit the plane!\n\n";
		}

		//std::cout << "\n";

	}

	return hitResult;
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
		}
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		{
			m_currentCamera->ProcessKeyboardInput(ECameraMoveDirection::BACKWARD, deltaTime);
		}
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		{
			m_currentCamera->ProcessKeyboardInput(ECameraMoveDirection::LEFT, deltaTime);
		}
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		{
			m_currentCamera->ProcessKeyboardInput(ECameraMoveDirection::RIGHT, deltaTime);
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

void App::MouseClickCallback(GLFWwindow* window, int button, int action, int mods)
{
	//Get app pointer
	App* appPtr = static_cast<App*>(glfwGetWindowUserPointer(window));

	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
	{
		//While editing, if there is a raycast hit and the user left-clicked, add to the SDF accordingly
		if (appPtr->m_currentAppState == EAppState::Editing && appPtr->m_userBrushRaycastResult.bHit)
		{
			//Flag updating the SDF
			appPtr->m_sphereBrush.bUpdateSDF = true;
		}


	}
}

glm::vec2 App::GetCursorPosNDC(GLFWwindow* window)
{
	//Poll the cursor position
	double xpos, ypos;
	glfwGetCursorPos(window, &xpos, &ypos);

	//Normalize the cursor position to normalized device coordinates range (-1 : 1)
	double n_xPos = xpos;
	double n_yPos = ypos;

	int width, height;
	glfwGetWindowSize(window, &width, &height);

	n_xPos = ((n_xPos * 2) / width) - 1.0;

	//To ensure range is from [1: -1] as y axis is positive at the top
	n_yPos = 1.0 - ((n_yPos * 2) / height);

	return { n_xPos, n_yPos };
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
