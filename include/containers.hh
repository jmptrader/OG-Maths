/*
 * Copyright (C) 2013 - present by OpenGamma Inc. and the OpenGamma group of companies
 *
 * See distribution for licence.
 *
 */

#ifndef _CONTAINERS_HH
#define _CONTAINERS_HH

#include <vector>
#include "exceptions.hh"
#include "warningmacros.h"

using namespace std;

namespace librdag {

/*
 * A vector that holds pointers. Null pointers are not allowed.
 */
template<typename cTp>
class PtrVector
{
  static_assert(is_pointer<cTp>::value && is_const<typename remove_pointer<cTp>::type>::value,
                "Type cTp must be pointer to const type");
  public:
    PtrVector();
    virtual ~PtrVector();
    typedef typename vector<cTp>::const_iterator citerator;
    void push_back(cTp arg);
    size_t size() const;
    citerator begin() const;
    citerator end() const;
    cTp operator[](size_t n) const;
    virtual PtrVector* copy() const = 0;
  private:
    vector<cTp>* _vector;
    void _check_arg(cTp arg);
};

/*
 * A vector that holds pointers that point to data that it does not own.
 */
template<typename cTp>
class NonOwningPtrVector: public PtrVector<cTp>
{
  public:
    virtual NonOwningPtrVector* copy() const override;
};

/*
 * A vector that holds pointers that point to data that it owns.
 *
 * Classes held in an OwningPtrVector must define a method copy() that returns
 * a pointer to a copy of itself.
 */
template<typename cTp>
class OwningPtrVector: public PtrVector<cTp>
{
  public:
    virtual ~OwningPtrVector() override;
    virtual OwningPtrVector* copy() const override;
};

class OGNumeric;

extern template class NonOwningPtrVector<const int*>;
extern template class NonOwningPtrVector<const OGNumeric*>;
extern template class OwningPtrVector<const int*>;
extern template class OwningPtrVector<const OGNumeric*>;

} // namespace librdag

#endif /* _CONTAINERS_HH */
