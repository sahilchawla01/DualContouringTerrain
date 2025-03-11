#pragma once
#include "ISignedDistanceField.h"

class SphereSDF : public ISignedDistanceField
{
public:

    glm::vec3 center;
    float radius;

    SphereSDF(const glm::vec3& sphereCenter, float sphereRadius) : center(sphereCenter), radius(sphereRadius) {}

    float EvaluateSDF(const glm::vec3 queryPoint) const override
    {
        return glm::length(queryPoint - center) - radius;
    }
};
