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

#include <iostream>

#include "itkBinaryMask3DMeshSource.h"
#include "itkTestingMacros.h"

// Define the dimension of the images
constexpr unsigned int Dimension = 3;

// Declare the types of the output images
using ImageType = itk::Image<unsigned short, Dimension>;

// Declare the type of the index,size and region to initialize images
using IndexType = ImageType::IndexType;
using SizeType = ImageType::SizeType;
using RegionType = ImageType::RegionType;
using PixelType = ImageType::PixelType;
using ImagePointerType = ImageType::Pointer;

void
CreateCubeConfig(ImagePointerType    image,
                 const unsigned int  StartX,
                 const unsigned int  StartY,
                 const unsigned int  StartZ,
                 const unsigned char value1,
                 const unsigned char value2,
                 const unsigned char value3,
                 const unsigned char value4,
                 const unsigned char value5,
                 const unsigned char value6,
                 const unsigned char value7,
                 const unsigned char value8);

void
Create16CubeConfig(ImagePointerType    image,
                   const unsigned int  StartX,
                   const unsigned int  StartY,
                   const unsigned int  StartZ,
                   const unsigned char value1,
                   const unsigned char value2,
                   const unsigned char value3,
                   const unsigned char value4);

int
itkBinaryMask3DMeshSourceTest(int argc, char * argv[])
{
  if (argc != 2)
  {
    std::cerr << "Missing parameters." << std::endl;
    std::cerr << "Usage: " << itkNameOfTestExecutableMacro(argv) << " useRegion" << std::endl;
    return EXIT_FAILURE;
  }

  // Declare the type of the Mesh
  using MeshType = itk::Mesh<double>;
  using MeshSourceType = itk::BinaryMask3DMeshSource<ImageType, MeshType>;

  constexpr PixelType backgroundValue = 0;
  constexpr PixelType internalValue = 1;

  SizeType size;
  size[0] = 128;
  size[1] = 128;
  size[2] = 128;

  constexpr IndexType start{};

  RegionType region{ start, size };

  const ImagePointerType image = ImageType::New();
  image->SetRegions(region);
  image->Allocate();
  image->FillBuffer(backgroundValue);

  for (unsigned char counter = 0; counter < 18; ++counter)
  {
    const unsigned int i = (counter / 1) % 2; // 0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1.
    const unsigned int j = (counter / 2) % 2; // 0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1.
    const unsigned int k = (counter / 4) % 2; // 0,0,0,0,1,1,1,1,0,0,0,0,1,1,1,1.
    const unsigned int l = (counter / 8) % 2; // 0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1.
    Create16CubeConfig(image, 0, 0, 3 * counter, i, j, k, l);
  }

  auto meshSource = MeshSourceType::New();

  ITK_EXERCISE_BASIC_OBJECT_METHODS(meshSource, BinaryMask3DMeshSource, ImageToMeshFilter);


  meshSource->SetInput(image);
  meshSource->SetObjectValue(internalValue);

  auto useRegion = static_cast<bool>(std::stoi(argv[1]));
  if (useRegion)
  {
    size[0] = 9;
    size[1] = 9;
    size[2] = 9;
    region.SetSize(size);
    meshSource->SetRegionOfInterest(region);
    ITK_TEST_SET_GET_VALUE(region, meshSource->GetRegionOfInterest());
  }

  ITK_TRY_EXPECT_NO_EXCEPTION(meshSource->Update());

  std::cout << "NumberOfNodes: " << meshSource->GetNumberOfNodes() << std::endl;
  std::cout << "NumberOfCells: " << meshSource->GetNumberOfCells() << std::endl;

  return EXIT_SUCCESS;
}

void
CreateCubeConfig(ImagePointerType    image,
                 const unsigned int  StartX,
                 const unsigned int  StartY,
                 const unsigned int  StartZ,
                 const unsigned char value1,
                 const unsigned char value2,
                 const unsigned char value3,
                 const unsigned char value4,
                 const unsigned char value5,
                 const unsigned char value6,
                 const unsigned char value7,
                 const unsigned char value8)
{
  IndexType index;

  // first corner 0, 0, 0 offset
  index[0] = StartX;
  index[1] = StartY;
  index[2] = StartZ;
  image->SetPixel(index, value1);

  // second  corner 1, 0, 0
  index[0] = StartX + 1;
  index[1] = StartY;
  index[2] = StartZ;
  image->SetPixel(index, value2);

  // third   corner 1, 1, 0
  index[0] = StartX + 1;
  index[1] = StartY + 1;
  index[2] = StartZ;
  image->SetPixel(index, value3);

  // fourth  corner 0, 1, 0
  index[0] = StartX;
  index[1] = StartY + 1;
  index[2] = StartZ;
  image->SetPixel(index, value4);

  // fifth   corner 0, 0, 1
  index[0] = StartX;
  index[1] = StartY;
  index[2] = StartZ + 1;
  image->SetPixel(index, value5);

  // sixth   corner 1, 0, 1
  index[0] = StartX + 1;
  index[1] = StartY;
  index[2] = StartZ + 1;
  image->SetPixel(index, value6);

  // seventh corner 1, 1, 1
  index[0] = StartX + 1;
  index[1] = StartY + 1;
  index[2] = StartZ + 1;
  image->SetPixel(index, value7);

  // eighth  corner 0, 1, 1
  index[0] = StartX;
  index[1] = StartY + 1;
  index[2] = StartZ + 1;
  image->SetPixel(index, value8);
}

void
Create16CubeConfig(ImagePointerType    image,
                   const unsigned int  StartX,
                   const unsigned int  StartY,
                   const unsigned int  StartZ,
                   const unsigned char value1,
                   const unsigned char value2,
                   const unsigned char value3,
                   const unsigned char value4)
{
  // Case 0
  CreateCubeConfig(image, StartX + 0, StartY + 0, StartZ + 0, value1, value2, value3, value4, 0, 0, 0, 0);

  // Case 1
  CreateCubeConfig(image, StartX + 3, StartY + 0, StartZ + 0, value1, value2, value3, value4, 0, 0, 0, 1);

  // Case 2
  CreateCubeConfig(image, StartX + 6, StartY + 0, StartZ + 0, value1, value2, value3, value4, 0, 0, 1, 0);

  // Case 3
  CreateCubeConfig(image, StartX + 9, StartY + 0, StartZ + 0, value1, value2, value3, value4, 0, 0, 1, 1);

  // Case 4
  CreateCubeConfig(image, StartX + 0, StartY + 3, StartZ + 0, value1, value2, value3, value4, 0, 1, 0, 0);

  // Case 5
  CreateCubeConfig(image, StartX + 3, StartY + 3, StartZ + 0, value1, value2, value3, value4, 0, 1, 0, 1);

  // Case 6
  CreateCubeConfig(image, StartX + 6, StartY + 3, StartZ + 0, value1, value2, value3, value4, 0, 1, 1, 0);

  // Case 7
  CreateCubeConfig(image, StartX + 9, StartY + 3, StartZ + 0, value1, value2, value3, value4, 0, 1, 1, 1);

  // Case 8
  CreateCubeConfig(image, StartX + 0, StartY + 6, StartZ + 0, value1, value2, value3, value4, 1, 0, 0, 0);

  // Case 9
  CreateCubeConfig(image, StartX + 3, StartY + 6, StartZ + 0, value1, value2, value3, value4, 1, 0, 0, 1);

  // Case 10
  CreateCubeConfig(image, StartX + 6, StartY + 6, StartZ + 0, value1, value2, value3, value4, 1, 0, 1, 0);

  // Case 11
  CreateCubeConfig(image, StartX + 9, StartY + 6, StartZ + 0, value1, value2, value3, value4, 1, 0, 1, 1);

  // Case 12
  CreateCubeConfig(image, StartX + 0, StartY + 9, StartZ + 0, value1, value2, value3, value4, 1, 1, 0, 0);

  // Case 13
  CreateCubeConfig(image, StartX + 3, StartY + 9, StartZ + 0, value1, value2, value3, value4, 1, 1, 0, 1);

  // Case 14
  CreateCubeConfig(image, StartX + 6, StartY + 9, StartZ + 0, value1, value2, value3, value4, 1, 1, 1, 0);

  // Case 15
  CreateCubeConfig(image, StartX + 9, StartY + 9, StartZ + 0, value1, value2, value3, value4, 1, 1, 1, 1);
}
