#include "reflection/TypeEnum.h"

namespace
{
bool CompareName(const rde::TypeEnum::Constant& e, const rde::StrId& name)
{
	return e.m_name == name;
}
}

namespace rde
{
TypeEnum::TypeEnum(uint32_t size, const StrId& name)
:	Type(size, ReflectionType::ENUM, name)
{
}

void TypeEnum::AddConstant(const Constant& constant)
{
	RDE_ASSERT(FindConstant(constant.m_name) == 0 && "Enumerator with given name already exists");
	m_constants.push_back(constant);
}
const TypeEnum::Constant* TypeEnum::FindConstant(const StrId& name) const
{
	Constants::const_iterator it = rde::find_if(m_constants.begin(), m_constants.end(), name, CompareName);
	return it == m_constants.end() ? 0 : &(*it);
}
int TypeEnum::GetNumConstants() const
{
	return m_constants.size();
}
const TypeEnum::Constant& TypeEnum::GetConstant(int index) const
{
	RDE_ASSERT(index >= 0 && index < GetNumConstants());
	return m_constants[index];
}
void TypeEnum::EnumerateConstants(ConstantEnumerator enumerator, void* userData) const
{
	for (Constants::const_iterator it = m_constants.begin(); it != m_constants.end(); ++it)
	{
		enumerator(*it, userData);
	}
}

}
