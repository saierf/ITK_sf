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

#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkBayesianClassifierImageFilter.h"
#include "itkBayesianClassifierInitializationImageFilter.h"
#include "itkImageFileWriter.h"
#include "itkImageToImageFilter.h"
#include "itkGradientAnisotropicDiffusionImageFilter.h"
#include "itkRescaleIntensityImageFilter.h"

#include "itkPipelineMonitorImageFilter.h"
#include "itkTestingMacros.h"


template <typename TInputImage, typename TBayesianClassifierInitializer, typename TBayesianClassifierFilter>
int
TestBayesianClassifierImageFilterWithNoPriors(typename TInputImage::Pointer image,
                                              unsigned int                  numberOfClasses,
                                              unsigned int                  numberOfSmoothingIterations,
                                              char *                        outputFilename)
{
  using BayesianClassifierInitializerType = TBayesianClassifierInitializer;
  using BayesianClassifierFilterType = TBayesianClassifierFilter;

  auto bayesianInitializer = BayesianClassifierInitializerType::New();

  bayesianInitializer->SetInput(image);
  bayesianInitializer->SetNumberOfClasses(numberOfClasses);
  ITK_TEST_SET_GET_VALUE(numberOfClasses, bayesianInitializer->GetNumberOfClasses());

  auto bayesianClassifier = BayesianClassifierFilterType::New();

  bayesianClassifier->SetInput(bayesianInitializer->GetOutput());

  bayesianClassifier->SetNumberOfSmoothingIterations(numberOfSmoothingIterations);
  ITK_TEST_SET_GET_VALUE(numberOfSmoothingIterations, bayesianClassifier->GetNumberOfSmoothingIterations());

  using ExtractedComponentImageType = typename BayesianClassifierFilterType::ExtractedComponentImageType;
  using SmoothingFilterType =
    itk::GradientAnisotropicDiffusionImageFilter<ExtractedComponentImageType, ExtractedComponentImageType>;
  auto smoother = SmoothingFilterType::New();
  smoother->SetNumberOfIterations(1);
  smoother->SetTimeStep(0.125);
  smoother->SetConductanceParameter(3);

  bayesianClassifier->SetSmoothingFilter(smoother);
  ITK_TEST_SET_GET_VALUE(smoother, bayesianClassifier->GetSmoothingFilter().GetPointer());


  using MonitorFilterType = itk::PipelineMonitorImageFilter<TInputImage>;
  auto monitor = MonitorFilterType::New();
  monitor->SetInput(bayesianClassifier->GetOutput());


  using ClassifierOutputImageType = typename BayesianClassifierFilterType::OutputImageType;
  using OutputImageType = itk::Image<unsigned char, TInputImage::ImageDimension>;
  using RescalerType = itk::RescaleIntensityImageFilter<ClassifierOutputImageType, OutputImageType>;
  auto rescaler = RescalerType::New();
  rescaler->SetInput(monitor->GetOutput());
  rescaler->SetOutputMinimum(0);
  rescaler->SetOutputMaximum(255);

  using WriterType = itk::ImageFileWriter<OutputImageType>;

  auto writer = WriterType::New();
  writer->SetFileName(outputFilename);

  writer->SetInput(rescaler->GetOutput());

  ITK_TRY_EXPECT_NO_EXCEPTION(writer->Update());

  if (!monitor->VerifyAllInputCanNotStream())
  {
    std::cout << "Pipeline did not execute as expected!" << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}


template <typename TInputImage, typename TBayesianClassifierInitializer, typename TBayesianClassifierFilter>
int
TestBayesianClassifierImageFilterWithPriors(typename TInputImage::Pointer                                image,
                                            typename TBayesianClassifierFilter::PriorsImageType::Pointer priorsImage,
                                            unsigned int numberOfClasses,
                                            unsigned int numberOfSmoothingIterations,
                                            char *       outputFilename)
{
  using BayesianClassifierInitializerType = TBayesianClassifierInitializer;
  using BayesianClassifierFilterType = TBayesianClassifierFilter;

  auto bayesianInitializer = BayesianClassifierInitializerType::New();

  bayesianInitializer->SetInput(image);

  bayesianInitializer->SetNumberOfClasses(numberOfClasses);

  auto bayesianClassifier = BayesianClassifierFilterType::New();

  bayesianClassifier->SetInput(bayesianInitializer->GetOutput());
  bayesianClassifier->SetPriors(priorsImage);

  bayesianClassifier->SetNumberOfSmoothingIterations(numberOfSmoothingIterations);
  ITK_TEST_SET_GET_VALUE(numberOfSmoothingIterations, bayesianClassifier->GetNumberOfSmoothingIterations());

  using ExtractedComponentImageType = typename BayesianClassifierFilterType::ExtractedComponentImageType;
  using SmoothingFilterType =
    itk::GradientAnisotropicDiffusionImageFilter<ExtractedComponentImageType, ExtractedComponentImageType>;
  auto smoother = SmoothingFilterType::New();
  smoother->SetNumberOfIterations(1);
  smoother->SetTimeStep(0.125);
  smoother->SetConductanceParameter(3);

  bayesianClassifier->SetSmoothingFilter(smoother);
  ITK_TEST_SET_GET_VALUE(smoother, bayesianClassifier->GetSmoothingFilter().GetPointer());


  using MonitorFilterType = itk::PipelineMonitorImageFilter<TInputImage>;
  auto monitor = MonitorFilterType::New();
  monitor->SetInput(bayesianClassifier->GetOutput());


  using ClassifierOutputImageType = typename BayesianClassifierFilterType::OutputImageType;
  using OutputImageType = itk::Image<unsigned char, TInputImage::ImageDimension>;
  using RescalerType = itk::RescaleIntensityImageFilter<ClassifierOutputImageType, OutputImageType>;
  auto rescaler = RescalerType::New();
  rescaler->SetInput(monitor->GetOutput());
  rescaler->SetOutputMinimum(0);
  rescaler->SetOutputMaximum(255);

  using WriterType = itk::ImageFileWriter<OutputImageType>;

  auto writer = WriterType::New();
  writer->SetFileName(outputFilename);

  writer->SetInput(rescaler->GetOutput());

  ITK_TRY_EXPECT_NO_EXCEPTION(writer->Update());

  if (!monitor->VerifyAllInputCanNotStream())
  {
    std::cout << "Pipeline did not execute as expected!" << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}


int
itkBayesianClassifierImageFilterTest(int argc, char * argv[])
{

  if (argc < 6)
  {
    std::cerr << "Usage: " << std::endl;
    std::cerr << itkNameOfTestExecutableMacro(argv)
              << " inputImageFile outputImageFile numberOfClasses smoothingIterations testPriors" << std::endl;
    return EXIT_FAILURE;
  }

  // Set up reader
  constexpr unsigned int Dimension = 2;
  using InputPixelType = unsigned char;
  using InputImageType = itk::Image<InputPixelType, Dimension>;
  using ReaderType = itk::ImageFileReader<InputImageType>;


  using BayesianInitializerType = itk::BayesianClassifierInitializationImageFilter<InputImageType>;

  auto bayesianInitializer = BayesianInitializerType::New();

  ITK_EXERCISE_BASIC_OBJECT_METHODS(
    bayesianInitializer, BayesianClassifierInitializationImageFilter, ImageToImageFilter);


  auto reader = ReaderType::New();
  reader->SetFileName(argv[1]);

  ITK_TRY_EXPECT_NO_EXCEPTION(reader->Update());

  const ReaderType::OutputImageType::Pointer inputImage = reader->GetOutput();

  char *             outputFilename = argv[2];
  const unsigned int numberOfClasses = std::stoi(argv[3]);
  const unsigned int numberOfSmoothingIterations = std::stoi(argv[4]);
  const bool         testPriors = std::stoi(argv[5]);


  using LabelType = unsigned char;
  using PriorType = float;
  using PosteriorType = float;

  using InitialLabelImageType = BayesianInitializerType::OutputImageType;

  using BayesianClassifierFilterType =
    itk::BayesianClassifierImageFilter<InitialLabelImageType, LabelType, PosteriorType, PriorType>;

  auto bayesianClassifier = BayesianClassifierFilterType::New();

  ITK_EXERCISE_BASIC_OBJECT_METHODS(bayesianClassifier, BayesianClassifierImageFilter, ImageToImageFilter);

  bool testStatus = EXIT_SUCCESS;


  if (!testPriors)
  {
    std::cout << "Running the filter with no Priors set..." << std::endl;

    testStatus = TestBayesianClassifierImageFilterWithNoPriors<ReaderType::OutputImageType,
                                                               BayesianInitializerType,
                                                               BayesianClassifierFilterType>(
      inputImage, numberOfClasses, numberOfSmoothingIterations, outputFilename);
  }
  else
  {
    std::cout << "Running the filter with Priors set..." << std::endl;

    using PriorsImageType = BayesianClassifierFilterType::PriorsImageType;

    const InputImageType * priorsInputImage = reader->GetOutput();

    auto priorsImage = PriorsImageType::New();
    priorsImage->CopyInformation(priorsInputImage);
    priorsImage->SetRegions(inputImage->GetLargestPossibleRegion());
    priorsImage->SetNumberOfComponentsPerPixel(5);
    priorsImage->AllocateInitialized();

    testStatus = TestBayesianClassifierImageFilterWithPriors<ReaderType::OutputImageType,
                                                             BayesianInitializerType,
                                                             BayesianClassifierFilterType>(
      inputImage, priorsImage, numberOfClasses, numberOfSmoothingIterations, outputFilename);
  }

  // TEST valid image type combinations.
  // The hypothesis is that the vector element type for
  // TestInitialLabelImageType must be the same as for TestPriorType
  {
    constexpr unsigned int TestDimension = 2;
    using TestLabelType = unsigned char;
    using TestPosteriorType = float;

    using TestPriorType = float;
    using TestInitialLabelImageType = itk::VectorImage<TestPriorType, TestDimension>;

    using ClassifierFilterType =
      itk::BayesianClassifierImageFilter<TestInitialLabelImageType, TestLabelType, TestPosteriorType, TestPriorType>;
    auto filter = ClassifierFilterType::New();

    if (filter.IsNull())
    {
      return EXIT_FAILURE;
    }

    ITK_EXERCISE_BASIC_OBJECT_METHODS(filter, BayesianClassifierImageFilter, ImageToImageFilter);
  }

  {
    constexpr unsigned int TestDimension = 2;
    using TestLabelType = unsigned char;
    using TestPosteriorType = float;

    using TestPriorType = float;
    using TestInitialLabelImageType = itk::VectorImage<double, TestDimension>; // The element type MUST be the PriorType

    using ClassifierFilterType =
      itk::BayesianClassifierImageFilter<TestInitialLabelImageType, TestLabelType, TestPosteriorType, TestPriorType>;
    auto filter = ClassifierFilterType::New();
    if (filter.IsNull())
    {
      return EXIT_FAILURE;
    }

    ITK_EXERCISE_BASIC_OBJECT_METHODS(filter, BayesianClassifierImageFilter, ImageToImageFilter);
  }

  {
    constexpr unsigned int TestDimension = 2;
    using TestLabelType = unsigned char;
    using TestPosteriorType = float;

    using TestPriorType = double;
    using TestInitialLabelImageType =
      itk::VectorImage<TestPriorType, TestDimension>; // The element type MUST be the PriorType

    using ClassifierFilterType =
      itk::BayesianClassifierImageFilter<TestInitialLabelImageType, TestLabelType, TestPosteriorType, TestPriorType>;
    auto filter = ClassifierFilterType::New();
    if (filter.IsNull())
    {
      return EXIT_FAILURE;
    }

    ITK_EXERCISE_BASIC_OBJECT_METHODS(filter, BayesianClassifierImageFilter, ImageToImageFilter);
  }

  std::cout << "Test passed." << std::endl;

  return testStatus;
}
