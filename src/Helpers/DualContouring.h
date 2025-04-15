#pragma once
#include <memory>
#include <unordered_map>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


class USDFComponent;

//Describes the data for a given intersection point
struct HermiteData
{
	glm::vec3 position;
	glm::vec3 normal;
	float distance;
	//Whether the intersection point is from positive to negative or vice versa (used for rendering order of triangles)
	bool bIntersecPosToNeg;
};


class ACamera;
class Settings; 

class DualContouring
{
public:
	DualContouring(const unsigned int& gridWidth, const unsigned int& gridHeight, const unsigned int& gridDepth, const float& voxelSize);
	~DualContouring();

	static const std::vector<glm::vec3> voxelCornerOffsets;
	static const std::vector<std::pair<int, int>> edgePairs;
	static const std::vector<std::vector<glm::vec3>> adjacentVoxelsOffsets;

	static const glm::vec3 GetIntersectionPoint(const glm::vec3& firstPosition, const glm::vec3& secondPosition, const glm::vec3& spherePosition, const float& sphereRadius, int totalSteps = 100);
	static const glm::vec3 CalculateSurfaceNormal(const glm::vec3& intersectionPos, std::weak_ptr<USDFComponent> actorSdfComponent);
	const glm::vec3 EvaluateNormalFromVoxelSDF(glm::vec3 queryPoint);
	//Generates mesh initially
	void InitGenerateMesh(std::vector<float>& vertices, std::vector<float>& normals, std::vector<unsigned int>& indices, std::vector<float>& colors, const std::weak_ptr<USDFComponent> actorSdfComponent);
	//Updates mesh depending on any edits made to the SDF using user-inputs
	void UpdateMesh(std::vector<float>& vertices, std::vector<float>& normals, std::vector<unsigned int>& indices, std::vector<float>& colors);
	void ApplyBrushToVoxels(const float& sphereRadius, const glm::vec3& sphereCenter);
	void DebugDrawVertices(const std::vector<float>& vertices,  std::weak_ptr<ACamera> curCamera, std::weak_ptr<Settings> settings);

	//Maps a voxel (unique index from x,y,z pos) to 3 front-most adjacent edge's hermite data
	std::unordered_map<int, std::array<HermiteData, 3>> voxelToEdgesHermiteDataMap;
	//Maps a voxel (unique index from x,y,z pos) to 8 corner's hermite data
	std::unordered_map<int, std::array<HermiteData, 8>> voxelToCornerHermiteDataMap;

private:
	int m_gridWidth = 15;
	int m_gridHeight = 15;
	int m_gridDepth = 15;
	float m_voxelResolution = 1.0f;

	std::unordered_map<int, int> voxelVertexIndexMap;

	std::weak_ptr<Settings> settings;

private:
	static int GetUniqueIndexForGrid(const int x, const int y, const int z, const int gridWidth, const int gridHeight);
	void ClearHashMapData();

};
