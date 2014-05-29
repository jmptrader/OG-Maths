/**
 * Copyright (C) 2014 - present by OpenGamma Inc. and the OpenGamma group of companies
 *
 * Please see distribution for licence.
 *
 */

#include "dispatch.hh"
#include "runners.hh"
#include "expression.hh"
#include "iss.hh"
#include "terminal.hh"
#include "execution.hh"
#include "dispatch.hh"
#include "uncopyable.hh"
#include "runtree.hh"

#include <stdio.h>
#include <complex>
#include <sstream>
#include <limits>

/*
 *  Unit contains code for PINV node runners
 */
namespace librdag {

void *
PINVRunner::run(RegContainer& reg, OGRealScalar::Ptr arg) const
{
  OGNumeric::Ptr ret;
  real16 x = arg->getValue();
  if(x == 0.e0)
  {
    ret = OGRealScalar::create(0.e0);
  }
  else
  {
    ret = OGRealScalar::create(1.e0/x);
  }
  reg.push_back(ret);
  return nullptr;
}

/**
 * Computes the threshold for numerical "zero" in the pinv sense
 * @param msv the max singular value
 * @param rows the number of rows in the matrix
 * @param cols the number of cols in the matrix
 * @return the threshold value for "zero"
 */
real16 pinv_threshold(real16 msv, int rows, int cols)
{
  return (rows > cols ? rows : cols) * msv * std::numeric_limits<real16>::epsilon();
}

template<typename T>
void
pinv_dense_runner(RegContainer& reg, shared_ptr<const OGMatrix<T>> arg)
{
  OGNumeric::Ptr ret; // the returned item

  // Matrix in scalar context, i.e. a 1x1 matrix, pinv is simply value**-1
  if(arg->getRows()==1 && arg->getCols()==1)
  {
    // NOTE: currently no cutoff grounding is applied to the argument, x;
    // if x == 0.e0 then pinv(x) = 0;
    // else pinv(x) = 1/x;
    T x = arg->getData()[0];
    if(x == 0.e0)
    {
      ret = makeConcreteScalar(0.e0);
    }
    else
    {
      ret = makeConcreteScalar(1.e0/x);
    }
  }
  else
  {
    const int m = arg->getRows();
    const int n = arg->getCols();
    const int minmn = m > n ? n : m;

    // Perform SVD on a copy of the argument, since it will get destroyed
    OGExpr::Ptr svd = OGExpr::Ptr{new SVD(OGNumeric::Ptr{arg->createOwningCopy()})};

    // run the tree
    runtree(svd);

    // svd regs now hold [U,S,V**T]
    OGNumeric::Ptr numericU = svd->getRegs()[0];
    OGNumeric::Ptr numericS = svd->getRegs()[1];
    OGNumeric::Ptr numericVT = svd->getRegs()[2];

    // walk S matrix, see if we have anything that is numerically zero.
    // go backwards as singular values are ordered descending.
    real16 * S = numericS->asOGRealDiagonalMatrix()->getData();
    real16 thres = pinv_threshold(S[0], m, n);
    int lim;
    for(lim = minmn - 1 ; lim >= 0; lim--)
    {
      if(std::abs(S[lim])>thres)
      {
        break;
      }
    }

    // scale the diags in the reachable part, zero the rest
    for(int i = 0 ; i <= lim; i++)
    {
      S[i] = 1.e0/S[i];
    }
    for(int i = lim + 1; i < minmn; i++)
    {
      S[i] = 0.e0;
    }

    // create a new transposed inverted diag matrix.
    // this matrix is just a viewer of S, numericS is the owner
    OGNumeric::Ptr invS = OGRealDiagonalMatrix::create(S,n,m);

    // need to transpose U
    OGNumeric::Ptr ctransposeU = OGNumeric::Ptr{new CTRANSPOSE(numericU)};

    // need to transpose VT
    OGNumeric::Ptr ctransposeVT = OGNumeric::Ptr{new CTRANSPOSE(numericVT)};

    // multiply back together as [(V**T)**T * inv(S) * U**T]
    OGNumeric::Ptr VTS = OGNumeric::Ptr{new MTIMES(ctransposeVT, invS)};
    OGNumeric::Ptr VTSUT = OGNumeric::Ptr{new MTIMES(VTS, ctransposeU)};

    // run the tree
    runtree(VTSUT);

    // get the return item
    ret = VTSUT->asOGExpr()->getRegs()[0];
  }

  // shove ret into register
  reg.push_back(ret);
}

void *
PINVRunner::run(RegContainer& reg, OGRealMatrix::Ptr arg) const
{
  pinv_dense_runner<real16>(reg, arg);
  return nullptr;
}

void *
PINVRunner::run(RegContainer& reg, OGComplexMatrix::Ptr arg) const
{
  pinv_dense_runner<complex16>(reg, arg);
  return nullptr;
}

} // end namespace
