#include "reflection/TypeRegistry.h"
#include "reflection/TypeClass.h"
#include "rdestl/hash_map.h"

namespace rde
{
struct TypeRegistry::Impl
{
	// Key: type ID
	typedef hash_map<uint32_t, Type*>	TypeMap;

	Impl()
	{
		AddFundamentalTypes();
	}
	~Impl()
	{
		// @TODO: This is so *ugly*, we need to find a more elegant way of cleaning types.
		for (TypeMap::iterator it = m_types.begin(); it != m_types.end(); ++it)
		{
			Type* t = it->second;
			if (t->m_reflectionType != ReflectionType::FUNDAMENTAL)
				delete t;
		}
	}
	void AddType(Type* t)
	{
		RDE_ASSERT(FindType(t->m_name.GetId()) == 0 && "Type already registered");
		m_types.insert(rde::make_pair(t->m_name.GetId(), t));
	}
	void PostInit(TypeRegistry& typeRegistry)
	{
		for (TypeMap::iterator it = m_types.begin(); it != m_types.end(); ++it)
			it->second->OnPostInit(typeRegistry);
	}
	void RemoveType(const StrId& typeName)
	{
		m_types.erase(typeName.GetId());
	}
	Type* FindType(uint32_t key) const
	{
		TypeMap::const_iterator it = m_types.find(key);
		return it == m_types.end() ? 0 : it->second;
	}
	void EnumerateTypes(TypeRegistry::TypeEnumerator enumerator, void* userData)
	{
		for (TypeMap::const_iterator it = m_types.begin(); it != m_types.end(); ++it)
		{
			enumerator(it->second, userData);
		}
	}
	void* CreateInstance(uint32_t typeTag) const
	{
		TypeClass* tc = rde::ReflectionTypeCast<TypeClass>(FindType(typeTag));
		return tc ? tc->CreateInstance() : 0;
	}
	size_t CalcMemoryUsage() const
	{
		size_t memUsage(0);
		for (TypeMap::const_iterator it = m_types.begin(); it != m_types.end(); ++it)
		{
			Type* t = it->second;
			if (t->m_reflectionType == ReflectionType::CLASS)
			{
				memUsage += sizeof(TypeClass);
				TypeClass* tc = static_cast<TypeClass*>(t);
				const int numFields = tc->GetNumFields(false);
				memUsage += numFields * sizeof(Field);
			}
			else
				memUsage += sizeof(Type);	// TODO: ptr/array etc.
		}
		return memUsage;
	}

	void AddFundamentalTypes()
	{
#		define RDE_PROCESS_FUNDAMENTAL(t, tname)	AddType(TypeOf<t>())
#		include "reflection/FundamentalTypes.h"
	}

	TypeMap	m_types;
};

TypeRegistry::TypeRegistry()
:	m_impl(new Impl)
{
}
TypeRegistry::~TypeRegistry()
{
}

void TypeRegistry::AddType(Type* t)
{
	m_impl->AddType(t);
}
void TypeRegistry::PostInit()
{
	m_impl->PostInit(*this);
}

void TypeRegistry::RemoveType(const StrId& typeName)
{
	m_impl->RemoveType(typeName);
}

const Type* TypeRegistry::FindType(const StrId& typeName) const
{
	return m_impl->FindType(typeName.GetId());
}
const Type* TypeRegistry::FindType(uint32_t typeTag) const
{
	return m_impl->FindType(typeTag);
}

void TypeRegistry::EnumerateTypes(TypeEnumerator enumerator, void* userData)
{
	m_impl->EnumerateTypes(enumerator, userData);
}

size_t TypeRegistry::CalcMemoryUsage() const
{
	return m_impl->CalcMemoryUsage();
}

void* TypeRegistry::Internal_CreateInstance(uint32_t typeTag) const
{
	return m_impl->CreateInstance(typeTag);
}

} // rde
