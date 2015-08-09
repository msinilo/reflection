#ifndef RDESTL_FIXED_VECTOR_H
#define RDESTL_FIXED_VECTOR_H

#include "rdestl/alignment.h"
#include "rdestl/vector.h"

#define RDESTL_RECORD_WATERMARKS	0

// @TODO Wont work on 64-bit!
// 4267 -- conversion from size_t to int.
#pragma warning(push)
#pragma warning(disable: 4267)

namespace rde
{
//=============================================================================
template<typename T, class TAllocator, int TCapacity, bool TGrowOnOverflow>
struct fixed_vector_storage 
{
	explicit fixed_vector_storage(const TAllocator& allocator)
	:	m_begin((T*)&m_data[0]),
		m_end(m_begin),
		m_capacity(TCapacity),
		m_allocator(allocator)
#if RDESTL_RECORD_WATERMARKS
		, m_max_size(0)
#endif
	{
		/**/
	}	
	explicit fixed_vector_storage(e_noinitialize)
	{
	}

	// @note	Cant shrink
	void reallocate(base_vector::size_type newCapacity, bool /*canShrink*/ = false)
	{
		if (newCapacity > m_capacity)
		{
			if (!TGrowOnOverflow)
			{
				RDE_ASSERT(!"fixed_vector cannot grow");
				// @TODO: do something more spectacular here... do NOT throw exception, tho :)
			}

			T* newBegin = static_cast<T*>(m_allocator.allocate(newCapacity * sizeof(T)));
			const base_vector::size_type currSize((size_t)(m_end - m_begin));
			const base_vector::size_type newSize = currSize < newCapacity ? 
				currSize : newCapacity;
			// Copy old data if needed.
			if (m_begin)
			{
				rde::copy_construct_n(m_begin, newSize, newBegin);
				destroy(m_begin, currSize);
			}
			m_begin = newBegin;
			m_end = m_begin + newSize;
			m_capacity = newCapacity;
			record_high_watermark();
			RDE_ASSERT(invariant());
		}
	}

	// Reallocates memory, doesnt copy contents of old buffer.
	void reallocate_discard_old(base_vector::size_type newCapacity)
	{
		if (newCapacity > m_capacity)
		{
			if (!TGrowOnOverflow)
			{
				RDE_ASSERT(!"fixed_vector cannot grow");
			}
			T* newBegin = static_cast<T*>(m_allocator.allocate(newCapacity * sizeof(T)));
			const base_vector::size_type currSize((size_t)(m_end - m_begin));
			if (m_begin)
				destroy(m_begin, currSize);
			m_begin = newBegin;
			m_end = m_begin + currSize;
			record_high_watermark();
			m_capacity = newCapacity;
		}
		RDE_ASSERT(invariant());
	}
	RDE_FORCEINLINE void destroy(T* ptr, base_vector::size_type n)
	{
		rde::destruct_n(ptr, n);
		if ((etype_t*)ptr != &m_data[0])
			m_allocator.deallocate(ptr, n * sizeof(T));
	}
	bool invariant() const
	{
		return m_end >= m_begin;
	}
	RDE_FORCEINLINE void record_high_watermark()
	{
#if RDESTL_RECORD_WATERMARKS
		const base_vector::size_type curr_size((size_t)(m_end - m_begin));
		if (curr_size > m_max_size)
			m_max_size = curr_size;
#endif
	}
	base_vector::size_type get_high_watermark() const
	{
#if RDESTL_RECORD_WATERMARKS
		return m_max_size;
#else
		return m_capacity;	// ???
#endif
	}

	typedef typename aligned_as<T>::res	etype_t;	
	
	T*						m_begin;
	T*						m_end;
	// Not T[], because we need uninitialized memory.
	etype_t					m_data[(TCapacity * sizeof(T)) / sizeof(etype_t)];
	// @todo: m_capacity is not really needed for containers that
	// cant overflow.
	base_vector::size_type	m_capacity;
	TAllocator				m_allocator;
#if RDESTL_RECORD_WATERMARKS
	base_vector::size_type	m_max_size;
#endif
};

//=============================================================================
template<typename T, int TCapacity, bool TGrowOnOverflow,
	class TAllocator = rde::allocator>
class fixed_vector : public vector<T, TAllocator, 
	fixed_vector_storage<T, TAllocator, TCapacity, TGrowOnOverflow> >
{
	typedef vector<T, TAllocator, 
		fixed_vector_storage<T, TAllocator, TCapacity, TGrowOnOverflow> > super;
public:
	explicit fixed_vector(const allocator_type& allocator = allocator_type())
	:	super(allocator)
	{
		/**/
	}
	explicit fixed_vector(size_type initialSize, const allocator_type& allocator = allocator_type())
	:	super(initialSize, allocator)
	{
		/**/
	}
	fixed_vector(const T* first, const T* last, const allocator_type& allocator = allocator_type())
	:	super(first, last, allocator)
	{
		/**/
	}
	// @note: allocator is not copied from rhs.
	// @note: will not perform default constructor for newly created objects.
	fixed_vector(const fixed_vector& rhs, const allocator_type& allocator = allocator_type())
	:	super(rhs, allocator)
	{
		/**/
	}
	explicit fixed_vector(e_noinitialize n)
	:	super(n)
	{
		/**/
	}

	fixed_vector& operator=(const fixed_vector& rhs)
	{
		if (&rhs != this)
		{
			super& superThis = *this;
			superThis = rhs;
		}
		return *this;
	}
};

#pragma warning(pop)

} // rde

#endif // #ifndef RDESTL_FIXED_VECTOR_H
