#pragma once
#include <vector>
#include <glm/vec3.hpp>

#include "Helpers/DualContouring.h"

class QEFSolver
{
public:

	static glm::vec3 ComputeBestVertexPosition(const std::vector<HermiteData>& hermiteDataPoints)
	{

        // Define A^T A and A^T b matrices
        glm::mat3 ATA(0.0f);
        glm::vec3 ATb(0.0f);

        // Compute A^T A and A^T b
        for (const auto& data : hermiteDataPoints) {
            glm::vec3 n = data.normal;
            float d = glm::dot(n, data.position); // b_i = n_i . p_i

            glm::mat3 outerProduct = glm::outerProduct(n, n); // n_i * n_i^T
            ATA += outerProduct; // Accumulate A^T A
            ATb += n * d; // Accumulate A^T b
        }

        // Check for rank deficiency
        float determinant = glm::determinant(ATA);
        if (glm::abs(determinant) < 1e-6f) { // Small determinant means nearly singular
            std::cout << "Rank deficient case detected. Using centroid instead.\n";

            glm::vec3 pos(0.0);

             //Get centroid of intersection positions 
             for (const auto& data: hermiteDataPoints)
	        	pos += data.position;

            pos = pos / static_cast<float>(hermiteDataPoints.size());
            return pos;
        }

        // Solve the system A^T A * x = A^T b
        glm::vec3 x = glm::inverse(ATA) * ATb; // Solve for x using matrix inverse

        
        glm::vec3 minBound(FLT_MAX), maxBound(-FLT_MAX);

        for (const auto& data : hermiteDataPoints) 
        {
            minBound = glm::min(minBound, data.position);
            maxBound = glm::max(maxBound, data.position);
        }

        // Sanity: Check if x lies within voxel bounds (with a small margin)
        const float epsilon = 1e-3f;
        if (x.x < minBound.x - epsilon || x.x > maxBound.x + epsilon ||
            x.y < minBound.y - epsilon || x.y > maxBound.y + epsilon ||
            x.z < minBound.z - epsilon || x.z > maxBound.z + epsilon)
        {
            std::cout << "Outside voxel bounds! Returning to centroid.\n";
            glm::vec3 pos(0.0);

            //Get centroid of intersection positions 
            for (const auto& data : hermiteDataPoints)
                pos += data.position;

            pos = pos / static_cast<float>(hermiteDataPoints.size());
            return pos;
        }


        return x;


	}
};


