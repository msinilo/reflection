#include "reflection/Field.h"
#include "reflection/TypeClass.h"
#include "reflection/TypeRegistry.h"

namespace rde
{
Field::Field()
:	m_ownerClass(0),
	m_type(0),
	m_offset(0),
	m_flags(0)
{
}
Field::Field(const StrId& fieldName, uint32_t typeId, uint16_t offset, const TypeClass* ownerClass)
:	m_ownerClass(ownerClass),
	m_type(0),
	m_name(fieldName),
	m_typeId(typeId),
	m_offset(offset),
	m_flags(0)
{
}

void* Field::GetRawDataPtr(void* object, const TypeClass* objectType) const
{
	uint32_t finalOffset = m_offset;
	if (objectType != m_ownerClass)
		finalOffset += objectType->CalcOffsetFrom(m_ownerClass);
	return (uint8_t*)object + finalOffset;
}
void Field::OnPostInit(TypeRegistry& typeRegistry)
{
	m_type = typeRegistry.FindType(m_typeId);
	RDE_ASSERT(m_type != 0);
}

FieldAccessor::FieldAccessor(void* object, const TypeClass* objectType, const Field* field)
:	m_ptr(0)
{
	Init(object, objectType, field);
}
FieldAccessor::FieldAccessor(void* object, const TypeClass* objectType, const StrId& fieldName)
:	m_ptr(0)
{
	const Field* field = objectType->FindField(fieldName);
	if (field)
		Init(object, objectType, field);
}

bool FieldAccessor::IsOK() const
{
	return m_ptr != 0;
}

void FieldAccessor::Init(void* object, const TypeClass* objectType, const Field* field)
{
	RDE_ASSERT(object && objectType && field);
	m_ptr = (uint8_t*)field->GetRawDataPtr(object, objectType);
}

} // rde
