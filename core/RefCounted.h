#ifndef CORE_REFCOUNTED_H
#define CORE_REFCOUNTED_H

#include "core/Config.h"

namespace rde
{
class RefCountedImpl
{
public:
	typedef short	CounterType;

	void AddRef();
	// True if resulting reference count drops to zero.
	bool Release();
	long GetRefCount() const;

protected:
	RefCountedImpl(CounterType refCount = 0);
	~RefCountedImpl();

private:
	CounterType	m_refCount;
};

class RefCountedImplThreadSafe
{
public:
	typedef short	CounterType;

	void AddRef();
	// True if resulting reference count drops to zero.
	bool Release();
	long GetRefCount() const;

protected:
	RefCountedImplThreadSafe(CounterType refCount = 0);
	~RefCountedImplThreadSafe();

private:
	CounterType	m_refCount;
};

// Usage (CRTP):
//	struct Foo : public RefCounted<Foo> {}
template<typename T>
class RefCounted : public RefCountedImpl
{
public:
	RefCounted() {}
	~RefCounted() {}

	RDE_FORCEINLINE void AddRef()
	{
		RefCountedImpl::AddRef();
	}
	void Release()
	{
		if (RefCountedImpl::Release())
			delete static_cast<T*>(this);
	}
};
template<typename T>
class RefCountedThreadSafe : public RefCountedImplThreadSafe
{
public:
	RefCountedThreadSafe() {}
	~RefCountedThreadSafe() {}

	RDE_FORCEINLINE void AddRef()
	{
		RefCountedImplThreadSafe::AddRef();
	}
	void Release()
	{
		if (RefCountedImplThreadSafe::Release())
			delete static_cast<T*>(this);
	}
};

} // rde

#endif // CORE_REFCOUNTED_H
