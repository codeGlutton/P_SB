#pragma once

#include <typeindex>

namespace Protocol
{
	class ObjectInfo;
	class ObjectBaseInfo;
}

class Object : public std::enable_shared_from_this<Object>
{
public:
	Object();
	virtual ~Object();

	bool											IsPlayer() { return _isPlayer; }
	virtual void									MakeObjectInfo(OUT Protocol::ObjectInfo& outInfo);

	template<typename T = Component>
	std::shared_ptr<T>								FindComponent();

public:
	/* 생성자 외 사용 금지 */

	template<typename T = Component, typename... Args>
	std::shared_ptr<T>								CreateComponent(Args&&... args);

	// 컴포넌트 owner 등록 함수
	void											RegisterComponent(ComponentRef& component);

public:
	Protocol::ObjectBaseInfo*						objectBaseInfo;
	Protocol::PosInfo*								posInfo;
	std::atomic<std::weak_ptr<class Room>>			ownerRoom;

protected:
	bool											_isPlayer = false;

private:
	xUnorderedMap<std::type_index, ComponentRef>	_components;
};

template<typename T>
inline std::shared_ptr<T> Object::FindComponent()
{
	auto iterator = _components.find(std::type_index(typeid(T)));
	if (iterator != _components.end())
		return std::static_pointer_cast<T>(iterator->second);
	return nullptr;
}

template<typename T, typename... Args>
inline std::shared_ptr<T> Object::CreateComponent(Args&&... args)
{
	std::shared_ptr<T> newComponent = MakeXShared<T>(std::forward<Args>(args)...);
	RegisterComponent(newComponent);
	_components.emplace(std::type_index(typeid(T)), newComponent);

	return newComponent;
}