#ifndef TYPE_REGISTRY_H
#define TYPE_REGISTRY_H

#include "core/ScopedPtr.h"
#include "reflection/Type.h"

namespace rde
{
class TypeRegistry
{
public:
	typedef void (*TypeEnumerator)(const Type* t, void* userData);

	// New type registry with all fundamental types added.
	TypeRegistry();
	~TypeRegistry();

	template<typename T>
	void AddType()
	{
		AddType(TypeOf<T>());
	}
	void AddType(Type* t);

	// To be called after adding all types that relate to themselves.
	// This will convert type names to pointers for quicker access.
	void PostInit();

	void RemoveType(const StrId& typeName);
	// NULL if not found.
	const Type* FindType(const StrId& typeName) const;
	const Type* FindType(uint32_t typeTag) const;	// Hash
	void EnumerateTypes(TypeEnumerator enumerator, void* userData = 0);

	template<typename T>
	T* CreateInstance(const StrId& typeName) const
	{
		return static_cast<T*>(Internal_CreateInstance(typeName.GetId()));
	}

	// Estimation, in bytes.
	size_t CalcMemoryUsage() const;

private:
	void* Internal_CreateInstance(uint32_t typeTag) const;

	struct Impl;
	ScopedPtr<Impl>	m_impl;
};

} // rde

#endif
