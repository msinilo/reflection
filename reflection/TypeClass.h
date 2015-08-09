#ifndef TYPE_CLASS_H
#define TYPE_CLASS_H

#include "reflection/Field.h"
#include "reflection/Type.h"
#include "rdestl/fixed_vector.h"

namespace rde
{	
class TypeClass : public Type
{
public:
	typedef void (*FieldEnumerator)(const Field*, void* userData);
	typedef void* (*FnInitVTable)(void*);
	typedef void* (*FnCreateInstance)();

	enum
	{
		REFLECTION_TYPE	= ReflectionType::CLASS
	};

	explicit TypeClass(uint32_t size, const StrId& name, FnCreateInstance pfnCreateInstance = 0, 
		FnInitVTable pfnInitVTable = 0, uint32_t baseClassId = 0, uint16_t baseOffset = 0);
	virtual ~TypeClass();

	virtual void OnPostInit(TypeRegistry&);

	void AddField(const Field& field);
	const Field* FindField(const StrId& name, bool includingBaseClasses = true) const;
	int GetNumFields(bool includingBaseClasses = true) const;
	const Field* GetField(int index) const;
	void EnumerateFields(FieldEnumerator enumerator, rde::uint32_t typeMask, void* userData = 0,
		bool includingBaseClasses = true) const;

	bool IsDerivedFrom(const TypeClass* base) const;
	// @pre: IsDerivedFrom(base)
	uint32_t CalcOffsetFrom(const TypeClass* base) const;

	void* CreateInstance() const;
	bool HasVTable() const;
	void* InitVTable(void* mem) const;

private:
	typedef rde::fixed_vector<Field, 32, true>	Fields;

	uint32_t			m_baseClassId;
	const TypeClass*	m_base;
	FnCreateInstance	m_pfnCreateInstance;
	FnInitVTable		m_pfnInitVTable;
	uint16_t			m_baseOffset;
	Fields				m_fields;
};

template<typename T>
T* CreateInstance(const TypeClass* type)
{
	return static_cast<T*>(type->CreateInstance());
}

} // rde

#endif
