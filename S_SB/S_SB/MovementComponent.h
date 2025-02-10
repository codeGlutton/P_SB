#pragma once

#include "Component.h"

class MovementComponent : public Component
{
public:
	MovementComponent();
	virtual ~MovementComponent();

public:
	Protocol::MoveState*				state;
};

