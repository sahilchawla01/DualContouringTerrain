#include "DualContouring.h"

#include <array>
#include <iostream>
#include <unordered_map>
#include <glad/glad.h>
#include <Helpers/Settings.h>
#include <Actors/ACamera.h>
#include "Shader.h"
#include "Brushes/SphereBrush.h"
#include "Components/USDFComponent.h"
#include "Enums/AppEnums.h"
#include "Math/QEFSolver.h"
#include "Math/RNG.h"
#include "Math/SDF.h"


DualContouring::DualContouring(const unsigned int& gridWidth, const unsigned int& gridHeight,
	const unsigned int& gridDepth, const float& voxelSize)
{
	this->m_gridWidth = gridWidth;
	this->m_gridHeight= gridHeight;
	this->m_gridDepth = gridDepth;
	this->m_voxelResolution = voxelSize;
}

DualContouring::~DualContouring()
= default;


const std::vector<glm::vec3> DualContouring::voxelCornerOffsets =
{
	//Lower cube vertices
	glm::vec3(0, 0, 0),
	glm::vec3(1, 0, 0),
	glm::vec3(1, 0, 1),
	glm::vec3(0, 0, 1),

	//Upper cube vertices
	glm::vec3(0, 1, 0),
	glm::vec3(1, 1, 0),
	glm::vec3(1, 1, 1),
	glm::vec3(0, 1, 1),

};

const std::vector<std::pair<int, int>> DualContouring::edgePairs =
{
		std::make_pair(0, 1), // 0 
		std::make_pair(1, 2),
		std::make_pair(2, 3),
		std::make_pair(0, 3), // 3
		std::make_pair(4, 5),
		std::make_pair(5, 6),
		std::make_pair(6, 7),
		std::make_pair(7, 4),
		std::make_pair(0, 4), // 8
		std::make_pair(1, 5),
		std::make_pair(2, 6),
		std::make_pair(3, 7)
};

const std::vector<std::vector<glm::vec3>> DualContouring::adjacentVoxelsOffsets =
{
	//Front-Bottom Horizontal edge adjacent voxels
	{

			glm::vec3(0.f, 0.f, 0.f),
			glm::vec3(0.f, -1.f, 0.f),
			glm::vec3(0.f, 0.f, -1.f),
			glm::vec3(0.f, -1.f, -1.f),

	},
	  //Left-Bottom most edge adjacent voxels
  {

		  glm::vec3(-1.f, 0.f, 0.f),
		  glm::vec3(-1.f, -1.f, 0.f),
		  glm::vec3(0.f, 0.f, 0.f),
		  glm::vec3(0.f, -1.f, 0.f),
  },
	//Front most left vertical edge adjacent voxels
 {
		glm::vec3(-1.f, 0.f, -1.f),
		glm::vec3(-1.f, 0.f, 0.f),
		glm::vec3(0.f, 0.f, -1.f),
		glm::vec3(0.f, 0.f, 0.f),
	}
};


const glm::vec3 DualContouring::GetIntersectionPoint(const glm::vec3& firstPosition, const glm::vec3& secondPosition, const glm::vec3& spherePosition, const float& sphereRadius, int totalSteps)
{
	glm::vec3 direction = glm::normalize(secondPosition - firstPosition);
	const float step = 1 / static_cast<float>(totalSteps);

	//float minValue = 100000.f;

	float finalT = 0.f;
	//float currentT = 0.f;

	//while(currentT <= 1.f)
	//{
	//	//TODO: Change it to a generic SDF function
	//	const glm::vec3 currentPos = firstPosition + (direction * currentT);
	//	const float density = SDF::GetSphereSDFValue(currentPos, spherePosition, sphereRadius);

	//	if(density < minValue)
	//	{
	//		minValue = density;
	//		finalT = currentT;
	//	}

	//	currentT += step;
	//}

	return firstPosition + (direction * finalT);

}

const glm::vec3 DualContouring::CalculateSurfaceNormal(const glm::vec3& intersectionPos, const std::weak_ptr<USDFComponent> actorSdfComponent)
{
	//Perform Finite Sum Difference
	const float h = 0.001f;

	const float dx =
		actorSdfComponent.lock()->EvaluateSDF(intersectionPos + glm::vec3(h, 0.f, 0.f)) -
		actorSdfComponent.lock()->EvaluateSDF(intersectionPos - glm::vec3(h, 0.f, 0.f));
	const float dy =
		actorSdfComponent.lock()->EvaluateSDF(intersectionPos + glm::vec3(0.f, h, 0.f)) -
		actorSdfComponent.lock()->EvaluateSDF(intersectionPos - glm::vec3(0.f, h, 0.f));
	const float dz =
		actorSdfComponent.lock()->EvaluateSDF(intersectionPos + glm::vec3(0.f, 0.f, h)) -
		actorSdfComponent.lock()->EvaluateSDF(intersectionPos - glm::vec3(0.f, 0.f, h));

	return glm::normalize(glm::vec3(dx, dy, dz));
}

void DualContouring::DebugDrawVertices(const std::vector<float>& vertices, std::weak_ptr<ACamera> curCamera, const Settings& settings)
{
	//DEBUG: Spawn cube at vertex positions
	if (settings.bIsDebugEnabled)
	{

		float cubeVertices[] = {
	  -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
	   0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
	   0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
	   0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
	  -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
	  -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

	  -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
	   0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
	   0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
	   0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
	  -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
	  -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

	  -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
	  -0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
	  -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
	  -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
	  -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
	  -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

	   0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
	   0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
	   0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
	   0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
	   0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
	   0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

	  -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
	   0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
	   0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
	   0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
	  -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
	  -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

	  -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
	   0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
	   0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
	   0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
	  -0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
	  -0.5f,  0.5f, -0.5f,  0.0f, 1.0f
		};
		Shader unlitShader("../../../src/Shaders/Test/test.vert", "../../../src/Shaders/Test/test.frag");

		//Generate vertex array and vertex buffer for triangle render
		unsigned int VBO, VAO;
		//Generate Vertex Array Object
		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);

		//Bind VAO
		glBindVertexArray(VAO);

		// 0. copy our cubeVertices array in a buffer for OpenGL to use
		//Binds a buffer object to the current buffer type, only 1 can be set at one time
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		//Copy data to the buffer
		glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);
		// 2. then set the vertex attributes pointers
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);

		unlitShader.use();

		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); // Normal shading
		//Draw vertex position
		for (size_t index = 0; index < vertices.size(); index += 3)
		{
			glm::vec3 vertexPos(vertices[index], vertices[index + 1], vertices[index + 2]);

			//Create MVP
			glm::mat4 model = glm::mat4(1.0f);
			model = glm::translate(model, vertexPos);
			model = glm::scale(model, glm::vec3(0.1f, 0.1f, 0.1f));

			glm::mat4 view = curCamera.lock()->GetViewMatrix();

			glm::mat4 projection = curCamera.lock()->GetProjectionMatrix();

			glm::mat4 mvp = projection * view * model;
			glm::vec3 debugCubeColor(0.027f, 0.843f, 1.0f);
			unlitShader.setMat4("mvp", mvp);
			unlitShader.setVec3("color", debugCubeColor);

			glDrawArrays(GL_TRIANGLES, 0, 36);
		}
	}
}

void DualContouring::InitGenerateMesh(std::vector<float>& vertices, std::vector<float>& normals,
	std::vector<unsigned int>& indices, std::vector<float>& colors, const std::weak_ptr<USDFComponent> actorSdfComponent, const Settings& settings)
{
	//Clear any index mapping data before generating mesh in case already generated the mesh
	ClearHashMapData();


	std::vector<float> modelVertices;
	std::vector<float> modelNormals;
	std::vector<unsigned int> modelIndices;
	std::vector<float> modelVertexColors;

	std::vector<float> modelDuplicateVertices;
	std::vector<float> modelDuplicateNormals;

	int expandedGridWidth = static_cast<int>(static_cast<float>(this->m_gridWidth) * (1 / this->m_voxelResolution));
	int expandedGridHeight = static_cast<int>(static_cast<float>(this->m_gridHeight) * (1 / this->m_voxelResolution));
	int expandedGridDepth = static_cast<int>(static_cast<float>(this->m_gridDepth) * (1 / this->m_voxelResolution));


	glm::vec3 gridPosition(0.f, 0.f, 0.f);

	glm::mat4 gridModelMatrix(1.f);
	gridModelMatrix = glm::translate(gridModelMatrix, gridPosition);

	glm::vec3 gridCenter((this->m_gridWidth) / 2, (this->m_gridHeight) / 2, (this->m_gridDepth) / 2);

	//Generate vertex positions
	for (int x = 0; x < expandedGridWidth; x++)
	{
		for (int y = 0; y < expandedGridHeight; y++)
		{
			for (int z = 0; z < expandedGridDepth; z++)
			{
				//Get relative position to grid position
				glm::vec3 relativePos((static_cast<float>(x) * this->m_voxelResolution), (static_cast<float>(y) * this->m_voxelResolution), (static_cast<float>(z) * this->m_voxelResolution));

				//Relative to grid center
				relativePos -= gridCenter;

				//Relative to grid position
				relativePos += gridPosition;

				HermiteData defaultHermiteData{ glm::vec3(0), glm::vec3(1, 0, 0), std::numeric_limits<float>::min(), false };

				//Array describing hermite data for 8 corners per voxel
				std::array<HermiteData, 8> voxelCornersHermiteData
				{
					{
						defaultHermiteData,
						defaultHermiteData,
						defaultHermiteData,
						defaultHermiteData,
						defaultHermiteData,
						defaultHermiteData,
						defaultHermiteData,
						defaultHermiteData,
					}
				};

				//Go over each corner
				{
					int cornersToConsider = 0;
					std::array<float, 8> cornerSDFValues;

					for (int i = 0; i < 8; ++i)
					{
						//Get current corner's position in world space
						glm::vec3 currentCornerPos = (voxelCornerOffsets[i] * this->m_voxelResolution) + relativePos;

						//Calculate signed distance of current corner
						float distanceValue = actorSdfComponent.lock()->EvaluateSDF(currentCornerPos);
						cornerSDFValues[i] = distanceValue;

						//Store corner hermite data 
						HermiteData cornerHermiteData;
						cornerHermiteData.distance = distanceValue;
						cornerHermiteData.position = currentCornerPos;
						cornerHermiteData.normal = CalculateSurfaceNormal(currentCornerPos, actorSdfComponent);

						voxelCornersHermiteData[i] = cornerHermiteData;


						//TODO: convert position from grid relative to SDF center relative
						//If within the surface, consider for triangulation
						if (distanceValue <= 0.f)
						{
							cornersToConsider |= 1 << i;
						}
					}

					//Store the corner hermite data in a map
					voxelToCornerHermiteDataMap[GetUniqueIndexForGrid(x, y, z, expandedGridWidth, expandedGridHeight)] = voxelCornersHermiteData;

					//If the voxel is completely within the surface, or outside the volume, ignore it.
					if (cornersToConsider == 0 || cornersToConsider == 255)
						continue;

					std::vector<glm::vec3> intersectionPoints;
					std::vector<glm::vec3> intersectionNormals;

					//Array describing hermite data for 3 edges per voxel
					std::array<HermiteData, 3> adjacentEdgeHermiteData
					{
						{
							defaultHermiteData,
							defaultHermiteData,
							defaultHermiteData,
						}
					};

					//Vector containing hermite data for all 12 edges per voxel, used for computing vertex position
					std::vector<HermiteData> allEdgeHermiteData;

					//If none of the 3 adjacent edges have an intersection, set flag. By default true.
					bool bNoIntersectionFor3AdjacentEdges = true;

					for (int i = 0; i < 12; ++i)
					{
						const int cornerIndex1 = edgePairs[i].first;
						const int cornerIndex2 = edgePairs[i].second;


						const int m1 = (cornersToConsider >> cornerIndex1) & 1;
						const int m2 = (cornersToConsider >> cornerIndex2) & 1;

						//This means that the edge has no crossing over from one sign to the other, skip.
						if (m1 == m2)
						{
							continue;
						}


						//Find position along the edge where surface crosses signs

						//TODO: convert position from grid relative to SDF center relative
						const glm::vec3 cornerPos1 = (voxelCornerOffsets[cornerIndex1] * (this->m_voxelResolution)) + relativePos;
						const glm::vec3 cornerPos2 = (voxelCornerOffsets[cornerIndex2] * (this->m_voxelResolution)) + relativePos;

						//Get current intersection point by using linear interpolation
						float interpolateFactor = abs(cornerSDFValues[cornerIndex1]) / (abs(cornerSDFValues[cornerIndex1]) + abs(cornerSDFValues[cornerIndex2]));
						interpolateFactor = glm::clamp(interpolateFactor, 0.0f, 1.0f);

						glm::vec3 currIntersectionPoint = cornerPos1 + ((cornerPos2 - cornerPos1) * interpolateFactor);

						intersectionPoints.push_back(currIntersectionPoint);

					
						//TODO: SAnity check whether point is within voxel

						//Calculate normal using Finite Sum Difference
						const glm::vec3 intersectionNormal = CalculateSurfaceNormal(currIntersectionPoint, actorSdfComponent);
						intersectionNormals.push_back(intersectionNormal);

						//Crossing over has occured, check if edge is one of the 3 adjacent left most corner ones, and set the value.
						if (i == 0)
						{
							////Mark intersection occurred 

							adjacentEdgeHermiteData[0].distance = actorSdfComponent.lock()->EvaluateSDF(currIntersectionPoint);
							adjacentEdgeHermiteData[0].normal = intersectionNormal;
							adjacentEdgeHermiteData[0].position = currIntersectionPoint;
							adjacentEdgeHermiteData[0].bIntersecPosToNeg = (m1 < m2);

							bNoIntersectionFor3AdjacentEdges = false;

						}
						else if (i == 3)
						{
							//Mark intersection occurred 

							adjacentEdgeHermiteData[1].distance = actorSdfComponent.lock()->EvaluateSDF(currIntersectionPoint);
							adjacentEdgeHermiteData[1].normal = intersectionNormal;
							adjacentEdgeHermiteData[1].position = currIntersectionPoint;
							adjacentEdgeHermiteData[1].bIntersecPosToNeg = (m1 < m2);

							bNoIntersectionFor3AdjacentEdges = false;

						}
						else if (i == 8)
						{
							//Mark intersection occurred 

							adjacentEdgeHermiteData[2].distance = actorSdfComponent.lock()->EvaluateSDF(currIntersectionPoint);
							adjacentEdgeHermiteData[2].normal = intersectionNormal;
							adjacentEdgeHermiteData[2].position = currIntersectionPoint;
							adjacentEdgeHermiteData[2].bIntersecPosToNeg = (m1 < m2);

							bNoIntersectionFor3AdjacentEdges = false;

						}


						//Store hermite info of an edge
						allEdgeHermiteData.push_back(
							{ currIntersectionPoint, intersectionNormal, actorSdfComponent.lock()->EvaluateSDF(currIntersectionPoint), (m1 < m2)
							}
						);

					}

					//In the case there is an intersection for any of the 3 adjacent edges, map the voxel to the adjacent edges, else don't.
					if (!bNoIntersectionFor3AdjacentEdges)
					{
						//Map voxel to adjacent edges hermite data array
						voxelToEdgesHermiteDataMap[GetUniqueIndexForGrid(x, y, z, expandedGridWidth, expandedGridHeight)] = adjacentEdgeHermiteData;
					}

					//Calculate the best vertex using Quadratic error function
					glm::vec3 vertexPos(0.f);
					vertexPos = QEFSolver::ComputeBestVertexPosition(allEdgeHermiteData);

					//Calculate centroid of intersection normals
					glm::vec3 vertexNormal(0.f);
					for (const glm::vec3& normal : intersectionNormals)
						vertexNormal += normal;

					vertexNormal = glm::normalize(vertexNormal);

					if (allEdgeHermiteData.empty())
					{
						std::cout << ".";
					}

					//SANITY CHECK: CHECK IF CURRENT UNIQUE ID HAS ALREADY BEEN SET FOR VOXEL-VERTEX MAP
					if (voxelVertexIndexMap.find(GetUniqueIndexForGrid(x, y, z, expandedGridWidth, expandedGridHeight)) != voxelVertexIndexMap.end())
					{
						std::cout << "ERROR: THIS UNIQUE ID HAS ALREADY BEEN SET\n";
					}

					//Map voxel to vertex array index position
					voxelVertexIndexMap[GetUniqueIndexForGrid(x, y, z, expandedGridWidth, expandedGridHeight)] = static_cast<int>(modelVertices.size());

					//std::cout << "New Vertex: " << modelVertices.size() / 3<<", ";

					//Store vertex position relative to grid space
					modelVertices.push_back(vertexPos.x);
					modelVertices.push_back(vertexPos.y);
					modelVertices.push_back(vertexPos.z);

					//std::cout << "Vertex added, model vertices size" << modelVertices.size();

					//Store model normals
					modelNormals.push_back(vertexNormal.x);
					modelNormals.push_back(vertexNormal.y);
					modelNormals.push_back(vertexNormal.z);


				}
			}
		}
	}


	//Iterate through the cubes again, and make the edge connections
	for (int x = 0; x < expandedGridWidth; x++)
	{
		for (int y = 0; y < expandedGridHeight; y++)
		{
			for (int z = 0; z < expandedGridDepth; z++)
			{
				//Check if there is no mapping, then no intersections for that voxel's 3 adjacent edges. Skip.
				if (voxelToEdgesHermiteDataMap.find(GetUniqueIndexForGrid(x, y, z, expandedGridWidth, expandedGridHeight)) == voxelToEdgesHermiteDataMap.end())
				{
					continue;
				}


				const std::array<HermiteData, 3> adjacentEdgesIntersection = voxelToEdgesHermiteDataMap[
					GetUniqueIndexForGrid(x, y, z, expandedGridWidth, expandedGridHeight)];


				//Iterate over adjacent edges and make connections where possible
				for (int axis = 0; axis < 3; ++axis)
				{
					//For an edge, check if an intersection exists and all 4 neighboring voxels are valid
					if (adjacentEdgesIntersection[axis].distance != std::numeric_limits<float>::min() && ((x - 1) > 0 && (y - 1) > 0 && (z - 1) > 0))
					{

						std::vector<unsigned int> vertexIndices;
						std::vector<unsigned int> actualVertexIndices;

						//Store vertex indices for neighboring voxels
						for (int i = 0; i < 4; ++i)
						{
							int curX = x + static_cast<int>(adjacentVoxelsOffsets[axis][i].x);
							int curY = y + static_cast<int>(adjacentVoxelsOffsets[axis][i].y);
							int curZ = z + static_cast<int>(adjacentVoxelsOffsets[axis][i].z);

							auto it = voxelVertexIndexMap.find(GetUniqueIndexForGrid(curX, curY, curZ, expandedGridWidth, expandedGridHeight));

							if (it == voxelVertexIndexMap.end())
								continue;  // If voxel is missing, just skip it

							// Store the found vertex index
							vertexIndices.push_back(it->second / 3);
							actualVertexIndices.push_back(it->second);
						}


						// If we have fewer than 4 valid neighbors, we cannot form a face
						if (vertexIndices.size() <= 3)
						{
							continue;
						}
						//Join all 4 vertices in those voxels
						if (vertexIndices.size() == 4)
						{

							//If the transition is from + to -ve 
							if (adjacentEdgesIntersection[axis].bIntersecPosToNeg == true)
							{

								//Triangle 1

								// enable duplicate vertices || //Used for glDrawArrays rather than glDrawElements
								if (settings.bShouldFlatShade)
								{
									//Pos 1 (pairs of 3 floats i.e. a 3D vector)
									modelDuplicateVertices.push_back(modelVertices[actualVertexIndices[1]]);
									modelDuplicateVertices.push_back(modelVertices[actualVertexIndices[1] + 1]);
									modelDuplicateVertices.push_back(modelVertices[actualVertexIndices[1] + 2]);

									//Pos 2
									modelDuplicateVertices.push_back(modelVertices[actualVertexIndices[3]]);
									modelDuplicateVertices.push_back(modelVertices[actualVertexIndices[3] + 1]);
									modelDuplicateVertices.push_back(modelVertices[actualVertexIndices[3] + 2]);

									//Pos 3
									modelDuplicateVertices.push_back(modelVertices[actualVertexIndices[2]]);
									modelDuplicateVertices.push_back(modelVertices[actualVertexIndices[2] + 1]);
									modelDuplicateVertices.push_back(modelVertices[actualVertexIndices[2] + 2]);

									//Similarly, push duplicate normals
									//Normal 1
									//modelDuplicateNormals.push_back(modelNormals[actualVertexIndices[1]]);
									//modelDuplicateNormals.push_back(modelNormals[actualVertexIndices[1] + 1]);
									//modelDuplicateNormals.push_back(modelNormals[actualVertexIndices[1] + 2]);

									////Normal 2
									//modelDuplicateNormals.push_back(modelNormals[actualVertexIndices[3]]);
									//modelDuplicateNormals.push_back(modelNormals[actualVertexIndices[3] + 1]);
									//modelDuplicateNormals.push_back(modelNormals[actualVertexIndices[3] + 2]);

									////Normal 3
									//modelDuplicateNormals.push_back(modelNormals[actualVertexIndices[2]]);
									//modelDuplicateNormals.push_back(modelNormals[actualVertexIndices[2] + 1]);
									//modelDuplicateNormals.push_back(modelNormals[actualVertexIndices[2] + 2]);


									//Triangle 1 Color
									float triangleColorR = RNG::GetRandomFloatNumber(0.0f, 1.0f);
									float triangleColorG = RNG::GetRandomFloatNumber(0.0f, 1.0f);
									float triangleColorB = RNG::GetRandomFloatNumber(0.0f, 1.0f);
									//Push 3 floats (vec3 color) per vertex i.e. 3 vertices
									for (int i = 0; i < 3; ++i)
									{
										modelVertexColors.push_back(triangleColorR);
										modelVertexColors.push_back(triangleColorG);
										modelVertexColors.push_back(triangleColorB);
										
									}

								}
								else // utilize indexing to render vertices
								{
									modelIndices.push_back(vertexIndices[1]);
									modelIndices.push_back(vertexIndices[3]);
									modelIndices.push_back(vertexIndices[2]);
								}


								//Triangle 2

								/*modelIndices.push_back(vertexIndices[0]);
								modelIndices.push_back(vertexIndices[1]);
								modelIndices.push_back(vertexIndices[2]);*/
								/*	modelDuplicateVertices.push_back(modelVertices[vertexIndices[0]]);
									modelDuplicateVertices.push_back(modelVertices[vertexIndices[1]]);
									modelDuplicateVertices.push_back(modelVertices[vertexIndices[2]]);*/

									// enable duplicate vertices || //Used for glDrawArrays rather than glDrawElements
								if (settings.bShouldFlatShade)
								{

									//Pos 1 (pairs of 3 floats i.e. a 3D vector)
									modelDuplicateVertices.push_back(modelVertices[actualVertexIndices[0]]);
									modelDuplicateVertices.push_back(modelVertices[actualVertexIndices[0] + 1]);
									modelDuplicateVertices.push_back(modelVertices[actualVertexIndices[0] + 2]);

									//Pos 2
									modelDuplicateVertices.push_back(modelVertices[actualVertexIndices[1]]);
									modelDuplicateVertices.push_back(modelVertices[actualVertexIndices[1] + 1]);
									modelDuplicateVertices.push_back(modelVertices[actualVertexIndices[1] + 2]);

									//Pos 3
									modelDuplicateVertices.push_back(modelVertices[actualVertexIndices[2]]);
									modelDuplicateVertices.push_back(modelVertices[actualVertexIndices[2] + 1]);
									modelDuplicateVertices.push_back(modelVertices[actualVertexIndices[2] + 2]);

									//Similarly, push duplicate normals
									//Normal 1
									//modelDuplicateNormals.push_back(modelNormals[actualVertexIndices[0]]);
									//modelDuplicateNormals.push_back(modelNormals[actualVertexIndices[0] + 1]);
									//modelDuplicateNormals.push_back(modelNormals[actualVertexIndices[0] + 2]);

									////Normal 2
									//modelDuplicateNormals.push_back(modelNormals[actualVertexIndices[1]]);
									//modelDuplicateNormals.push_back(modelNormals[actualVertexIndices[1] + 1]);
									//modelDuplicateNormals.push_back(modelNormals[actualVertexIndices[1] + 2]);

									////Normal 3
									//modelDuplicateNormals.push_back(modelNormals[actualVertexIndices[2]]);
									//modelDuplicateNormals.push_back(modelNormals[actualVertexIndices[2] + 1]);
									//modelDuplicateNormals.push_back(modelNormals[actualVertexIndices[2] + 2]);

									float triangleColorR = RNG::GetRandomFloatNumber(0.0f, 1.0f);
									float triangleColorG = RNG::GetRandomFloatNumber(0.0f, 1.0f);
									float triangleColorB = RNG::GetRandomFloatNumber(0.0f, 1.0f);
									//Push 3 floats (vec3 color) per vertex i.e. 3 vertices
									for (int i = 0; i < 3; ++i)
									{
										modelVertexColors.push_back(triangleColorR);
										modelVertexColors.push_back(triangleColorG);
										modelVertexColors.push_back(triangleColorB);

									}

								}
								else // utilize indexing to render vertices
								{
									modelIndices.push_back(vertexIndices[0]);
									modelIndices.push_back(vertexIndices[1]);
									modelIndices.push_back(vertexIndices[2]);
								}

							}
							else //Reverse indices order otherwise (-ve to +ve transition)
							{
								////Triangle 1
								//modelIndices.push_back(vertexIndices[1]);
								//modelIndices.push_back(vertexIndices[2]);
								//modelIndices.push_back(vertexIndices[3]);

								//modelDuplicateVertices.push_back(modelVertices[vertexIndices[1]]);/*
								//modelDuplicateVertices.push_back(modelVertices[vertexIndices[2]]);
								//modelDuplicateVertices.push_back(modelVertices[vertexIndices[3]]);*/

								// enable duplicate vertices || //Used for glDrawArrays rather than glDrawElements
								if (settings.bShouldFlatShade)
								{

									//Pos 1 (pairs of 3 floats i.e. a 3D vector)
									modelDuplicateVertices.push_back(modelVertices[actualVertexIndices[1]]);
									modelDuplicateVertices.push_back(modelVertices[actualVertexIndices[1] + 1]);
									modelDuplicateVertices.push_back(modelVertices[actualVertexIndices[1] + 2]);

									//Pos 2
									modelDuplicateVertices.push_back(modelVertices[actualVertexIndices[2]]);
									modelDuplicateVertices.push_back(modelVertices[actualVertexIndices[2] + 1]);
									modelDuplicateVertices.push_back(modelVertices[actualVertexIndices[2] + 2]);

									//Pos 3
									modelDuplicateVertices.push_back(modelVertices[actualVertexIndices[3]]);
									modelDuplicateVertices.push_back(modelVertices[actualVertexIndices[3] + 1]);
									modelDuplicateVertices.push_back(modelVertices[actualVertexIndices[3] + 2]);

									//Similarly, push duplicate normals
									//Normal 1
									//modelDuplicateNormals.push_back(modelNormals[actualVertexIndices[1]]);
									//modelDuplicateNormals.push_back(modelNormals[actualVertexIndices[1] + 1]);
									//modelDuplicateNormals.push_back(modelNormals[actualVertexIndices[1] + 2]);

									////Normal 2
									//modelDuplicateNormals.push_back(modelNormals[actualVertexIndices[2]]);
									//modelDuplicateNormals.push_back(modelNormals[actualVertexIndices[2] + 1]);
									//modelDuplicateNormals.push_back(modelNormals[actualVertexIndices[2] + 2]);

									////Normal 3
									//modelDuplicateNormals.push_back(modelNormals[actualVertexIndices[3]]);
									//modelDuplicateNormals.push_back(modelNormals[actualVertexIndices[3] + 1]);
									//modelDuplicateNormals.push_back(modelNormals[actualVertexIndices[3] + 2]);

									float triangleColorR = RNG::GetRandomFloatNumber(0.0f, 1.0f);
									float triangleColorG = RNG::GetRandomFloatNumber(0.0f, 1.0f);
									float triangleColorB = RNG::GetRandomFloatNumber(0.0f, 1.0f);
									//Push 3 floats (vec3 color) per vertex i.e. 3 vertices
									for (int i = 0; i < 3; ++i)
									{
										modelVertexColors.push_back(triangleColorR);
										modelVertexColors.push_back(triangleColorG);
										modelVertexColors.push_back(triangleColorB);

									}


								}
								else // utilize indexing to render vertices
								{
									modelIndices.push_back(vertexIndices[1]);
									modelIndices.push_back(vertexIndices[2]);
									modelIndices.push_back(vertexIndices[3]);
								}

								//Triangle 2
								/*modelIndices.push_back(vertexIndices[0]);
								modelIndices.push_back(vertexIndices[2]);
								modelIndices.push_back(vertexIndices[1]);*/

								//modelDuplicateVertices.push_back(modelVertices[vertexIndices[0]]);/*
								//modelDuplicateVertices.push_back(modelVertices[vertexIndices[2]]);
								//modelDuplicateVertices.push_back(modelVertices[vertexIndices[1]]);*/

								// enable duplicate vertices || //Used for glDrawArrays rather than glDrawElements
								if (settings.bShouldFlatShade)
								{

									//Pos 1 (pairs of 3 floats i.e. a 3D vector)
									modelDuplicateVertices.push_back(modelVertices[actualVertexIndices[0]]);
									modelDuplicateVertices.push_back(modelVertices[actualVertexIndices[0] + 1]);
									modelDuplicateVertices.push_back(modelVertices[actualVertexIndices[0] + 2]);

									//Pos 2
									modelDuplicateVertices.push_back(modelVertices[actualVertexIndices[2]]);
									modelDuplicateVertices.push_back(modelVertices[actualVertexIndices[2] + 1]);
									modelDuplicateVertices.push_back(modelVertices[actualVertexIndices[2] + 2]);

									//Pos 3
									modelDuplicateVertices.push_back(modelVertices[actualVertexIndices[1]]);
									modelDuplicateVertices.push_back(modelVertices[actualVertexIndices[1] + 1]);
									modelDuplicateVertices.push_back(modelVertices[actualVertexIndices[1] + 2]);

									//Similarly, push duplicate normals
									//Normal 1
									//modelDuplicateNormals.push_back(modelNormals[actualVertexIndices[0]]);
									//modelDuplicateNormals.push_back(modelNormals[actualVertexIndices[0] + 1]);
									//modelDuplicateNormals.push_back(modelNormals[actualVertexIndices[0] + 2]);

									////Normal 2
									//modelDuplicateNormals.push_back(modelNormals[actualVertexIndices[2]]);
									//modelDuplicateNormals.push_back(modelNormals[actualVertexIndices[2] + 1]);
									//modelDuplicateNormals.push_back(modelNormals[actualVertexIndices[2] + 2]);

									////Normal 3
									//modelDuplicateNormals.push_back(modelNormals[actualVertexIndices[1]]);
									//modelDuplicateNormals.push_back(modelNormals[actualVertexIndices[1] + 1]);
									//modelDuplicateNormals.push_back(modelNormals[actualVertexIndices[1] + 2]);

									float triangleColorR = RNG::GetRandomFloatNumber(0.0f, 1.0f);
									float triangleColorG = RNG::GetRandomFloatNumber(0.0f, 1.0f);
									float triangleColorB = RNG::GetRandomFloatNumber(0.0f, 1.0f);
									//Push 3 floats (vec3 color) per vertex i.e. 3 vertices
									for (int i = 0; i < 3; ++i)
									{
										modelVertexColors.push_back(triangleColorR);
										modelVertexColors.push_back(triangleColorG);
										modelVertexColors.push_back(triangleColorB);

									}


								}
								else // utilize indexing to render vertices
								{
									modelIndices.push_back(vertexIndices[0]);
									modelIndices.push_back(vertexIndices[2]);
									modelIndices.push_back(vertexIndices[1]);
								}
							}

						}

					}
				}




			}
		}
	}

	//Finally assign the mesh details

	//Set vertices to duplicate mode or indices mode
	vertices = (settings.bShouldFlatShade) ? modelDuplicateVertices : modelVertices;
	normals = (settings.bShouldFlatShade) ? modelDuplicateNormals : modelNormals;
	indices = modelIndices;
	colors = modelVertexColors;


}

void DualContouring::UpdateMesh(std::vector<float>& vertices, std::vector<float>& normals,
	std::vector<unsigned int>& indices, std::vector<float>& colors, const Settings& settings)
{
	ClearHashMapData();

	std::vector<float> modelVertices;
	std::vector<float> modelNormals;
	std::vector<unsigned int> modelIndices;
	std::vector<float> modelVertexColors;

	std::vector<float> modelDuplicateVertices;
	std::vector<float> modelDuplicateNormals;

	int expandedGridWidth = static_cast<int>(static_cast<float>(this->m_gridWidth) * (1 / this->m_voxelResolution));
	int expandedGridHeight = static_cast<int>(static_cast<float>(this->m_gridHeight) * (1 / this->m_voxelResolution));
	int expandedGridDepth = static_cast<int>(static_cast<float>(this->m_gridDepth) * (1 / this->m_voxelResolution));


	glm::vec3 gridPosition(0.f, 0.f, 0.f);

	glm::mat4 gridModelMatrix(1.f);
	gridModelMatrix = glm::translate(gridModelMatrix, gridPosition);

	glm::vec3 gridCenter((this->m_gridWidth) / 2, (this->m_gridHeight) / 2, (this->m_gridDepth) / 2);

	//Generate vertex positions
	for (int x = 0; x < expandedGridWidth; x++)
	{
		for (int y = 0; y < expandedGridHeight; y++)
		{
			for (int z = 0; z < expandedGridDepth; z++)
			{
				//Get relative position to grid position
				glm::vec3 relativePos((static_cast<float>(x) * this->m_voxelResolution), (static_cast<float>(y) * this->m_voxelResolution), (static_cast<float>(z) * this->m_voxelResolution));

				//Relative to grid center
				relativePos -= gridCenter;

				//Relative to grid position
				relativePos += gridPosition;

				HermiteData defaultHermiteData{ glm::vec3(0), glm::vec3(1, 0, 0), std::numeric_limits<float>::min(), false };

				//Get hermite data for corners
				std::array < HermiteData, 8> voxelCornersHermiteData = voxelToCornerHermiteDataMap[GetUniqueIndexForGrid(x, y, z, expandedGridWidth, expandedGridHeight)];

				int cornersToConsider = 0;

				//Check if voxel is completely inside or outside the surface
				for (int idx = 0; idx < 8; ++idx)
				{
					//If within the surface, consider for triangulation
					if (voxelCornersHermiteData[idx].distance <= 0.f)
					{
						cornersToConsider |= 1 << idx;
					}
				}

				//Skip this voxel because it is completely inside/outside the surface
				if (cornersToConsider == 0 || cornersToConsider == 255)
					continue;


				std::vector<glm::vec3> intersectionPoints;
				std::vector<glm::vec3> intersectionNormals;

				//Array describing hermite data for 3 edges per voxel
				std::array<HermiteData, 3> adjacentEdgeHermiteData
				{
					{
						defaultHermiteData,
						defaultHermiteData,
						defaultHermiteData,
					}
				};

				//Vector containing hermite data for all 12 edges per voxel, used for computing vertex position
				std::vector<HermiteData> allEdgeHermiteData;

				//If none of the 3 adjacent edges have an intersection, set flag. By default true.
				bool bNoIntersectionFor3AdjacentEdges = true;

				for (int i = 0; i < 12; ++i)
				{
					const int cornerIndex1 = edgePairs[i].first;
					const int cornerIndex2 = edgePairs[i].second;

					const HermiteData corner1HermiteData = voxelCornersHermiteData[cornerIndex1];
					const HermiteData corner2HermiteData = voxelCornersHermiteData[cornerIndex2];

					//This means that the edge has no crossing over from one sign to the other, skip.
					if ((corner1HermiteData.distance > 0.f && corner2HermiteData.distance > 0.f) || (corner1HermiteData.distance < 0.f && corner2HermiteData.distance < 0.f))
					{
						continue;
					}

					float interpolateFactor = abs(corner1HermiteData.distance) / (abs(corner1HermiteData.distance) + abs(corner2HermiteData.distance));
					interpolateFactor = glm::clamp(interpolateFactor, 0.0f, 1.0f);

					glm::vec3 currIntersectionPoint = corner1HermiteData.position + ((corner2HermiteData.position - corner1HermiteData.position) * interpolateFactor);

					intersectionPoints.push_back(currIntersectionPoint);

					//Calculate normal by using linear interpolation
					const glm::vec3 intersectionNormal = glm::normalize(glm::mix(corner1HermiteData.normal, corner2HermiteData.normal, interpolateFactor));

					intersectionNormals.push_back(intersectionNormal);

					//Crossing over has occured, check if edge is one of the 3 adjacent left most corner ones, and set the value.
					if (i == 0)
					{
						////Mark intersection occurred 

						adjacentEdgeHermiteData[0].distance = 0.f;
						adjacentEdgeHermiteData[0].normal = intersectionNormal;
						adjacentEdgeHermiteData[0].position = currIntersectionPoint;
						adjacentEdgeHermiteData[0].bIntersecPosToNeg = (corner1HermiteData.distance > corner2HermiteData.distance);

						bNoIntersectionFor3AdjacentEdges = false;

					}
					else if (i == 3)
					{
						//Mark intersection occurred 

						adjacentEdgeHermiteData[1].distance = 0.f;
						adjacentEdgeHermiteData[1].normal = intersectionNormal;
						adjacentEdgeHermiteData[1].position = currIntersectionPoint;
						adjacentEdgeHermiteData[1].bIntersecPosToNeg = (corner1HermiteData.distance > corner2HermiteData.distance);

						bNoIntersectionFor3AdjacentEdges = false;

					}
					else if (i == 8)
					{
						//Mark intersection occurred 

						adjacentEdgeHermiteData[2].distance = 0.f;
						adjacentEdgeHermiteData[2].normal = intersectionNormal;
						adjacentEdgeHermiteData[2].position = currIntersectionPoint;
						adjacentEdgeHermiteData[2].bIntersecPosToNeg = (corner1HermiteData.distance > corner2HermiteData.distance);

						bNoIntersectionFor3AdjacentEdges = false;

					}


					//Store hermite info of an edge
					allEdgeHermiteData.push_back(
						{ currIntersectionPoint, intersectionNormal, 0.f, (corner1HermiteData.distance > corner2HermiteData.distance)
						}
					);

				}

				//In the case there is an intersection for any of the 3 adjacent edges, map the voxel to the adjacent edges, else don't.
				if (!bNoIntersectionFor3AdjacentEdges)
				{
					//Map voxel to adjacent edges hermite data array
					voxelToEdgesHermiteDataMap[GetUniqueIndexForGrid(x, y, z, expandedGridWidth, expandedGridHeight)] = adjacentEdgeHermiteData;
				}

				//Calculate the best vertex using Quadratic error function
				glm::vec3 vertexPos(0.f);
				vertexPos = QEFSolver::ComputeBestVertexPosition(allEdgeHermiteData);

				//Calculate centroid of intersection normals
				glm::vec3 vertexNormal(0.f);
				for (const glm::vec3& normal : intersectionNormals)
					vertexNormal += normal;

				vertexNormal = glm::normalize(vertexNormal);

				//SANITY CHECK: CHECK IF CURRENT UNIQUE ID HAS ALREADY BEEN SET FOR VOXEL-VERTEX MAP
				if (voxelVertexIndexMap.find(GetUniqueIndexForGrid(x, y, z, expandedGridWidth, expandedGridHeight)) != voxelVertexIndexMap.end())
				{
					std::cout << "ERROR: THIS UNIQUE ID HAS ALREADY BEEN SET\n";
				}

				//Map voxel to vertex array index position
				voxelVertexIndexMap[GetUniqueIndexForGrid(x, y, z, expandedGridWidth, expandedGridHeight)] = static_cast<int>(modelVertices.size());

				//std::cout << "New Vertex: " << modelVertices.size() / 3<<", ";

				//Store vertex position relative to grid space
				modelVertices.push_back(vertexPos.x);
				modelVertices.push_back(vertexPos.y);
				modelVertices.push_back(vertexPos.z);

				//std::cout << "Vertex added, model vertices size" << modelVertices.size();

				//Store model normals
				modelNormals.push_back(vertexNormal.x);
				modelNormals.push_back(vertexNormal.y);
				modelNormals.push_back(vertexNormal.z);


				}
			}
	}


	//Iterate through the cubes again, and make the edge connections
	for (int x = 0; x < expandedGridWidth; x++)
	{
		for (int y = 0; y < expandedGridHeight; y++)
		{
			for (int z = 0; z < expandedGridDepth; z++)
			{
				//Check if there is no mapping, then no intersections for that voxel's 3 adjacent edges. Skip.
				if (voxelToEdgesHermiteDataMap.find(GetUniqueIndexForGrid(x, y, z, expandedGridWidth, expandedGridHeight)) == voxelToEdgesHermiteDataMap.end())
				{
					continue;
				}


				const std::array<HermiteData, 3> adjacentEdgesIntersection = voxelToEdgesHermiteDataMap[
					GetUniqueIndexForGrid(x, y, z, expandedGridWidth, expandedGridHeight)];


				//Iterate over adjacent edges and make connections where possible
				for (int axis = 0; axis < 3; ++axis)
				{
					//For an edge, check if an intersection exists and all 4 neighboring voxels are valid
					if (adjacentEdgesIntersection[axis].distance != std::numeric_limits<float>::min() && ((x - 1) > 0 && (y - 1) > 0 && (z - 1) > 0))
					{

						std::vector<unsigned int> vertexIndices;
						std::vector<unsigned int> actualVertexIndices;

						//Store vertex indices for neighboring voxels
						for (int i = 0; i < 4; ++i)
						{
							int curX = x + static_cast<int>(adjacentVoxelsOffsets[axis][i].x);
							int curY = y + static_cast<int>(adjacentVoxelsOffsets[axis][i].y);
							int curZ = z + static_cast<int>(adjacentVoxelsOffsets[axis][i].z);

							auto it = voxelVertexIndexMap.find(GetUniqueIndexForGrid(curX, curY, curZ, expandedGridWidth, expandedGridHeight));

							if (it == voxelVertexIndexMap.end())
								continue;  // If voxel is missing, just skip it

							// Store the found vertex index
							vertexIndices.push_back(it->second / 3);
							actualVertexIndices.push_back(it->second);
						}


						// If we have fewer than 4 valid neighbors, we cannot form a face
						if (vertexIndices.size() <= 3)
						{
							continue;
						}
						//Join all 4 vertices in those voxels
						if (vertexIndices.size() == 4)
						{

							//If the transition is from + to -ve 
							if (adjacentEdgesIntersection[axis].bIntersecPosToNeg == true)
							{

								//Triangle 1

								// enable duplicate vertices || //Used for glDrawArrays rather than glDrawElements
								if (settings.bShouldFlatShade)
								{
									//Pos 1 (pairs of 3 floats i.e. a 3D vector)
									modelDuplicateVertices.push_back(modelVertices[actualVertexIndices[1]]);
									modelDuplicateVertices.push_back(modelVertices[actualVertexIndices[1] + 1]);
									modelDuplicateVertices.push_back(modelVertices[actualVertexIndices[1] + 2]);

									//Pos 2
									modelDuplicateVertices.push_back(modelVertices[actualVertexIndices[3]]);
									modelDuplicateVertices.push_back(modelVertices[actualVertexIndices[3] + 1]);
									modelDuplicateVertices.push_back(modelVertices[actualVertexIndices[3] + 2]);

									//Pos 3
									modelDuplicateVertices.push_back(modelVertices[actualVertexIndices[2]]);
									modelDuplicateVertices.push_back(modelVertices[actualVertexIndices[2] + 1]);
									modelDuplicateVertices.push_back(modelVertices[actualVertexIndices[2] + 2]);

									//Similarly, push duplicate normals
									//Normal 1
									//modelDuplicateNormals.push_back(modelNormals[actualVertexIndices[1]]);
									//modelDuplicateNormals.push_back(modelNormals[actualVertexIndices[1] + 1]);
									//modelDuplicateNormals.push_back(modelNormals[actualVertexIndices[1] + 2]);

									////Normal 2
									//modelDuplicateNormals.push_back(modelNormals[actualVertexIndices[3]]);
									//modelDuplicateNormals.push_back(modelNormals[actualVertexIndices[3] + 1]);
									//modelDuplicateNormals.push_back(modelNormals[actualVertexIndices[3] + 2]);

									////Normal 3
									//modelDuplicateNormals.push_back(modelNormals[actualVertexIndices[2]]);
									//modelDuplicateNormals.push_back(modelNormals[actualVertexIndices[2] + 1]);
									//modelDuplicateNormals.push_back(modelNormals[actualVertexIndices[2] + 2]);


									//Triangle 1 Color
									float triangleColorR = RNG::GetRandomFloatNumber(0.0f, 1.0f);
									float triangleColorG = RNG::GetRandomFloatNumber(0.0f, 1.0f);
									float triangleColorB = RNG::GetRandomFloatNumber(0.0f, 1.0f);
									//Push 3 floats (vec3 color) per vertex i.e. 3 vertices
									for (int i = 0; i < 3; ++i)
									{
										modelVertexColors.push_back(triangleColorR);
										modelVertexColors.push_back(triangleColorG);
										modelVertexColors.push_back(triangleColorB);

									}

								}
								else // utilize indexing to render vertices
								{
									modelIndices.push_back(vertexIndices[1]);
									modelIndices.push_back(vertexIndices[3]);
									modelIndices.push_back(vertexIndices[2]);
								}


								//Triangle 2

								/*modelIndices.push_back(vertexIndices[0]);
								modelIndices.push_back(vertexIndices[1]);
								modelIndices.push_back(vertexIndices[2]);*/
								/*	modelDuplicateVertices.push_back(modelVertices[vertexIndices[0]]);
									modelDuplicateVertices.push_back(modelVertices[vertexIndices[1]]);
									modelDuplicateVertices.push_back(modelVertices[vertexIndices[2]]);*/

									// enable duplicate vertices || //Used for glDrawArrays rather than glDrawElements
								if (settings.bShouldFlatShade)
								{

									//Pos 1 (pairs of 3 floats i.e. a 3D vector)
									modelDuplicateVertices.push_back(modelVertices[actualVertexIndices[0]]);
									modelDuplicateVertices.push_back(modelVertices[actualVertexIndices[0] + 1]);
									modelDuplicateVertices.push_back(modelVertices[actualVertexIndices[0] + 2]);

									//Pos 2
									modelDuplicateVertices.push_back(modelVertices[actualVertexIndices[1]]);
									modelDuplicateVertices.push_back(modelVertices[actualVertexIndices[1] + 1]);
									modelDuplicateVertices.push_back(modelVertices[actualVertexIndices[1] + 2]);

									//Pos 3
									modelDuplicateVertices.push_back(modelVertices[actualVertexIndices[2]]);
									modelDuplicateVertices.push_back(modelVertices[actualVertexIndices[2] + 1]);
									modelDuplicateVertices.push_back(modelVertices[actualVertexIndices[2] + 2]);

									//Similarly, push duplicate normals
									//Normal 1
									//modelDuplicateNormals.push_back(modelNormals[actualVertexIndices[0]]);
									//modelDuplicateNormals.push_back(modelNormals[actualVertexIndices[0] + 1]);
									//modelDuplicateNormals.push_back(modelNormals[actualVertexIndices[0] + 2]);

									////Normal 2
									//modelDuplicateNormals.push_back(modelNormals[actualVertexIndices[1]]);
									//modelDuplicateNormals.push_back(modelNormals[actualVertexIndices[1] + 1]);
									//modelDuplicateNormals.push_back(modelNormals[actualVertexIndices[1] + 2]);

									////Normal 3
									//modelDuplicateNormals.push_back(modelNormals[actualVertexIndices[2]]);
									//modelDuplicateNormals.push_back(modelNormals[actualVertexIndices[2] + 1]);
									//modelDuplicateNormals.push_back(modelNormals[actualVertexIndices[2] + 2]);

									float triangleColorR = RNG::GetRandomFloatNumber(0.0f, 1.0f);
									float triangleColorG = RNG::GetRandomFloatNumber(0.0f, 1.0f);
									float triangleColorB = RNG::GetRandomFloatNumber(0.0f, 1.0f);
									//Push 3 floats (vec3 color) per vertex i.e. 3 vertices
									for (int i = 0; i < 3; ++i)
									{
										modelVertexColors.push_back(triangleColorR);
										modelVertexColors.push_back(triangleColorG);
										modelVertexColors.push_back(triangleColorB);

									}

								}
								else // utilize indexing to render vertices
								{
									modelIndices.push_back(vertexIndices[0]);
									modelIndices.push_back(vertexIndices[1]);
									modelIndices.push_back(vertexIndices[2]);
								}

							}
							else //Reverse indices order otherwise (-ve to +ve transition)
							{
								////Triangle 1
								//modelIndices.push_back(vertexIndices[1]);
								//modelIndices.push_back(vertexIndices[2]);
								//modelIndices.push_back(vertexIndices[3]);

								//modelDuplicateVertices.push_back(modelVertices[vertexIndices[1]]);/*
								//modelDuplicateVertices.push_back(modelVertices[vertexIndices[2]]);
								//modelDuplicateVertices.push_back(modelVertices[vertexIndices[3]]);*/

								// enable duplicate vertices || //Used for glDrawArrays rather than glDrawElements
								if (settings.bShouldFlatShade)
								{

									//Pos 1 (pairs of 3 floats i.e. a 3D vector)
									modelDuplicateVertices.push_back(modelVertices[actualVertexIndices[1]]);
									modelDuplicateVertices.push_back(modelVertices[actualVertexIndices[1] + 1]);
									modelDuplicateVertices.push_back(modelVertices[actualVertexIndices[1] + 2]);

									//Pos 2
									modelDuplicateVertices.push_back(modelVertices[actualVertexIndices[2]]);
									modelDuplicateVertices.push_back(modelVertices[actualVertexIndices[2] + 1]);
									modelDuplicateVertices.push_back(modelVertices[actualVertexIndices[2] + 2]);

									//Pos 3
									modelDuplicateVertices.push_back(modelVertices[actualVertexIndices[3]]);
									modelDuplicateVertices.push_back(modelVertices[actualVertexIndices[3] + 1]);
									modelDuplicateVertices.push_back(modelVertices[actualVertexIndices[3] + 2]);

									//Similarly, push duplicate normals
									//Normal 1
									//modelDuplicateNormals.push_back(modelNormals[actualVertexIndices[1]]);
									//modelDuplicateNormals.push_back(modelNormals[actualVertexIndices[1] + 1]);
									//modelDuplicateNormals.push_back(modelNormals[actualVertexIndices[1] + 2]);

									////Normal 2
									//modelDuplicateNormals.push_back(modelNormals[actualVertexIndices[2]]);
									//modelDuplicateNormals.push_back(modelNormals[actualVertexIndices[2] + 1]);
									//modelDuplicateNormals.push_back(modelNormals[actualVertexIndices[2] + 2]);

									////Normal 3
									//modelDuplicateNormals.push_back(modelNormals[actualVertexIndices[3]]);
									//modelDuplicateNormals.push_back(modelNormals[actualVertexIndices[3] + 1]);
									//modelDuplicateNormals.push_back(modelNormals[actualVertexIndices[3] + 2]);

									float triangleColorR = RNG::GetRandomFloatNumber(0.0f, 1.0f);
									float triangleColorG = RNG::GetRandomFloatNumber(0.0f, 1.0f);
									float triangleColorB = RNG::GetRandomFloatNumber(0.0f, 1.0f);
									//Push 3 floats (vec3 color) per vertex i.e. 3 vertices
									for (int i = 0; i < 3; ++i)
									{
										modelVertexColors.push_back(triangleColorR);
										modelVertexColors.push_back(triangleColorG);
										modelVertexColors.push_back(triangleColorB);

									}


								}
								else // utilize indexing to render vertices
								{
									modelIndices.push_back(vertexIndices[1]);
									modelIndices.push_back(vertexIndices[2]);
									modelIndices.push_back(vertexIndices[3]);
								}

								//Triangle 2
								/*modelIndices.push_back(vertexIndices[0]);
								modelIndices.push_back(vertexIndices[2]);
								modelIndices.push_back(vertexIndices[1]);*/

								//modelDuplicateVertices.push_back(modelVertices[vertexIndices[0]]);/*
								//modelDuplicateVertices.push_back(modelVertices[vertexIndices[2]]);
								//modelDuplicateVertices.push_back(modelVertices[vertexIndices[1]]);*/

								// enable duplicate vertices || //Used for glDrawArrays rather than glDrawElements
								if (settings.bShouldFlatShade)
								{

									//Pos 1 (pairs of 3 floats i.e. a 3D vector)
									modelDuplicateVertices.push_back(modelVertices[actualVertexIndices[0]]);
									modelDuplicateVertices.push_back(modelVertices[actualVertexIndices[0] + 1]);
									modelDuplicateVertices.push_back(modelVertices[actualVertexIndices[0] + 2]);

									//Pos 2
									modelDuplicateVertices.push_back(modelVertices[actualVertexIndices[2]]);
									modelDuplicateVertices.push_back(modelVertices[actualVertexIndices[2] + 1]);
									modelDuplicateVertices.push_back(modelVertices[actualVertexIndices[2] + 2]);

									//Pos 3
									modelDuplicateVertices.push_back(modelVertices[actualVertexIndices[1]]);
									modelDuplicateVertices.push_back(modelVertices[actualVertexIndices[1] + 1]);
									modelDuplicateVertices.push_back(modelVertices[actualVertexIndices[1] + 2]);

									//Similarly, push duplicate normals
									//Normal 1
									//modelDuplicateNormals.push_back(modelNormals[actualVertexIndices[0]]);
									//modelDuplicateNormals.push_back(modelNormals[actualVertexIndices[0] + 1]);
									//modelDuplicateNormals.push_back(modelNormals[actualVertexIndices[0] + 2]);

									////Normal 2
									//modelDuplicateNormals.push_back(modelNormals[actualVertexIndices[2]]);
									//modelDuplicateNormals.push_back(modelNormals[actualVertexIndices[2] + 1]);
									//modelDuplicateNormals.push_back(modelNormals[actualVertexIndices[2] + 2]);

									////Normal 3
									//modelDuplicateNormals.push_back(modelNormals[actualVertexIndices[1]]);
									//modelDuplicateNormals.push_back(modelNormals[actualVertexIndices[1] + 1]);
									//modelDuplicateNormals.push_back(modelNormals[actualVertexIndices[1] + 2]);

									float triangleColorR = RNG::GetRandomFloatNumber(0.0f, 1.0f);
									float triangleColorG = RNG::GetRandomFloatNumber(0.0f, 1.0f);
									float triangleColorB = RNG::GetRandomFloatNumber(0.0f, 1.0f);
									//Push 3 floats (vec3 color) per vertex i.e. 3 vertices
									for (int i = 0; i < 3; ++i)
									{
										modelVertexColors.push_back(triangleColorR);
										modelVertexColors.push_back(triangleColorG);
										modelVertexColors.push_back(triangleColorB);

									}


								}
								else // utilize indexing to render vertices
								{
									modelIndices.push_back(vertexIndices[0]);
									modelIndices.push_back(vertexIndices[2]);
									modelIndices.push_back(vertexIndices[1]);
								}
							}

						}

					}
				}




			}
		}
	}

	//Finally assign the mesh details

	//Set vertices to duplicate mode or indices mode
	vertices = (settings.bShouldFlatShade) ? modelDuplicateVertices : modelVertices;
	normals = (settings.bShouldFlatShade) ? modelDuplicateNormals : modelNormals;
	indices = modelIndices;
	colors = modelVertexColors;

}

void DualContouring::ApplyBrushToVoxels(const float& sphereRadius, const glm::vec3& sphereCenter, EBrushType brushType)
{

	int expandedGridWidth = static_cast<int>(static_cast<float>(this->m_gridWidth) * (1 / this->m_voxelResolution));
	int expandedGridHeight = static_cast<int>(static_cast<float>(this->m_gridHeight) * (1 / this->m_voxelResolution));
	int expandedGridDepth = static_cast<int>(static_cast<float>(this->m_gridDepth) * (1 / this->m_voxelResolution));

	glm::vec3 gridPosition(0.f, 0.f, 0.f);

	glm::mat4 gridModelMatrix(1.f);
	gridModelMatrix = glm::translate(gridModelMatrix, gridPosition);

	glm::vec3 gridCenter((this->m_gridWidth) / 2, (this->m_gridHeight) / 2, (this->m_gridDepth) / 2);

	//Generate vertex positions
	for (int x = 0; x < expandedGridWidth; x++)
	{
		for (int y = 0; y < expandedGridHeight; y++)
		{
			for (int z = 0; z < expandedGridDepth; z++)
			{
				//Get SDF values at corner of this voxel
				std::array < HermiteData, 8>& voxelCornersHermiteData = voxelToCornerHermiteDataMap[GetUniqueIndexForGrid(x, y, z, expandedGridWidth, expandedGridHeight)];

				for (int i = 0; i < 8; ++i)
				{

					float brushSDF = SphereBrush::EvaluateBrushSDF(voxelCornersHermiteData[i].position, sphereCenter, sphereRadius);

					switch (brushType)
					{
						case EBrushType::HardBrushAdd:
						{
							//Use union operation
							if (brushSDF < voxelCornersHermiteData[i].distance)
							{
								voxelCornersHermiteData[i].distance = brushSDF;
								voxelCornersHermiteData[i].normal = SphereBrush::CalculateSurfaceNormal(voxelCornersHermiteData[i].position, sphereCenter, sphereRadius);
							}
							break;
						}
						case EBrushType::HardBrushSubtract:
						{
							//Subtract

							brushSDF *= -1.0f;

							if (brushSDF > voxelCornersHermiteData[i].distance)
							{
								voxelCornersHermiteData[i].distance = brushSDF;
								voxelCornersHermiteData[i].normal = SphereBrush::CalculateSurfaceNormal(voxelCornersHermiteData[i].position, sphereCenter, sphereRadius);
							}
							break;
						}
						case EBrushType::SoftBrushAdd:
						{
							// Calculate distance from center (normalized to [0,1] at brush edge)
							float distToCenter = glm::distance(voxelCornersHermiteData[i].position, sphereCenter);
							float normalizedDist = distToCenter / sphereRadius;

							// Skip if completely outside brush influence
							if (normalizedDist >= 1.0f) break;

							// Gaussian function with finite support
							float falloff = exp(-3.0f * normalizedDist * normalizedDist); 
							float brushInfluence = (1.0f - normalizedDist) * falloff;

							// Blend with existing SDF
							float newSDF = voxelCornersHermiteData[i].distance - brushInfluence * sphereRadius;

							if (newSDF < voxelCornersHermiteData[i].distance) {
								voxelCornersHermiteData[i].distance = newSDF;
								voxelCornersHermiteData[i].normal = SphereBrush::CalculateSurfaceNormal(
									voxelCornersHermiteData[i].position, sphereCenter, sphereRadius);
							}
							break;
						}
						case EBrushType::SoftBrushSubtract:
						{
							// Calculate distance from center (normalized to [0,1] at brush edge)
							float distToCenter = glm::distance(voxelCornersHermiteData[i].position, sphereCenter);
							float normalizedDist = distToCenter / sphereRadius;

							// Skip if completely outside brush influence
							if (normalizedDist >= 1.0f) break;

							// Gaussian function with finite support
							float falloff = exp(-3.0f * normalizedDist * normalizedDist);
							float brushInfluence = (1.0f - normalizedDist) * falloff;

							// Blend with existing SDF (note the + sign for "subtracting")
							float newSDF = voxelCornersHermiteData[i].distance + brushInfluence * sphereRadius;

							if (newSDF > voxelCornersHermiteData[i].distance) {
								voxelCornersHermiteData[i].distance = newSDF;
								voxelCornersHermiteData[i].normal = SphereBrush::CalculateSurfaceNormal(
									voxelCornersHermiteData[i].position, sphereCenter, sphereRadius);
							}
							break;
						}
						default:
						{
							break;
						}
					}

			

				}

			}
		}
	}

}

int DualContouring::GetUniqueIndexForGrid(const int x, const int y, const int z, const int gridWidth,
                                          const int gridHeight)
{
	return x + gridWidth * (y + gridHeight * z);
}

void DualContouring::ClearHashMapData()
{
	voxelVertexIndexMap.clear();
	voxelToEdgesHermiteDataMap.clear();
}
