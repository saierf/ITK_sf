/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    itkFEMException.cxx
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) 2002 Insight Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

// disable debug warnings in MS compiler
#ifdef _MSC_VER
#pragma warning(disable: 4786)
#endif

#include "itkFEMException.h"
#include <strstream>

namespace itk {
namespace fem {




FEMException::FEMException(const char *file, unsigned int lineNumber, std::string location) :
    ExceptionObject(file,lineNumber)
{
  SetDescription("Unhandled exception in FEM class!");
  SetLocation(location);
}




FEMExceptionIO::FEMExceptionIO(const char *file, unsigned int lineNumber, std::string location, std::string moreDescription) :
    FEMException(file,lineNumber)
{
  SetDescription("IO error in FEM class: "+moreDescription);
  SetLocation(location);
}




FEMExceptionWrongClass::FEMExceptionWrongClass(const char *file, unsigned int lineNumber, std::string location)
  : FEMException(file, lineNumber, location)
{
  SetDescription("Object was of wrong class!");
}




FEMExceptionObjectNotFound::FEMExceptionObjectNotFound(const char *file, unsigned int lineNumber, std::string location, std::string baseClassName, int GN)
  : FEMException(file, lineNumber, location)
{
  m_baseClassName=baseClassName;
  m_GN=GN;
  std::ostrstream buf;
  buf.clear();
  buf<<"Object not found ("<<m_baseClassName<<", GN="<<m_GN<<")!"<<'\0';
  SetDescription(buf.str());
  buf.freeze(false);
}




FEMExceptionSolution::FEMExceptionSolution(const char *file, unsigned int lineNumber, std::string location, std::string moreDescription) :
  FEMException(file,lineNumber)
{
  SetDescription("Error when solving FEM problem: "+moreDescription);
  SetLocation(location);
}




}} // end namespace itk::fem
