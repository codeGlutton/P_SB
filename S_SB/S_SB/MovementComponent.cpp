#include "pch.h"
#include "MovementComponent.h"

MovementComponent::MovementComponent()
{
	state = xnew<Protocol::MoveState>();
}

MovementComponent::~MovementComponent()
{
	xdelete(state);
	state = nullptr;
}
