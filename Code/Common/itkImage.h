/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    itkImage.h
  Language:  C++
  Date:      $Date$
  Version:   $Revision$


Copyright (c) 2000 National Library of Medicine
All rights reserved.

See COPYRIGHT.txt for copyright details.

=========================================================================*/
///templated image class
/**
 * itkImage is the templated image class.
 */

#ifndef __itkImage_h
#define __itkImage_h

#include "itkImageBase.h"
#include "itkIndex.h"
#include <vector>

template <class T, unsigned int TImageDimension=2>
class ITK_EXPORT itkImage : public itkImageBase
{
public:
  /** Smart pointer typedef support */
  typedef itkSmartPointer< itkImage<T, TImageDimension> > Pointer;

  /** Index (iterator) typedef support */
  typedef itkIndex<TImageDimension> Index;

  /** Create an empty image. */
  static itkImage<T, TImageDimension>::Pointer New();

  /**
   * Allocate the image memory. Dimension and Size must be set a priori.
   */
  void Allocate();

  /**
   * Set a pixel
   */
  void SetPixel(const itkImage::Index &index, const T& value);
  
  /**
   * Get a pixel
   */
  const T& GetPixel(const itkImage::Index &index);


 private:
  itkImage();
  virtual ~itkImage();

  std::vector<T> *m_Data;

};

#include "itkImage.cxx"

#endif

