#include "pch.h"
#include "Object.h"
#include "MovementComponent.h"
#include "Room.h"

USING_SHARED_PTR(MovementComponent);

Object::Object()
{
	objectBaseInfo = xnew<Protocol::ObjectBaseInfo>();
	posInfo = xnew<Protocol::PosInfo>();
	_isPlayer = false;
}

Object::~Object()
{
	xdelete(objectBaseInfo);
	objectBaseInfo = nullptr;
	xdelete(posInfo);
	posInfo = nullptr;
}

void Object::MakeObjectInfo(OUT Protocol::ObjectInfo& outInfo)
{
	outInfo.mutable_object_base_info()->CopyFrom(*objectBaseInfo);
	outInfo.mutable_pos_info()->CopyFrom(*posInfo);

	if (MovementComponentRef movementComponent = FindComponent<MovementComponent>())
	{
		outInfo.set_move_state(*movementComponent->state);
	}
}

void Object::RegisterComponent(ComponentRef& component)
{
	component->_owner = shared_from_this();
	component->OnRegisted();
}
