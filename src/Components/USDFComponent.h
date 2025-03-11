#pragma once
#include <iostream>
#include <vector>

#include "UActorComponent.h"
#include "Helpers/SDFs/ISignedDistanceField.h"


class USDFComponent : public UActorComponent
{

public:

	USDFComponent(std::weak_ptr<const AActor> owningActor) : UActorComponent(owningActor) {}
	~USDFComponent() override = default;

	//Add an SDF
	template<typename T, typename... Args>
	void AddSDF(Args&&... args)
	{
		sdfList.push_back(std::make_unique<T>(std::forward<Args>(args)...));

		std::cout << "\nAdded an SDF to the list";
	}

	float EvaluateSDF(const glm::vec3& queryPoint) const
	{
		float result = std::numeric_limits<float>::max();

		if (sdfList.empty())
		{
			std::cout << "\nCould not evaluate SDF for this query point. The sdf list was empty";
			return result;
		}

		for (const auto& sdf : sdfList)
		{
			result = std::min(result, sdf->EvaluateSDF(queryPoint));
		}

		return result;
	}



private:

	//Stores all the sdf objects
	std::vector<std::unique_ptr<ISignedDistanceField>> sdfList;

};
