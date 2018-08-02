#pragma once
#ifndef _SHARED_STL_ALLOCATOR_
#define _SHARED_STL_ALLOCATOR_
#include "xSharedMemory.h"
#if 1
template<typename T>
class shared_stl_allocator
{
public:
  typedef T* pointer;
  typedef T value_type;
  typedef T& reference;
  typedef std::size_t size_type;
  typedef std::ptrdiff_t difference_type;
  typedef const T* const_pointer;
  typedef const T& const_reference;

  template<class U>
  struct rebind { typedef shared_stl_allocator<U> other; };

  pointer address(reference value)             const { return &value; }
  const_pointer address(const_reference value) const { return &value; }

  pointer allocate(size_type n, const void* = 0)
  {
    if(n==0)
    	return NULL;
    pointer p = (pointer)smalloc(n* sizeof(T));
    if (p == 0)
    	throw std::bad_alloc();
    return p;
  }

  void deallocate(pointer p, const size_type n){sfree(p);}
  void construct(pointer p, const T& val){new((void*)p)T(val);}

  void destroy(pointer p){p->~T();}

  size_type max_size() const throw(){ return (1<<16);}
  shared_stl_allocator() {}
  shared_stl_allocator(const shared_stl_allocator&) {}
  template<class U> shared_stl_allocator(const shared_stl_allocator<U>&) {}
  ~shared_stl_allocator() {}

  bool operator==(const shared_stl_allocator &) const { return true; }
  bool operator!=(const shared_stl_allocator &) const { return false; }
};

#else


template <typename T>
struct shared_stl_allocator
{
  typedef T * pointer;
  typedef const T * const_pointer;
  typedef T & reference;
  typedef const T & const_reference;
  typedef T value_type;


  typedef std::size_t size_type;
  typedef std::ptrdiff_t difference_type;


  template <typename U>
  struct rebind
  {
    typedef shared_stl_allocator<U> other;
  };


  static pointer address(reference r) { return &r; }
  static const_pointer address(const_reference r) { return &r; }
  static pointer allocate(const size_type n, const void* = 0)
  {
    const pointer ret = (pointer) smalloc(n * sizeof(T));
    if (ret == 0)
      throw std::bad_alloc();
    return ret;
  }
  static void deallocate(const pointer p, const size_type)
  { sfree(p); }
  static size_type max_size() { return (std::numeric_limits<size_type>::max)(); }


  bool operator==(const shared_stl_allocator &) const { return true; }
  bool operator!=(const shared_stl_allocator &) const { return false; }


  shared_stl_allocator() { }
  template <typename U>
  shared_stl_allocator(const shared_stl_allocator<U> &) { }


  static void construct(const pointer p, const_reference t)
  { new ((void *) p) T(t); }
  static void destroy(const pointer p)
  { p->~T(); }
};
#endif
#endif
