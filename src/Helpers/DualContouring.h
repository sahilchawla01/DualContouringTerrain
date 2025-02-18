#pragma once
#include <memory>
#include <unordered_map>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class ACamera;
class Settings; 

class DualContouring
{
public:
	DualContouring(const unsigned int& gridWidth, const unsigned int& gridHeight, const unsigned int& gridDepth, const unsigned int& voxelSize);
	~DualContouring();

	static const std::vector<glm::vec3> voxelCornerOffsets;
	static const std::vector<std::pair<int, int>> edgePairs;
	static const std::vector<std::vector<glm::vec3>> adjacentVoxelsOffsets;

	static const glm::vec3 GetIntersectionPoint(const glm::vec3& firstPosition, const glm::vec3& secondPosition, const glm::vec3& spherePosition, const float& sphereRadius, int totalSteps = 100);
	static const glm::vec3 CalculateSurfaceNormal(const glm::vec3& intersectionPos, const glm::vec3& spherePosition, const float& sphereRadius);

	void GenerateMesh(std::vector<glm::vec3>& vertices, std::vector<glm::vec3>& normals, std::vector<unsigned int>& indices);
	void DebugDrawVertices(std::weak_ptr<ACamera> curCamera, std::weak_ptr<Settings> settings);

private:
	int m_gridWidth = 15;
	int m_gridHeight = 15;
	int m_gridDepth = 15;
	int m_voxelSize = 1;


	std::unordered_map<int, std::array<std::pair<bool, bool>, 3>> voxelEdgeIsoSurfaceMap;
	std::unordered_map<int, std::array<std::pair<float, float>, 3>> voxelEdgeSDFMap;

	std::unordered_map<int, int> voxelVertexIndexMap;

	std::weak_ptr<Settings> settings;

private:
	static int GetUniqueIndexForGrid(const int x, const int y, const int z, const int gridWidth, const int gridHeight);

};
