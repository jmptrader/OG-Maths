/*
 * Copyright (C) 2013 - present by OpenGamma Inc. and the OpenGamma group of companies
 *
 * See distribution for licence.
 *
 */

#include "containers.hh"
#include "expression.hh"

namespace librdag {

/**
 * PtrVector
 */

template<typename T>
PtrVector<T>::PtrVector()
{
  _vector = new vector<T const *>();
}

template<typename T>
PtrVector<T>::~PtrVector()
{
  delete _vector;
}

template<typename T>
void
PtrVector<T>::push_back(T const * arg)
{
  _check_arg(arg);
  _vector->push_back(arg);
}

template<typename T>
size_t
PtrVector<T>::size() const
{
  return _vector->size();
}

template<typename T>
typename PtrVector<T>::citerator
PtrVector<T>::begin() const
{
  return _vector->begin();
}

template<typename T>
typename PtrVector<T>::citerator
PtrVector<T>::end() const
{
  return _vector->end();
}

template<typename T>
const T*
PtrVector<T>::operator[](size_t n) const
{
  return (*_vector)[n];
}

template<typename T>
void
PtrVector<T>::_check_arg(T const * arg)
{
  if (arg == nullptr)
  {
    throw new librdagException();
  }
}

template class PtrVector<int>;
template class PtrVector<OGNumeric>;

/**
 * NonOwningPtrVector
 */

template<typename T>
NonOwningPtrVector<T>*
NonOwningPtrVector<T>::copy() const
{
  NonOwningPtrVector* c = new NonOwningPtrVector();
  for (auto it = this->begin(); it != this->end(); ++it)
  {
    c->push_back(*it);
  }
  return c;
}

template class NonOwningPtrVector<int>;
template class NonOwningPtrVector<OGNumeric>;

/**
 * OwningPtrVector
 */

template<typename T>
OwningPtrVector<T>::~OwningPtrVector()
{
  for (auto it = this->begin(); it != this->end(); ++it)
  {
    delete *it;
  }
}

namespace detail {

/**
 * OwningPtrVector expects classes to implement the copy() method - this is
 * fine.  However, fundamental types do not implement copy, but we know how to
 * copy them. ptrvector_copy_impl defines an implementation for classes using
 * copy() and an implementation for fundamental types that manually copies the
 * data.
 */

template<typename T, bool Q = is_fundamental<T>::value >
struct owningptrvector_copy_impl;

template<typename T>
struct owningptrvector_copy_impl<T, false>
{
  OwningPtrVector<T>* operator()(const OwningPtrVector<T>* src)
  {
    OwningPtrVector<T>* c = new OwningPtrVector<T>();
    for (auto it = src->begin(); it != src->end(); ++it)
    {
      c->push_back((*it)->copy());
    }
    return c;
  }
};

template<typename T>
struct owningptrvector_copy_impl<T, true>
{
  OwningPtrVector<T>* operator()(const OwningPtrVector<T>* src)
  {
    OwningPtrVector<T>* c = new OwningPtrVector<T>();
    for (auto it = src->begin(); it != src->end(); ++it)
    {
      T* n = new T;
      *n = **it;
      c->push_back(n);
    }
    return c;
  }
};

} // namespace detail

template<typename T>
OwningPtrVector<T>*
OwningPtrVector<T>::copy() const
{
  return detail::owningptrvector_copy_impl<T>()(this);
}

template class OwningPtrVector<int>;
template class OwningPtrVector<OGNumeric>;

} // namespace librdag
