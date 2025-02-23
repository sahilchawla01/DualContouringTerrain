#pragma once
#include <memory>

class AActor;

class UActorComponent
{
public:
	UActorComponent(const std::weak_ptr<const AActor> owningActor);

	virtual ~UActorComponent() = default;

protected:

	std::weak_ptr<const AActor> owningActor;
};