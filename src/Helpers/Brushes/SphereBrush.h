#pragma once
#include <vector>
#include <glm/glm.hpp>
#include <glm/ext/scalar_constants.hpp>


class SphereBrush
{
public:

	bool bUpdateSDF;

	static float GetBrushContribution(glm::vec3 queryPoint, glm::vec3 brushCenter, float brushRadius)
	{
		float dist = glm::length(queryPoint - brushCenter);
		float falloff = 1.0f - glm::clamp(dist / brushRadius, 0.0f, 1.0f);

		float strength = 1.0f;

		return falloff * strength;
	}

	static glm::vec3 CalculateSurfaceNormal(const glm::vec3& intersectionPos, glm::vec3 brushCenter, float brushRadius)
	{
		//Perform Finite Sum Difference
		const float h = 0.001f;

		const float dx =
			EvaluateBrushSDF(intersectionPos + glm::vec3(h, 0.f, 0.f), brushCenter, brushRadius) -
			EvaluateBrushSDF(intersectionPos - glm::vec3(h, 0.f, 0.f), brushCenter, brushRadius);
		const float dy =
			EvaluateBrushSDF(intersectionPos + glm::vec3(0.f, h, 0.f), brushCenter, brushRadius) -
			EvaluateBrushSDF(intersectionPos - glm::vec3(0.f, h, 0.f), brushCenter, brushRadius);
		const float dz =
			EvaluateBrushSDF(intersectionPos + glm::vec3(0.f, 0.f, h), brushCenter, brushRadius) -
			EvaluateBrushSDF(intersectionPos - glm::vec3(0.f, 0.f, h), brushCenter, brushRadius);

		return glm::normalize(glm::vec3(dx, dy, dz));
	}

	static float EvaluateBrushSDF(const glm::vec3 queryPoint, glm::vec3 brushCenter, float brushRadius)
	{
		return glm::length(queryPoint - brushCenter) - brushRadius;
	}

	static void GenerateSphereMesh(std::vector<float> &vertices, std::vector<float>& normals, std::vector<unsigned int>& indices, float sphereBrushRadius)
	{
		int sectorCount = 16;
		int stackCount = 16;

		float x, y, z, xy;                              // vertex position
		float nx, ny, nz, lengthInv = 1.0f / sphereBrushRadius;    // vertex normal

		float sectorStep = 2.f * glm::pi<float>() / sectorCount;
		float stackStep = glm::pi<float>() / stackCount;
		float sectorAngle, stackAngle;

		for (int i = 0; i <= stackCount; ++i)
		{
			stackAngle = glm::pi<float>() / 2 - i * stackStep;        // starting from pi/2 to -pi/2
			xy = sphereBrushRadius * cosf(stackAngle);             // r * cos(u)
			z = sphereBrushRadius * sinf(stackAngle);              // r * sin(u)

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

	}
};
