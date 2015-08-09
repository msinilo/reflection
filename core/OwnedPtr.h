#pragma once
#ifndef CORE_OWNEDPTR_H
#define CORE_OWNEDPTR_H

namespace rde
{

// Single owner pointer that can be stored in STL-like containers.
// A little bit like ref-counted object with RC==1 all the time.
template<typename T>
class OwnedPtr
{
public:
	OwnedPtr(T* ptr = 0): m_ptr(ptr) {}
	OwnedPtr(const OwnedPtr<T>& rhs)
	:	m_ptr(rhs.m_ptr)
	{
		rhs.Release();
	}
	template<typename T1>
	OwnedPtr(const OwnedPtr<T1>& rhs)
	:	m_ptr(rhs.m_ptr)
	{
		rhs.Release();
	}
	~OwnedPtr()
	{
		delete m_ptr;
	}

	OwnedPtr& operator=(const OwnedPtr<T>& rhs)
	{
		if (this != &rhs)
		{
			delete m_ptr;
			m_ptr = rhs.m_ptr;
			rhs.Release();
		}
		return *this;
	}
	template<typename T1>
	OwnedPtr& operator=(const OwnedPtr<T1>& rhs)
	{
		if (this != &rhs)
		{
			delete m_ptr;
			m_ptr = rhs.m_ptr;
			rhs.Release();
		}
		return *this;
	}
	OwnedPtr& operator=(T* ptr)
	{
		if (m_ptr != ptr)
		{
			delete m_ptr;
			m_ptr = ptr;
		}
		return *this;
	}

	void Release() const
	{
		m_ptr = 0;
	}

	T* operator->() const	
	{ 
		RDE_ASSERT(m_ptr);
		return m_ptr; 
	}
	T& operator*() const	
	{ 
		RDE_ASSERT(m_ptr);
		return *m_ptr; 
	}

private:
	T mutable* 	m_ptr;
};
}

#endif

