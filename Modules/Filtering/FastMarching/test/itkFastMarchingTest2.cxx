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

#include "itkFastMarchingImageFilter.h"
#include "itkImageRegionIteratorWithIndex.h"
#include "itkTextOutput.h"
#include "itkCommand.h"


namespace
{
// The following class is used to support callbacks
// on the filter in the pipeline that follows later
class ShowProgressObject
{
public:
  ShowProgressObject(itk::ProcessObject * o) { m_Process = o; }
  void
  ShowProgress()
  {
    std::cout << "Progress " << m_Process->GetProgress() << std::endl;
  }
  itk::ProcessObject::Pointer m_Process;
};
} // namespace

int
itkFastMarchingTest2(int, char *[])
{

  itk::OutputWindow::SetInstance(itk::TextOutput::New().GetPointer());

  // create a fastmarching object
  using PixelType = float;
  using FloatImage = itk::Image<PixelType, 2>;
  using FloatFMType = itk::FastMarchingImageFilter<FloatImage, FloatImage>;

  auto marcher = FloatFMType::New();

  ShowProgressObject                                          progressWatch(marcher);
  const itk::SimpleMemberCommand<ShowProgressObject>::Pointer command =
    itk::SimpleMemberCommand<ShowProgressObject>::New();
  command->SetCallbackFunction(&progressWatch, &ShowProgressObject::ShowProgress);
  marcher->AddObserver(itk::ProgressEvent(), command);

  using NodeType = FloatFMType::NodeType;
  using NodeContainer = FloatFMType::NodeContainer;

  // setup alive points
  auto alivePoints = NodeContainer::New();

  NodeType node;

  constexpr FloatImage::OffsetType offset0 = { { 28, 35 } };

  itk::Index<2> index{};

  node.SetValue(0.0);
  node.SetIndex(index + offset0);
  alivePoints->InsertElement(0, node);

  node.SetValue(42.0);
  index.Fill(200);
  node.SetIndex(index); // this node is out of range
  alivePoints->InsertElement(1, node);

  marcher->SetAlivePoints(alivePoints);


  // setup trial points
  auto trialPoints = NodeContainer::New();

  node.SetValue(1.0);

  index.Fill(0);
  index += offset0;

  index[0] += 1;
  node.SetIndex(index);
  trialPoints->InsertElement(0, node);

  index[0] -= 1;
  index[1] += 1;
  node.SetIndex(index);
  trialPoints->InsertElement(1, node);

  index[0] -= 1;
  index[1] -= 1;
  node.SetIndex(index);
  trialPoints->InsertElement(2, node);

  index[0] += 1;
  index[1] -= 1;
  node.SetIndex(index);
  trialPoints->InsertElement(3, node);

  node.SetValue(42.0);
  index.Fill(300); // this node is out of range
  node.SetIndex(index);
  trialPoints->InsertElement(4, node);

  marcher->SetTrialPoints(trialPoints);

  // specify the size of the output image
  constexpr FloatImage::SizeType size = { { 64, 64 } };
  marcher->SetOutputSize(size);

  // setup a speed image of ones
  auto                   speedImage = FloatImage::New();
  FloatImage::RegionType region;
  region.SetSize(size);
  speedImage->SetLargestPossibleRegion(region);
  speedImage->SetBufferedRegion(region);
  speedImage->Allocate();

  // setup a binary mask image in float (to make sure it works with float)
  auto MaskImage = FloatImage::New();
  MaskImage->SetLargestPossibleRegion(region);
  MaskImage->SetBufferedRegion(region);
  MaskImage->Allocate();

  itk::ImageRegionIterator<FloatImage>          speedIter(speedImage, speedImage->GetBufferedRegion());
  itk::ImageRegionIteratorWithIndex<FloatImage> maskIter(MaskImage, MaskImage->GetBufferedRegion());
  while (!speedIter.IsAtEnd())
  {
    speedIter.Set(1.0);
    FloatImage::IndexType idx = maskIter.GetIndex();
    if (((idx[0] > 22) && (idx[0] < 42) && (idx[1] > 27) && (idx[1] < 37)) ||
        ((idx[1] > 22) && (idx[1] < 42) && (idx[0] > 27) && (idx[0] < 37)))
    {
      maskIter.Set(1.0);
    }
    else
    {
      maskIter.Set(0.0);
    }

    ++maskIter;
    ++speedIter;
  }

  speedImage->Print(std::cout);
  marcher->SetInput(speedImage);
  marcher->SetBinaryMask(MaskImage.GetPointer());
  marcher->SetStoppingValue(100.0);

  // turn on debugging
  marcher->DebugOn();


  // update the marcher
  marcher->Update();


  // check the results
  const FloatImage::Pointer            output = marcher->GetOutput();
  itk::ImageRegionIterator<FloatImage> iterator(output, output->GetBufferedRegion());

  bool passed = true;
  for (; !iterator.IsAtEnd(); ++iterator)
  {
    FloatImage::IndexType tempIndex = iterator.GetIndex();
    auto                  outputValue = static_cast<float>(iterator.Get());

    if (((tempIndex[0] > 22) && (tempIndex[0] < 42) && (tempIndex[1] > 27) && (tempIndex[1] < 37)) ||
        ((tempIndex[1] > 22) && (tempIndex[1] < 42) && (tempIndex[0] > 27) && (tempIndex[0] < 37)))
    {
      tempIndex -= offset0;
      double distance = 0.0;
      for (int j = 0; j < 2; ++j)
      {
        distance += tempIndex[j] * tempIndex[j];
      }
      distance = std::sqrt(distance);

      if (distance < itk::NumericTraits<double>::epsilon())
      {
        continue;
      }

      if (itk::Math::abs(outputValue) / distance > 1.42)
      {
        std::cout << iterator.GetIndex() << ' ';
        std::cout << itk::Math::abs(outputValue) / distance << ' ';
        std::cout << itk::Math::abs(outputValue) << ' ' << distance << std::endl;
        passed = false;
      }
    }
    else
    {
      if (outputValue != 0.)
      {
        std::cout << iterator.GetIndex() << ' ';
        std::cout << outputValue << ' ' << 0.;
        std::cout << std::endl;
        passed = false;
      }
    }
  }

  if (passed)
  {
    std::cout << "Fast Marching test passed" << std::endl;
    return EXIT_SUCCESS;
  }

  std::cout << "Fast Marching test failed" << std::endl;
  return EXIT_FAILURE;
}
