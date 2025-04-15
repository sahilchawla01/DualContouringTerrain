#pragma once
#include <glm/glm.hpp>


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
};
