#include "UActorComponent.h"
#include "Actors/AActor.h"

UActorComponent::UActorComponent(const std::weak_ptr<const AActor> owningActor)
{
	this->owningActor = owningActor;
}
