#ifndef CORE_REFPTR_H
#define CORE_REFPTR_H

#include "core/Config.h"

namespace rde
{
template<typename Pointed>
class RefPtr
{
public:
	enum EDontAddRef { DontAddRef };

	RefPtr():	m_ptr(0) {/**/}
	explicit RefPtr(Pointed* ptr):		m_ptr(ptr)
	{
		AddRef(m_ptr);
	}
	RefPtr(Pointed* ptr, EDontAddRef):	m_ptr(ptr) {/**/}
	RefPtr(const RefPtr& rhs):		m_ptr(rhs.m_ptr)
	{
		AddRef(m_ptr);
	}
	template<typename U>
	RefPtr(const RefPtr<U>& rhs):	m_ptr(rhs.GetPtr())
	{
		AddRef(m_ptr);
	}
	~RefPtr()
	{
		if (m_ptr)
			m_ptr->Release();
	}

	RefPtr& operator=(const Pointed* ptr)
	{
		if (m_ptr != ptr)
		{
			Unlink();
			AddRef(const_cast<Pointed*>(ptr));
			if (m_ptr)
				m_ptr->Release();
			m_ptr = const_cast<Pointed*>(ptr);
		}
		return *this;
	}
	RefPtr& operator=(const RefPtr& rhs)
	{
		*this = rhs.m_ptr;
		return *this;
	}

	Pointed* operator->() 
	{
		return m_ptr;
	}
	Pointed const* operator->() const 
	{
		return m_ptr;
	}
	Pointed& operator*() 
	{
		return *m_ptr;
	}
	Pointed const& operator*() const 
	{
		return *m_ptr;
	}
	bool operator!() const 
	{
		return !m_ptr;
	}
	bool operator==(const RefPtr& rhs) const
	{
		return m_ptr == rhs.m_ptr;
	}
	bool operator==(Pointed const* p) const 
	{
		return m_ptr == p;
	}
	bool operator<(const RefPtr& rhs) const 
	{
		return m_ptr < rhs.m_ptr;
	}
	bool operator<(Pointed const* p) const 
	{
		return m_ptr < p;
	}

	Pointed* GetPtr() const 
	{
		return m_ptr;
	}

private:
	inline void AddRef(Pointed* ptr)
	{
		if (ptr)
		{
			ptr->AddRef();
		}
	}
	Pointed*	m_ptr;
};

} // rde

#endif // CORE_REFPTR_H
