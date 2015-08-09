#include "reflection/TypeClass.h"
#include "reflection/TypeRegistry.h"

namespace rde
{
TypeClass::TypeClass(uint32_t size, const StrId& name, FnCreateInstance pfnCreateInstance,
					 FnInitVTable pfnInitVTable, uint32_t baseClassId, uint16_t baseOffset)
:	Type(size, ReflectionType::CLASS, name),
	m_baseClassId(baseClassId),
	m_base(0),
	m_pfnCreateInstance(pfnCreateInstance),
	m_pfnInitVTable(pfnInitVTable),
	m_baseOffset(baseOffset)
{
}
TypeClass::~TypeClass()
{
}

void TypeClass::OnPostInit(TypeRegistry& typeReg)
{
	// Resolve base type (by name).
	const Type* baseType = (m_baseClassId == 0 ? 0 : typeReg.FindType(m_baseClassId));
	RDE_ASSERT(baseType == 0 ||
		baseType->m_reflectionType == ReflectionType::CLASS && "Base type of class has to be TypeClass");
	m_base = (const TypeClass*)baseType;

	for (Fields::iterator it = m_fields.begin(); it != m_fields.end(); ++it)
		it->OnPostInit(typeReg);
}

void TypeClass::AddField(const Field& field)
{
	RDE_ASSERT(FindField(field.m_name) == 0 && "Field with specified name already present");
	m_fields.push_back(field);
}

const Field* TypeClass::FindField(const StrId& name, bool includingBaseClasses) const
{
	const int numFields = m_fields.size();
	for (int i = 0; i < numFields; ++i)
	{
		if (name == m_fields[i].m_name)
			return &m_fields[i];
	}
	return m_base && includingBaseClasses ? m_base->FindField(name, true) : 0;
}

int TypeClass::GetNumFields(bool includingBaseClasses) const
{
	int numFields = m_fields.size();
	if (includingBaseClasses)
	{
		const TypeClass* iter = m_base;
		while (iter != 0)
		{
			numFields += iter->GetNumFields(false);
			iter = iter->m_base;
		}
	}
	return numFields;
}

const Field* TypeClass::GetField(int index) const
{
	RDE_ASSERT(index >= 0 && index < GetNumFields(true));
	const int numFields = m_fields.size();
	if (index < numFields)
		return &m_fields[index];

	RDE_ASSERT(m_base);	// We have to have base class if searching for field with index > numFields
	// Convert index to base class index 'space'.
	const int indexInBaseClass = index - numFields;
	return m_base->GetField(indexInBaseClass);
}

void TypeClass::EnumerateFields(FieldEnumerator enumerator, rde::uint32_t typeMask, 
								void* userData, bool includingBaseClasses) const
{
	const int numFields = GetNumFields(includingBaseClasses);
	for (int i = 0; i < numFields; ++i)
	{
		const Field* field = GetField(i);
		if (field->m_type->m_reflectionType & typeMask)
			enumerator(field, userData);
	}
}

bool TypeClass::IsDerivedFrom(const TypeClass* base) const
{
	const TypeClass* iter(m_base);
	while (iter != 0)
	{
		if (iter == base)
			return true;
		iter = iter->m_base;
	}
	return false;
}

uint32_t TypeClass::CalcOffsetFrom(const TypeClass* base) const
{
	if (base == 0)
		return 0;

	RDE_ASSERT(IsDerivedFrom(base));
	const TypeClass* iter(this);
	uint32_t totalOffset(0);
	while (iter && iter != base)
	{
		totalOffset += iter->m_baseOffset;
		iter = iter->m_base;
	}
	return totalOffset;
}

void* TypeClass::CreateInstance() const
{
	RDE_ASSERT(m_pfnCreateInstance && "Trying to instantiate abstract class.");
	return m_pfnCreateInstance();
}

bool TypeClass::HasVTable() const
{
	return m_pfnInitVTable != 0;
}
void* TypeClass::InitVTable(void* mem) const
{
	return m_pfnInitVTable ? m_pfnInitVTable(mem) : mem;
}

} // rde
