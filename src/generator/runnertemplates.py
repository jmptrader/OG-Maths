#
# Copyright (C) 2013 - present by OpenGamma Inc. and the OpenGamma group of companies
#
# Please see distribution for license.
#

# Runners header file

runners_header = """\
/**
 * Copyright (C) 2013 - present by OpenGamma Inc. and the OpenGamma group of companies
 *
 * Please see distribution for license.
 *
 * This file is autogenerated during the DOGMA2 build process - src/generator/generator.py
 */

#ifndef _RUNNERS_HH
#define _RUNNERS_HH

#include "dispatch.hh"
#include "uncopyable.hh"

namespace librdag {

%(class_definitions)s
} // end namespace librdag

#endif // _RUNNERS_HH
"""

# Runners cc file

runners_cc = """\
/**
 * Copyright (C) 2013 - present by OpenGamma Inc. and the OpenGamma group of companies
 *
 * Please see distribution for licence.
 *
 * This file is autogenerated during the DOGMA2 build process - src/generator/generator.py
 */
#include "dispatch.hh"
#include "runners.hh"
#include "expression.hh"
#include "terminal.hh"
#include "uncopyable.hh"

using namespace std;

namespace librdag {
%(function_definitions)s
} // end namespace librdag
"""

# Binary runner

binary_runner_class_definition = """\
class %(nodename)sRunner: public DispatchVoidBinaryOp, private Uncopyable
{
  public:
    using DispatchBinaryOp<void *>::run;
    virtual void * run(RegContainer& reg0, OGComplexDenseMatrix::Ptr arg0, OGComplexDenseMatrix::Ptr arg1) const override;
    virtual void * run(RegContainer& reg0, OGRealDenseMatrix::Ptr    arg0, OGRealDenseMatrix::Ptr    arg1) const override;
    virtual void * run(RegContainer& reg0, OGRealScalar::Ptr    arg0, OGRealScalar::Ptr    arg1) const override;
};

"""

binary_runner_function =  """\
void *
%(nodename)sRunner::run(RegContainer& reg0, %(arg0type)s::Ptr arg0, %(arg1type)s::Ptr arg1) const
{
  OGNumeric::Ptr ret;
%(implementation)s
  reg0.push_back(ret);
  return nullptr;
}

"""

# Integer parameter runner

integer_parameter_runner_class_definition = """\
class %(nodename)sRunner: public DispatchVoidOp, private Uncopyable
{
  public:
    virtual void* eval(RegContainer& reg, const RegContainer& arg0, OGIntegerScalar::Ptr arg1) const;
    virtual void* run(RegContainer& reg0, const RegContainer& arg0, OGIntegerScalar::Ptr arg1) const;
};

"""

# Infix runner

infix_scalar_runner_implementation = """\
  ret = %(returntype)s::create(arg0->getValue() %(symbol)s arg1->getValue());\
"""

infix_matrix_runner_implementation = """\
  size_t r0 = arg0->getRows();
  size_t r1 = arg1->getRows();
  size_t c0 = arg0->getCols();
  size_t c1 = arg1->getCols();
  
  bool arg0scalar = false, arg1scalar = false;
  %(datatype)s arg0value = 0.0, arg1value = 0.0;

  size_t newRows, newCols;
  
  if ((r0 == 1) && (c0 == 1))
  {
    arg0scalar = true;
    arg0value = arg0->getData()[0];
    newRows = arg1->getRows();
    newCols = arg1->getCols();
  }
  else if ((r1 == 1) && (c1 == 1))
  {
    arg1scalar = true;
    arg1value = arg1->getData()[0];
    newRows = arg0->getRows();
    newCols = arg0->getCols();
  }
  else
  {
    newRows = arg0->getRows();
    newCols = arg0->getCols();
  }
  
  if (((r0 != r1) || (c0 != c1)) && !arg0scalar && !arg1scalar)
  {
    stringstream s;
    s << "Matrix dimensions ";
    s << "(" << r0 << "," << c0 << ")";
    s << " and ";
    s << "(" << r1 << "," << c1 << ")";
    s << " mismatch for operation: %(symbol)s";
    throw rdag_error(s.str());
  }

  %(datatype)s* newData;

  if (arg0scalar)
  {
    size_t datalen = arg1->getDatalen();
    %(datatype)s* data1 = arg1->getData();
    newData = new %(datatype)s[datalen];
    
    for (size_t i = 0; i < datalen; i++)
    {
      newData[i] = data1[i] %(symbol)s arg0value;
    }
  }
  else if (arg1scalar)
  {
    size_t datalen = arg0->getDatalen();
    %(datatype)s* data0 = arg0->getData();
    newData = new %(datatype)s[datalen];
    
    for (size_t i = 0; i < datalen; i++)
    {
      newData[i] = data0[i] %(symbol)s arg1value;
    }
  }
  else
  {
    size_t datalen = arg0->getDatalen();
    %(datatype)s* data0 = arg0->getData();
    %(datatype)s* data1 = arg1->getData();
    newData = new %(datatype)s[datalen];
    
    for (size_t i = 0; i < datalen; i++)
    {
      newData[i] = data0[i] %(symbol)s data1[i];
    }
  }

  ret = %(returntype)s::create(newData, newRows, newCols, OWNER);
"""

# Unary runner

unary_runner_class_definition = """\
class %(nodename)sRunner: public DispatchVoidUnaryOp, private Uncopyable
{
  public:
    virtual void * run(RegContainer& reg, OGRealScalar::Ptr    arg) const override;
    virtual void * run(RegContainer& reg, OGRealDenseMatrix::Ptr    arg) const override;
    virtual void * run(RegContainer& reg, OGComplexDenseMatrix::Ptr arg) const override;
};
"""

unary_runner_function = """\
void *
%(nodename)sRunner::run(RegContainer& reg, %(argtype)s::Ptr arg) const
{
  OGNumeric::Ptr ret;
%(implementation)s
  reg.push_back(ret);
  return nullptr;
}

"""

# Prefix runner

prefix_scalar_runner_implementation = """\
  ret = %(returntype)s::create(%(symbol)s(arg->getValue()));\
"""

prefix_matrix_runner_implementation = """\
  %(datatype)s* data = arg->getData();
  size_t datalen = arg->getDatalen();
  %(datatype)s* newData = new %(datatype)s[datalen];
  for (size_t i = 0; i < datalen; ++i)
  {
    newData[i] = %(symbol)sdata[i];
  }
  ret = %(returntype)s::create(newData, arg->getRows(), arg->getCols(), OWNER);
"""

# UnaryFunction runner

unaryfunction_scalar_runner_implementation = """\
  ret = %(returntype)s::create(%(function)s(arg->getValue()));\
"""

unaryfunction_matrix_runner_implementation = """\
  %(datatype)s* data = arg->getData();
  size_t datalen = arg->getDatalen();
  %(datatype)s* newData = new %(datatype)s[datalen];
  for (size_t i = 0; i < datalen; ++i)
  {
    newData[i] = %(function)s(data[i]);
  }
  ret = %(returntype)s::create(newData, arg->getRows(), arg->getCols(), OWNER);

"""

# Unimplemented runners

unimplementedunary_runner_function = """\
void *
%(nodename)sRunner::run(RegContainer SUPPRESS_UNUSED & reg, %(argtype)s::Ptr SUPPRESS_UNUSED arg) const
{
  throw rdag_error("Unimplemented unary expression node");
  return nullptr;
}
"""

unimplementedbinary_runner_function =  """\
void *
%(nodename)sRunner::run(RegContainer SUPPRESS_UNUSED & reg0,
                        %(arg0type)s::Ptr SUPPRESS_UNUSED arg0,
                        %(arg1type)s::Ptr SUPPRESS_UNUSED arg1) const
{
  throw rdag_error("Unimplemented unary expression node");
  return nullptr;
}
"""
