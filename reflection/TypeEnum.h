#ifndef TYPE_ENUM_H
#define TYPE_ENUM_H

#include "reflection/Type.h"
#include "rdestl/fixed_vector.h"

namespace rde
{
class TypeEnum : public Type
{
public:
	struct Constant
	{
		Constant(): m_value(0) {}
		Constant(const StrId& name, long value): m_name(name), m_value(value) {}
		StrId	m_name;
		long	m_value;
	};
	typedef void (*ConstantEnumerator)(const Constant&, void* userData);
	enum
	{
		REFLECTION_TYPE	= ReflectionType::ENUM
	};

	TypeEnum(uint32_t size, const StrId& name);

	void AddConstant(const Constant& constant);
	// NULL if not found.
	const Constant* FindConstant(const StrId& name) const;

	int GetNumConstants() const;
	const Constant& GetConstant(int index) const;
	void EnumerateConstants(ConstantEnumerator enumerator, void* userData = 0) const;

private:
	typedef rde::fixed_vector<Constant, 16, true> Constants;
	Constants m_constants;
};
}

#endif

