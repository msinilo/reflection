#include "reflection/Type.h"

namespace rde
{
Type::Type(uint32_t size, ReflectionType::Enum reflectionType, const StrId& name)
:	m_size(size),
	m_reflectionType(reflectionType),
	m_name(name)
{
}
Type::~Type()
{
}

TypeArray::TypeArray(uint32_t size, const StrId& name, uint32_t containedTypeId, long numElements)
:	Type(size, ReflectionType::ARRAY, name),
	m_containedTypeId(containedTypeId),
	m_numElements(numElements)
{
	RDE_ASSERT(numElements >= 0);

	/*StrId containedTypeName(name);
	const int bracketIndex = containedTypeName.FindIndexOf('[');
	RDE_ASSERT(bracketIndex > 0);
	containedTypeName.TrimEnd(bracketIndex);
	m_containedTypeId = containedTypeName.GetId();*/
}

TypePointer::TypePointer(uint32_t size, const StrId& name, uint32_t pointedTypeId)
:	Type(size, ReflectionType::POINTER, name),
	m_pointedTypeId(pointedTypeId)
{
	// Trim at first '*' to make pointedType name from pointer name.
	//StrId pointedTypeName(name);
	//const int starIndex = pointedTypeName.FindIndexOf('*');
	//RDE_ASSERT(starIndex > 0);
	//pointedTypeName.TrimEnd(starIndex);
	//m_pointedTypeId = pointedTypeName.GetId();
}

}
