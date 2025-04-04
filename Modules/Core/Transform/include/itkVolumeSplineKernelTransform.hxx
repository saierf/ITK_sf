/*=========================================================================
 *
 *  Copyright NumFOCUS
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *         https://www.apache.org/licenses/LICENSE-2.0.txt
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *=========================================================================*/
#ifndef itkVolumeSplineKernelTransform_hxx
#define itkVolumeSplineKernelTransform_hxx

namespace itk
{
template <typename TParametersValueType, unsigned int VDimension>
void
VolumeSplineKernelTransform<TParametersValueType, VDimension>::ComputeG(const InputVectorType & x,
                                                                        GMatrixType &           gmatrix) const
{
  const TParametersValueType r = x.GetNorm();

  gmatrix.fill(TParametersValueType{});
  const TParametersValueType r3 = r * r * r;
  for (unsigned int i = 0; i < VDimension; ++i)
  {
    gmatrix[i][i] = r3;
  }
}

template <typename TParametersValueType, unsigned int VDimension>
void
VolumeSplineKernelTransform<TParametersValueType, VDimension>::ComputeDeformationContribution(
  const InputPointType & thisPoint,
  OutputPointType &      result) const
{
  const unsigned long numberOfLandmarks = this->m_SourceLandmarks->GetNumberOfPoints();

  PointsIterator sp = this->m_SourceLandmarks->GetPoints()->Begin();

  for (unsigned int lnd = 0; lnd < numberOfLandmarks; ++lnd)
  {
    const InputVectorType      position = thisPoint - sp->Value();
    const TParametersValueType r = position.GetNorm();
    const TParametersValueType r3 = r * r * r;

    for (unsigned int odim = 0; odim < VDimension; ++odim)
    {
      result[odim] += r3 * this->m_DMatrix(odim, lnd);
    }
    ++sp;
  }
}
} // namespace itk
#endif
