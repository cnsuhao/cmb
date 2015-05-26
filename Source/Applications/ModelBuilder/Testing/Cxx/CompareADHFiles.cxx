//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include <vtksys/SystemTools.hxx>

int main(int argc, const char* argv[])
{
  if(argc != 3)
    {
    std::cerr << "Executable requires 2 arguments: CompareADHFiles <Baseline Dir> <Testing Dir>\n";
    return 1;
    }
  std::string baselineDir = argv[1];
  if(baselineDir[baselineDir.size()-1] != '/')
    {
    baselineDir.append("/");
    }
  std::string testingDir = argv[2];
  if(testingDir[testingDir.size()-1] != '/')
    {
    testingDir.append("/");
    }

  std::vector<std::string> fileNames;
#if defined(_WIN32) || defined(WIN32) || defined(__CYGWIN__)
  fileNames.push_back("RayAndVeg.bc");
#else
  fileNames.push_back("RayAndVeg_Linux.bc");
#endif
  fileNames.push_back("vegfiles/SandySilt.mtd");
  fileNames.push_back("vegfiles/Duricrust.mtd");
  fileNames.push_back("vegfiles/RayAndVeg.dat");
  fileNames.push_back("vegfiles/RayAndVeg.txt");

  int returnValue = 0;
  for(size_t i=0;i<fileNames.size();i++)
    {
    std::string baseLineFileName = baselineDir + fileNames[i];
    if(!vtksys::SystemTools::FileExists(baseLineFileName.c_str(), true))
      {
      std::cerr << baseLineFileName << " is missing.\n";
      returnValue = 1;
      continue;
      }
    std::ifstream baseLineIn(baseLineFileName.c_str());
    std::string testingFileName = testingDir + fileNames[i];
    if(fileNames[i] == "RayAndVeg_Linux.bc")
      {
      testingFileName = testingDir + "RayAndVeg.bc";
      }
    if(!vtksys::SystemTools::FileExists(testingFileName.c_str(), true))
      {
      std::cerr << testingFileName << " is missing.\n";
      returnValue = 1;
      continue;
      }
    std::ifstream testingIn(testingFileName.c_str());
    std::string baseLineLine;
    std::string testingLine;
    while (!baseLineIn.eof() && !testingIn.eof())
      {
      std::getline(baseLineIn,baseLineLine);
      std::getline(testingIn,testingLine);
      if(baseLineLine != testingLine)
        {
        if(baseLineLine.find("SimBuilder version 2.0 generated file on") == std::string::npos &&
           testingLine.find("SimBuilder version 2.0 generated file on") == std::string::npos &&
           baseLineLine.find("short_grass.obj") == std::string::npos &&
           testingLine.find("short_grass.obj") == std::string::npos)
          {
          std::cerr << fileNames[i] << " has differing lines\n";
          std::cerr << "  Base: " << baseLineLine << std::endl;
          std::cerr << "  Test: " << testingLine << std::endl;
          returnValue = 1;
          }
        }
      }
    if(!baseLineIn.eof() || !testingIn.eof())
      {
      std::cerr << fileNames[i] << " are different.\n";
      returnValue = 1;
      }
    }

  if(returnValue == 0)
    {
    std::cerr << "Test passed.\n";
    }
  else
    {
    std::cerr << "\nTest failed.\n";
    }

return returnValue;
}
