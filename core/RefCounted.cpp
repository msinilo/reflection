#include "core/RefCounted.h"
//#include "core/Atomic.h"
#include "core/RdeAssert.h"

namespace
{
// Sanity check
const long MAX_REF_COUNT(1000);
} // <anonymous>

namespace rde
{
//-----------------------------------------------------------------------------
RefCountedImpl::RefCountedImpl(CounterType initialRefCount /* = 0 */)
:	m_refCount(initialRefCount)
{
	RDE_ASSERT(initialRefCount >= 0);
	RDE_ASSERT(initialRefCount < MAX_REF_COUNT);
}

RefCountedImpl::~RefCountedImpl()
{
	RDE_ASSERT(m_refCount == 0);
}

void RefCountedImpl::AddRef()
{
	RDE_ASSERT(m_refCount >= 0);
	RDE_ASSERT(m_refCount < MAX_REF_COUNT);
	++m_refCount;
}

bool RefCountedImpl::Release()
{
	RDE_ASSERT(m_refCount >= 0);
	RDE_ASSERT(m_refCount < MAX_REF_COUNT);
	return --m_refCount <= 0;
}

long RefCountedImpl::GetRefCount() const
{
	return m_refCount;
}

} // rde

