#pragma once
#ifndef CORE_SCOPEDPTR_H
#define CORE_SCOPEDPTR_H

#include "core/RdeAssert.h"

namespace rde
{
template<typename T> 
struct DefaultDeleter
{
	static void Delete(T* ptr)
	{
		typedef char T_IsComplete[sizeof(*ptr)];
		delete ptr;
	}
};
template<typename T> 
struct ArrayDeleter
{
	static void Delete(T* ptr)
	{
		typedef char T_IsComplete[sizeof(*ptr)];
		delete[] ptr;
	}
};

// @note	Alternatively we could pass deleter functor,
//			but this would mean overhead for each scoped pointer
//			instance and that's something I'd like to avoid.
template<typename T, template<typename> class Deleter = DefaultDeleter>
class ScopedPtr
{
public:
	explicit ScopedPtr(T* ptr = 0):	m_ptr(ptr) {}
	~ScopedPtr()
	{
		Deleter<T>::Delete(m_ptr);
	}

	// @pre ptr != Get()
	void Reset(T* ptr = 0)
	{
		RDE_ASSERT(m_ptr != ptr);
		Deleter<T>::Delete(m_ptr);
		m_ptr = ptr;
	}

	T& operator*() const
	{
		RDE_ASSERT(m_ptr);
		return *m_ptr;
	}
	T* operator->() const
	{
		RDE_ASSERT(m_ptr);
		return m_ptr;
	}
	bool operator!() const
	{
		return !m_ptr;
	}
	T* Get() const
	{
		return m_ptr;
	}
	void Swap(ScopedPtr& other)
	{
		T* tmp(other.m_ptr);
		other.m_ptr = m_ptr;
		m_ptr = tmp;
	}

private:
	RDE_FORBID_COPY(ScopedPtr);
	T*	m_ptr;
};
} // rde

#endif // RDE_CORE_SCOPEDPTR
