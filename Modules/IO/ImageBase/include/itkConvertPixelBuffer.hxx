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
#ifndef itkConvertPixelBuffer_hxx
#define itkConvertPixelBuffer_hxx

#include "itkRGBPixel.h"
#include "itkDefaultConvertPixelTraits.h"
#include <cstddef>


namespace itk
{

template <typename InputPixelType, typename OutputPixelType, typename OutputConvertTraits>
template <typename UComponentType>
std::enable_if_t<!std::is_integral_v<UComponentType>, UComponentType>
ConvertPixelBuffer<InputPixelType, OutputPixelType, OutputConvertTraits>::DefaultAlphaValue()
{
  return NumericTraits<UComponentType>::One;
}

template <typename InputPixelType, typename OutputPixelType, typename OutputConvertTraits>
template <typename UComponentType>
std::enable_if_t<std::is_integral_v<UComponentType>, UComponentType>
ConvertPixelBuffer<InputPixelType, OutputPixelType, OutputConvertTraits>::DefaultAlphaValue()
{
  return NumericTraits<UComponentType>::max();
}

template <typename InputPixelType, typename OutputPixelType, typename OutputConvertTraits>
void
ConvertPixelBuffer<InputPixelType, OutputPixelType, OutputConvertTraits>::Convert(const InputPixelType * inputData,
                                                                                  int inputNumberOfComponents,
                                                                                  OutputPixelType * outputData,
                                                                                  size_t            size)
{
  switch (OutputConvertTraits::GetNumberOfComponents())
  {
    // output number of components is 1
    case 1:
    {
      switch (inputNumberOfComponents)
      {
        case 1:
          ConvertGrayToGray(inputData, outputData, size);
          break;
        case 3:
          ConvertRGBToGray(inputData, outputData, size);
          break;
        case 4:
          ConvertRGBAToGray(inputData, outputData, size);
          break;
        default:
          ConvertMultiComponentToGray(inputData, inputNumberOfComponents, outputData, size);
          break;
      }
      break;
    }
    // handle the complex case
    case 2:
    {
      switch (inputNumberOfComponents)
      {
        case 1:
          ConvertGrayToComplex(inputData, outputData, size);
          break;
        case 2:
          ConvertComplexToComplex(inputData, outputData, size);
          break;
        default:
          ConvertMultiComponentToComplex(inputData, inputNumberOfComponents, outputData, size);
          break;
      }
      break;
    }

    // output number of components is 3 RGB
    case 3:
    {
      switch (inputNumberOfComponents)
      {
        case 1:
          ConvertGrayToRGB(inputData, outputData, size);
          break;
        case 3:
          ConvertRGBToRGB(inputData, outputData, size);
          break;
        case 4:
          ConvertRGBAToRGB(inputData, outputData, size);
          break;
        default:
          ConvertMultiComponentToRGB(inputData, inputNumberOfComponents, outputData, size);
      }
      break;
    }
    // output number of components is 4 RGBA
    case 4:
    {
      switch (inputNumberOfComponents)
      {
        case 1:
          ConvertGrayToRGBA(inputData, outputData, size);
          break;
        case 3:
          ConvertRGBToRGBA(inputData, outputData, size);
          break;
        case 4:
          ConvertRGBAToRGBA(inputData, outputData, size);
          break;
        default:
          ConvertMultiComponentToRGBA(inputData, inputNumberOfComponents, outputData, size);
      }
      break;
    }
    // output number of components is 6 (SymmetricSecondRankTensor)
    case 6:
    {
      switch (inputNumberOfComponents)
      {
        case 6:
          ConvertTensor6ToTensor6(inputData, outputData, size);
          break;
        case 9:
          ConvertTensor9ToTensor6(inputData, outputData, size);
          break;
        default:
          itkGenericExceptionMacro("No conversion available from " << inputNumberOfComponents
                                                                   << " components to: 6 components");
          break;
      }
      break;
    }
    default:
    {
      if (inputNumberOfComponents == static_cast<int>(OutputConvertTraits::GetNumberOfComponents()))
      {
        ConvertVectorToVector(inputData, inputNumberOfComponents, outputData, size);
      }
      else
      {
        itkGenericExceptionMacro("No conversion available from " << inputNumberOfComponents << " components to: "
                                                                 << OutputConvertTraits::GetNumberOfComponents()
                                                                 << " components");
      }
      break;
    }
  }
}

template <typename InputPixelType, typename OutputPixelType, typename OutputConvertTraits>
void
ConvertPixelBuffer<InputPixelType, OutputPixelType, OutputConvertTraits>::ConvertGrayToGray(
  const InputPixelType * inputData,
  OutputPixelType *      outputData,
  size_t                 size)
{
  const InputPixelType * endInput = inputData + size;

  while (inputData != endInput)
  {
    OutputConvertTraits::SetNthComponent(0, *outputData++, static_cast<OutputComponentType>(*inputData));
    ++inputData;
  }
}

template <typename InputPixelType, typename OutputPixelType, typename OutputConvertTraits>
void
ConvertPixelBuffer<InputPixelType, OutputPixelType, OutputConvertTraits>::ConvertRGBToGray(
  const InputPixelType * inputData,
  OutputPixelType *      outputData,
  size_t                 size)
{
  // Weights convert from linear RGB to CIE luminance assuming a
  // modern monitor.  See Charles Poynton's Colour FAQ
  // http://www.poynton.com/notes/colour_and_gamma/ColorFAQ.html
  // NOTE: The scale factors are converted to whole numbers for precision

  const InputPixelType * endInput = inputData + size * 3;

  while (inputData != endInput)
  {
    auto val = static_cast<OutputComponentType>((2125.0 * static_cast<OutputComponentType>(*inputData) +
                                                 7154.0 * static_cast<OutputComponentType>(*(inputData + 1)) +
                                                 0721.0 * static_cast<OutputComponentType>(*(inputData + 2))) /
                                                10000.0);
    inputData += 3;
    OutputConvertTraits::SetNthComponent(0, *outputData++, val);
  }
}

template <typename InputPixelType, typename OutputPixelType, typename OutputConvertTraits>
void
ConvertPixelBuffer<InputPixelType, OutputPixelType, OutputConvertTraits>::ConvertRGBAToGray(
  const InputPixelType * inputData,
  OutputPixelType *      outputData,
  size_t                 size)
{
  // Weights convert from linear RGB to CIE luminance assuming a
  // modern monitor.  See Charles Poynton's Colour FAQ
  // http://www.poynton.com/notes/colour_and_gamma/ColorFAQ.html
  // NOTE: The scale factors are converted to whole numbers for
  // precision
  const InputPixelType * endInput = inputData + size * 4;
  double                 maxAlpha(DefaultAlphaValue<InputPixelType>());
  //
  // To be backwards compatible, if the output pixel type
  // isn't a short or char type, don't fix the problem.
  if (sizeof(*outputData) > 2)
  {
    maxAlpha = 1.0;
  }
  while (inputData != endInput)
  {
    // this is an ugly implementation of the simple equation
    // greyval = (.2125 * red + .7154 * green + .0721 * blue) / alpha
    //
    const double tempval = ((2125.0 * static_cast<double>(*inputData) + 7154.0 * static_cast<double>(*(inputData + 1)) +
                             0721.0 * static_cast<double>(*(inputData + 2))) /
                            10000.0) *
                           static_cast<double>(*(inputData + 3)) / maxAlpha;
    inputData += 4;
    auto val = static_cast<OutputComponentType>(tempval);
    OutputConvertTraits::SetNthComponent(0, *outputData++, val);
  }
}

template <typename InputPixelType, typename OutputPixelType, typename OutputConvertTraits>
void
ConvertPixelBuffer<InputPixelType, OutputPixelType, OutputConvertTraits>::ConvertMultiComponentToGray(
  const InputPixelType * inputData,
  int                    inputNumberOfComponents,
  OutputPixelType *      outputData,
  size_t                 size)
{
  //
  // To be backwards compatible, if the output pixel type
  // isn't a short or char type, don't fix the problem.
  double maxAlpha(DefaultAlphaValue<InputPixelType>());
  if (sizeof(*outputData) > 2)
  {
    maxAlpha = 1.0;
  }
  // 2 components assumed intensity and alpha
  if (inputNumberOfComponents == 2)
  {
    const InputPixelType * endInput = inputData + size * 2;
    while (inputData != endInput)
    {
      const OutputComponentType val =
        static_cast<OutputComponentType>(*inputData) * static_cast<OutputComponentType>(*(inputData + 1) / maxAlpha);
      inputData += 2;
      OutputConvertTraits::SetNthComponent(0, *outputData++, val);
    }
  }
  // just skip the rest of the data
  else
  {
    // Weights convert from linear RGB to CIE luminance assuming a
    // modern monitor.  See Charles Poynton's Colour FAQ
    // http://www.poynton.com/notes/colour_and_gamma/ColorFAQ.html
    // NOTE: The scale factors are converted to whole numbers for
    // precision
    const InputPixelType * endInput = inputData + size * static_cast<size_t>(inputNumberOfComponents);
    while (inputData != endInput)
    {
      const double tempval =
        ((2125.0 * static_cast<double>(*inputData) + 7154.0 * static_cast<double>(*(inputData + 1)) +
          0721.0 * static_cast<double>(*(inputData + 2))) /
         10000.0) *
        static_cast<double>(*(inputData + 3)) / maxAlpha;
      auto val = static_cast<OutputComponentType>(tempval);
      OutputConvertTraits::SetNthComponent(0, *outputData++, val);
      inputData += inputNumberOfComponents;
    }
  }
}

template <typename InputPixelType, typename OutputPixelType, typename OutputConvertTraits>
void
ConvertPixelBuffer<InputPixelType, OutputPixelType, OutputConvertTraits>::ConvertGrayToRGB(
  const InputPixelType * inputData,
  OutputPixelType *      outputData,
  size_t                 size)
{
  const InputPixelType * endInput = inputData + size;

  while (inputData != endInput)
  {
    OutputConvertTraits::SetNthComponent(0, *outputData, static_cast<OutputComponentType>(*inputData));
    OutputConvertTraits::SetNthComponent(1, *outputData, static_cast<OutputComponentType>(*inputData));
    OutputConvertTraits::SetNthComponent(2, *outputData, static_cast<OutputComponentType>(*inputData));
    ++inputData;
    ++outputData;
  }
}

template <typename InputPixelType, typename OutputPixelType, typename OutputConvertTraits>
void
ConvertPixelBuffer<InputPixelType, OutputPixelType, OutputConvertTraits>::ConvertRGBToRGB(
  const InputPixelType * inputData,
  OutputPixelType *      outputData,
  size_t                 size)
{
  const InputPixelType * endInput = inputData + size * 3;

  while (inputData != endInput)
  {
    OutputConvertTraits::SetNthComponent(0, *outputData, static_cast<OutputComponentType>(*inputData));
    OutputConvertTraits::SetNthComponent(1, *outputData, static_cast<OutputComponentType>(*(inputData + 1)));
    OutputConvertTraits::SetNthComponent(2, *outputData, static_cast<OutputComponentType>(*(inputData + 2)));
    inputData += 3;
    ++outputData;
  }
}

template <typename InputPixelType, typename OutputPixelType, typename OutputConvertTraits>
void
ConvertPixelBuffer<InputPixelType, OutputPixelType, OutputConvertTraits>::ConvertRGBAToRGB(
  const InputPixelType * inputData,
  OutputPixelType *      outputData,
  size_t                 size)
{
  const InputPixelType * endInput = inputData + size * 4;

  while (inputData != endInput)
  {
    OutputConvertTraits::SetNthComponent(0, *outputData, static_cast<OutputComponentType>(*inputData));
    OutputConvertTraits::SetNthComponent(1, *outputData, static_cast<OutputComponentType>(*(inputData + 1)));
    OutputConvertTraits::SetNthComponent(2, *outputData, static_cast<OutputComponentType>(*(inputData + 2)));
    inputData += 3;
    ++inputData; // skip alpha
    ++outputData;
  }
}

template <typename InputPixelType, typename OutputPixelType, typename OutputConvertTraits>
void
ConvertPixelBuffer<InputPixelType, OutputPixelType, OutputConvertTraits>::ConvertMultiComponentToRGB(
  const InputPixelType * inputData,
  int                    inputNumberOfComponents,
  OutputPixelType *      outputData,
  size_t                 size)
{
  // assume intensity alpha
  if (inputNumberOfComponents == 2)
  {
    const InputPixelType * endInput = inputData + size * 2;
    while (inputData != endInput)
    {
      const OutputComponentType val =
        static_cast<OutputComponentType>(*inputData) * static_cast<OutputComponentType>(*(inputData + 1));
      inputData += 2;
      OutputConvertTraits::SetNthComponent(0, *outputData, val);
      OutputConvertTraits::SetNthComponent(1, *outputData, val);
      OutputConvertTraits::SetNthComponent(2, *outputData, val);
      ++outputData;
    }
  }
  // just skip the rest of the data
  else
  {
    const InputPixelType * endInput = inputData + size * static_cast<size_t>(inputNumberOfComponents);
    while (inputData != endInput)
    {
      OutputConvertTraits::SetNthComponent(0, *outputData, static_cast<OutputComponentType>(*inputData));
      OutputConvertTraits::SetNthComponent(1, *outputData, static_cast<OutputComponentType>(*(inputData + 1)));
      OutputConvertTraits::SetNthComponent(2, *outputData, static_cast<OutputComponentType>(*(inputData + 2)));
      inputData += inputNumberOfComponents;
      ++outputData;
    }
  }
}

template <typename InputPixelType, typename OutputPixelType, typename OutputConvertTraits>
void
ConvertPixelBuffer<InputPixelType, OutputPixelType, OutputConvertTraits>::ConvertGrayToRGBA(
  const InputPixelType * inputData,
  OutputPixelType *      outputData,
  size_t                 size)

{
  const InputPixelType * endInput = inputData + size;

  while (inputData != endInput)
  {
    OutputConvertTraits::SetNthComponent(0, *outputData, static_cast<OutputComponentType>(*inputData));
    OutputConvertTraits::SetNthComponent(1, *outputData, static_cast<OutputComponentType>(*inputData));
    OutputConvertTraits::SetNthComponent(2, *outputData, static_cast<OutputComponentType>(*inputData));
    OutputConvertTraits::SetNthComponent(
      3, *outputData, static_cast<OutputComponentType>(DefaultAlphaValue<InputPixelType>()));
    ++inputData;
    ++outputData;
  }
}

template <typename InputPixelType, typename OutputPixelType, typename OutputConvertTraits>
void
ConvertPixelBuffer<InputPixelType, OutputPixelType, OutputConvertTraits>::ConvertRGBToRGBA(
  const InputPixelType * inputData,
  OutputPixelType *      outputData,
  size_t                 size)
{
  using InputConvertTraits = itk::DefaultConvertPixelTraits<InputPixelType>;
  using InputComponentType = typename InputConvertTraits::ComponentType;
  const InputPixelType * endInput = inputData + size * 3;

  while (inputData != endInput)
  {
    OutputConvertTraits::SetNthComponent(0, *outputData, static_cast<OutputComponentType>(*inputData));
    OutputConvertTraits::SetNthComponent(1, *outputData, static_cast<OutputComponentType>(*(inputData + 1)));
    OutputConvertTraits::SetNthComponent(2, *outputData, static_cast<OutputComponentType>(*(inputData + 2)));
    OutputConvertTraits::SetNthComponent(
      3, *outputData, static_cast<OutputComponentType>(DefaultAlphaValue<InputComponentType>()));

    inputData += 3;
    ++outputData;
  }
}

template <typename InputPixelType, typename OutputPixelType, typename OutputConvertTraits>
void
ConvertPixelBuffer<InputPixelType, OutputPixelType, OutputConvertTraits>::ConvertRGBAToRGBA(
  const InputPixelType * inputData,
  OutputPixelType *      outputData,
  size_t                 size)
{
  const InputPixelType * endInput = inputData + size * 4;

  while (inputData != endInput)
  {
    OutputConvertTraits::SetNthComponent(0, *outputData, static_cast<OutputComponentType>(*inputData));
    OutputConvertTraits::SetNthComponent(1, *outputData, static_cast<OutputComponentType>(*(inputData + 1)));
    OutputConvertTraits::SetNthComponent(2, *outputData, static_cast<OutputComponentType>(*(inputData + 2)));
    OutputConvertTraits::SetNthComponent(3, *outputData, static_cast<OutputComponentType>(*(inputData + 3)));
    inputData += 4;
    ++outputData;
  }
}

template <typename InputPixelType, typename OutputPixelType, typename OutputConvertTraits>
void
ConvertPixelBuffer<InputPixelType, OutputPixelType, OutputConvertTraits>::ConvertMultiComponentToRGBA(
  const InputPixelType * inputData,
  int                    inputNumberOfComponents,
  OutputPixelType *      outputData,
  size_t                 size)
{
  // equal weights for 2 components??
  if (inputNumberOfComponents == 2)
  {
    const InputPixelType * endInput = inputData + size * 2;
    while (inputData != endInput)
    {
      auto val = static_cast<OutputComponentType>(*inputData);
      auto alpha = static_cast<OutputComponentType>(*(inputData + 1));
      inputData += 2;
      OutputConvertTraits::SetNthComponent(0, *outputData, val);
      OutputConvertTraits::SetNthComponent(1, *outputData, val);
      OutputConvertTraits::SetNthComponent(2, *outputData, val);
      OutputConvertTraits::SetNthComponent(3, *outputData, alpha);
    }
  }
  else
  {
    const InputPixelType * endInput = inputData + size * static_cast<size_t>(inputNumberOfComponents);
    while (inputData != endInput)
    {
      OutputConvertTraits::SetNthComponent(0, *outputData, static_cast<OutputComponentType>(*inputData));
      OutputConvertTraits::SetNthComponent(1, *outputData, static_cast<OutputComponentType>(*(inputData + 1)));
      OutputConvertTraits::SetNthComponent(2, *outputData, static_cast<OutputComponentType>(*(inputData + 2)));
      OutputConvertTraits::SetNthComponent(3, *outputData, static_cast<OutputComponentType>(*(inputData + 3)));
      inputData += inputNumberOfComponents;
      ++outputData;
    }
  }
}

template <typename InputPixelType, typename OutputPixelType, typename OutputConvertTraits>
void
ConvertPixelBuffer<InputPixelType, OutputPixelType, OutputConvertTraits>::ConvertVectorToVector(
  const InputPixelType * inputData,
  int                    inputNumberOfComponents,
  OutputPixelType *      outputData,
  size_t                 size)
{
  const int outputNumberOfComponents = OutputConvertTraits::GetNumberOfComponents();
  const int componentCount = std::min(inputNumberOfComponents, outputNumberOfComponents);

  for (size_t i = 0; i < size; ++i)
  {
    for (int c = 0; c < componentCount; ++c)
    {
      OutputConvertTraits::SetNthComponent(c, *outputData, static_cast<OutputComponentType>(*(inputData + c)));
    }
    for (int c = componentCount; c < outputNumberOfComponents; ++c)
    {
      OutputConvertTraits::SetNthComponent(c, *outputData, 0); // set the rest of components to zero
    }

    ++outputData;
    inputData += inputNumberOfComponents;
  }
}

template <typename InputPixelType, typename OutputPixelType, typename OutputConvertTraits>
void
ConvertPixelBuffer<InputPixelType, OutputPixelType, OutputConvertTraits>::ConvertTensor6ToTensor6(
  const InputPixelType * inputData,
  OutputPixelType *      outputData,
  size_t                 size)
{
  for (size_t i = 0; i < size; ++i)
  {
    OutputConvertTraits::SetNthComponent(0, *outputData, static_cast<OutputComponentType>(*inputData));
    OutputConvertTraits::SetNthComponent(1, *outputData, static_cast<OutputComponentType>(*(inputData + 1)));
    OutputConvertTraits::SetNthComponent(2, *outputData, static_cast<OutputComponentType>(*(inputData + 2)));
    OutputConvertTraits::SetNthComponent(3, *outputData, static_cast<OutputComponentType>(*(inputData + 3)));
    OutputConvertTraits::SetNthComponent(4, *outputData, static_cast<OutputComponentType>(*(inputData + 4)));
    OutputConvertTraits::SetNthComponent(5, *outputData, static_cast<OutputComponentType>(*(inputData + 5)));
    ++outputData;
    inputData += 6;
  }
}

// Convert Grayscale to Complex

template <typename InputPixelType, typename OutputPixelType, typename OutputConvertTraits>
void
ConvertPixelBuffer<InputPixelType, OutputPixelType, OutputConvertTraits>::ConvertGrayToComplex(
  const InputPixelType * inputData,
  OutputPixelType *      outputData,
  size_t                 size)
{
  const InputPixelType * endInput = inputData + size;

  while (inputData != endInput)
  {
    OutputConvertTraits::SetNthComponent(0, *outputData, static_cast<OutputComponentType>(*inputData));
    OutputConvertTraits::SetNthComponent(1, *outputData, static_cast<OutputComponentType>(*inputData));
    ++inputData;
    ++outputData;
  }
}

template <typename InputPixelType, typename OutputPixelType, typename OutputConvertTraits>
void
ConvertPixelBuffer<InputPixelType, OutputPixelType, OutputConvertTraits>::ConvertComplexToComplex(
  const InputPixelType * inputData,
  OutputPixelType *      outputData,
  size_t                 size)
{
  const InputPixelType * endInput = inputData + size * 2;

  while (inputData != endInput)
  {
    OutputConvertTraits::SetNthComponent(0, *outputData, static_cast<OutputComponentType>(*inputData));
    OutputConvertTraits::SetNthComponent(1, *outputData, static_cast<OutputComponentType>(*(inputData + 1)));
    inputData += 2;
    ++outputData;
  }
}

template <typename InputPixelType, typename OutputPixelType, typename OutputConvertTraits>
void
ConvertPixelBuffer<InputPixelType, OutputPixelType, OutputConvertTraits>::ConvertTensor9ToTensor6(
  const InputPixelType * inputData,
  OutputPixelType *      outputData,
  size_t                 size)
{
  for (size_t i = 0; i < size; ++i)
  {
    OutputConvertTraits::SetNthComponent(0, *outputData, static_cast<OutputComponentType>(*inputData));
    OutputConvertTraits::SetNthComponent(1, *outputData, static_cast<OutputComponentType>(*(inputData + 1)));
    OutputConvertTraits::SetNthComponent(2, *outputData, static_cast<OutputComponentType>(*(inputData + 2)));
    OutputConvertTraits::SetNthComponent(3, *outputData, static_cast<OutputComponentType>(*(inputData + 4)));
    OutputConvertTraits::SetNthComponent(4, *outputData, static_cast<OutputComponentType>(*(inputData + 5)));
    OutputConvertTraits::SetNthComponent(5, *outputData, static_cast<OutputComponentType>(*(inputData + 8)));
    ++outputData;
    inputData += 9;
  }
}

template <typename InputPixelType, typename OutputPixelType, typename OutputConvertTraits>
void
ConvertPixelBuffer<InputPixelType, OutputPixelType, OutputConvertTraits>::ConvertMultiComponentToComplex(
  const InputPixelType * inputData,
  int                    inputNumberOfComponents,
  OutputPixelType *      outputData,
  size_t                 size)
{
  const InputPixelType * endInput = inputData + size * static_cast<size_t>(inputNumberOfComponents);

  while (inputData != endInput)
  {
    OutputConvertTraits::SetNthComponent(0, *outputData, static_cast<OutputComponentType>(*inputData));
    OutputConvertTraits::SetNthComponent(1, *outputData, static_cast<OutputComponentType>(*(inputData + 1)));
    inputData += inputNumberOfComponents;
    ++outputData;
  }
}

template <typename InputPixelType, typename OutputPixelType, typename OutputConvertTraits>
void
ConvertPixelBuffer<InputPixelType, OutputPixelType, OutputConvertTraits>::ConvertVectorImage(
  const InputPixelType * inputData,
  int                    inputNumberOfComponents,
  OutputPixelType *      outputData,
  size_t                 size)
{
  const size_t length = size * static_cast<size_t>(inputNumberOfComponents);

  for (size_t i = 0; i < length; ++i)
  {
    OutputConvertTraits::SetNthComponent(0, *outputData, static_cast<OutputComponentType>(*inputData));
    ++outputData;
    ++inputData;
  }
}
} // end namespace itk

#endif
