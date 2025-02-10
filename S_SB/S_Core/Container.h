#pragma once
#include "Types.h"
#include "Allocator.h"

#include <array>
#include <vector>
#include <list>
#include <queue>
#include <stack>
#include <map>
#include <set>
#include <unordered_map>
#include <unordered_set>

/* xAlloc, xRelease 를 이용한 stl (기본 메모리 풀) */

template<typename Type, uint32 Size>
using xArray				= std::array<Type, Size>;

template<typename Type>
using xVector				= std::vector<Type, StlAllocator<Type>>;

template<typename Type>
using xList					= std::list<Type, StlAllocator<Type>>;

template<typename Type>
using xDeque				= std::deque<Type, StlAllocator<Type>>;

template<typename Type, typename Container = xDeque<Type>>
using xQueue				= std::queue<Type, Container>;

template<typename Type, typename Container = xDeque<Type>>
using xStack				= std::stack<Type, Container>;

template<typename Type, typename Container = xVector<Type>, typename Pred = std::less<typename Container::value_type>>
using xPriorityQueue		= std::priority_queue<Type, Container, Pred>;

template<typename Key, typename Type, typename Pred = std::less<Key>>
using xMap					= std::map<Key, Type, Pred, StlAllocator<std::pair<const Key, Type>>>;

template<typename Key, typename Pred = std::less<Key>>
using xSet					= std::set<Key, Pred, StlAllocator<Key>>;

template <typename Key, typename Type, typename Hasher = std::hash<Key>, typename KeyEq = std::equal_to<Key>>
using xUnorderedMap			= std::unordered_map <Key, Type, Hasher, KeyEq, StlAllocator<std::pair<const Key, Type>>>;

template <typename Key, typename Hasher = std::hash<Key>, typename KeyEq = std::equal_to<Key>>
using xUnorderedSet			= std::unordered_set <Key, Hasher, KeyEq, StlAllocator<const Key>>;

using xString				= std::basic_string<char, std::char_traits<char>, StlAllocator<char>>;
using xWString				= std::basic_string<wchar_t, std::char_traits<wchar_t>, StlAllocator<wchar_t>>;