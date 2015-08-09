#ifndef TYPE_H
#define TYPE_H

#include "reflection/StrId.h"

namespace rde
{
class TypeRegistry;

namespace ReflectionType
{
	enum Enum
	{
		INVALID,
		CLASS			= 0x1,
		ENUM			= 0x2,
		POINTER			= 0x4,
		ARRAY			= 0x8,
		FUNDAMENTAL		= 0x10,

		ALL				= 0xFFFFFFFF,	// For enumeration purposes
	};
} 

struct Type
{
	enum
	{
		REFLECTION_TYPE	= ReflectionType::FUNDAMENTAL
	};

	Type(uint32_t size, ReflectionType::Enum reflectionType, const StrId& name);
	virtual ~Type();

	bool IsA(const Type* otherType) const
	{
		return this == otherType;
	}
	// Called on every registered type once they're all added to the registry.
	// Usually performs name/tag -> type resolve.
	virtual void OnPostInit(TypeRegistry&) {}

	uint32_t				m_size;
	ReflectionType::Enum	m_reflectionType;
	StrId					m_name;
};

// @TODO:	Array/pointer types should not perform any string manipulations.
//			We should explicitly pass contained/pointed type tags.
//			This would make switching to purely hash based system much easier.

struct TypeArray : public Type
{
	enum
	{
		REFLECTION_TYPE	= ReflectionType::ARRAY
	};

	TypeArray(uint32_t size, const StrId& name, uint32_t containedTypeId, long numElements);

	uint32_t	m_containedTypeId;
	long		m_numElements;
};
struct TypePointer : public Type
{
	enum
	{
		REFLECTION_TYPE	= ReflectionType::POINTER
	};

	TypePointer(uint32_t size, const StrId& name, uint32_t pointedTypeId);

	uint32_t	m_pointedTypeId;
};

// NULL if reflection type doesn't match.
template<typename T> inline
T* ReflectionTypeCast(Type* ptr)
{
	return (ptr && ptr->m_reflectionType == T::REFLECTION_TYPE) ? static_cast<T*>(ptr) : 0;
}
template<typename T> inline
const T* ReflectionTypeCast(const Type* ptr)
{
	return (ptr && ptr->m_reflectionType == T::REFLECTION_TYPE) ? static_cast<const T*>(ptr) : 0;
}

// Assertion if reflection type doesn't match.
template<typename T> inline
T* SafeReflectionCast(Type* ptr)
{
	RDE_REFLECTION_ASSERT(ptr == 0 || ptr->m_reflectionType == T::REFLECTION_TYPE);
	return static_cast<T*>(ptr);
}
template<typename T> inline
const T* SafeReflectionCast(const Type* ptr)
{
	RDE_REFLECTION_ASSERT(ptr == 0 || ptr->m_reflectionType == T::REFLECTION_TYPE);
	return static_cast<const T*>(ptr);
}


template<typename T>
Type* TypeOf()
{
	// Do not return anything on purpose, so that compiler detects missing cases for us.
}
template<typename T>
const char* GetTypeName()
{
	// Do not return anything on purpose, so that compiler detects missing cases for us.
}
#define RDE_IMPL_GET_TYPE_NAME(type)	\
	template<> inline \
	const char* GetTypeName<type>() \
	{ \
		return #type; \
	}

// Helpers
template<typename T>
Type* TypeOf_Generic(const StrId& name, ReflectionType::Enum reflectionType)
{
	static Type s_type(sizeof(T), reflectionType, name);
	return &s_type;
}
template<typename T>
Type* TypeOf_Fundamental(const StrId& name)
{
	return TypeOf_Generic<T>(name, ReflectionType::FUNDAMENTAL);
}

#define RDE_TYPEOF_FUNDAMENTAL(ftype, tname) \
	template<> \
	inline Type* TypeOf<ftype>() \
	{ \
		return TypeOf_Fundamental<ftype>(tname); \
	} \
	RDE_IMPL_GET_TYPE_NAME(ftype)

//--- Fundamental types
#define RDE_PROCESS_FUNDAMENTAL	RDE_TYPEOF_FUNDAMENTAL
#include "reflection/FundamentalTypes.h"

} // rde

#endif
