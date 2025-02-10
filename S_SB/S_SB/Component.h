#pragma once

#include "Object.h"

class Component
{
	friend void				Object::RegisterComponent(ComponentRef& component);

public:
	Component()
	{
	}

protected:
	virtual void			OnRegisted() { };

protected:
	std::weak_ptr<Object>	_owner;
};

