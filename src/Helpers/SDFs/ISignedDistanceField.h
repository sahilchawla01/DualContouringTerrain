#pragma once

#include <glm/glm.hpp>

enum class SDFType { Box, Sphere};


class ISignedDistanceField
{
public:
	virtual ~ISignedDistanceField() = default;

	virtual float EvaluateSDF(const glm::vec3 queryPoint) const = 0;
	virtual SDFType GetType() const = 0; 
};
