//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __vtkPointThresholdFilter_h
#define __vtkPointThresholdFilter_h
#include "cmbSystemConfig.h"
#include "vtkCMBFilteringModule.h" // For export macro
#include "vtkPolyDataAlgorithm.h"
#include <iostream>
#include <ostream>

class vtkTransform;

//BTX
class PointThreshold
{

public:
  friend class vtkPointThresholdFilter;

  PointThreshold()
  {
    UseMinX = UseMinY = UseMinZ = false;
    UseMaxX = UseMaxY = UseMaxZ = false;
    Invert = false;
    UseMinRGB = false;
    UseMaxRGB = false;
    UseFilter = 0;

    MinX = MaxX = MinY = 0;
    MaxY = MinZ = MaxZ = 0;
    MinRGB[0] = MinRGB[1] = MinRGB[2] = 0;
    MaxRGB[0] = MaxRGB[1] = MaxRGB[2] = 0;
  }

  ~PointThreshold();

private:
  bool Invert;

  double MinX;
  double MaxX;
  bool UseMinX;
  bool UseMaxX;

  double MinY;
  double MaxY;
  bool UseMinY;
  bool UseMaxY;

  double MinZ;
  double MaxZ;
  bool UseMinZ;
  bool UseMaxZ;

  bool UseMinRGB;
  bool UseMaxRGB;

  /*REPLACED*/ int MinRGB[3];
  /*REPLACED*/ int MaxRGB[3];

  int UseFilter;
};
//ETX

class VTKCMBFILTERING_EXPORT vtkPointThresholdFilter : public vtkPolyDataAlgorithm
{

public:
  static vtkPointThresholdFilter* New();
  vtkTypeMacro(vtkPointThresholdFilter, vtkPolyDataAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  void AddFilter()
  {
    FilterList.push_back(new PointThreshold());
    this->Modified();
  }

  void RemoveFilter()
  {
    if (ActiveFilterIndex < 0 || ActiveFilterIndex >= static_cast<int>(FilterList.size()))
      return;
    std::vector<PointThreshold*>::iterator itRemove = FilterList.begin() + ActiveFilterIndex;
    FilterList.erase(itRemove);
    ActiveFilterIndex = -1;
    this->Modified();
  }
  void RemoveAllFilters()
  {
    FilterList.clear();
    ActiveFilterIndex = -1;
    this->Modified();
  }

  int GetNumberOfThresholdSets() { return static_cast<int>(this->FilterList.size()); }

  int GetNumberOfActiveThresholdSets()
  {
    int count = 0;
    for (int i = 0; i < static_cast<int>(this->FilterList.size()); i++)
    {
      if (this->FilterList[i]->UseFilter)
      {
        count++;
      }
    }
    return count;
  }

  void SetActiveFilterIndex(int i)
  {
    if (i != this->ActiveFilterIndex)
    {
      ActiveFilterIndex = i;
      this->Modified();
    }
  }

  void SetInvert(bool invert) { this->SetInvert(ActiveFilterIndex, invert); }

  void SetInvert(int idx, bool invert)
  {
    if (idx < 0 || idx >= static_cast<int>(FilterList.size()))
      return;
    FilterList[idx]->Invert = invert;
    this->Modified();
  }

  void SetUseFilter(int idx, int use)
  {
    if (idx < 0 || idx >= static_cast<int>(FilterList.size()))
      return;
    FilterList[idx]->UseFilter = use;
    this->Modified();
  }

  void SetMinX(double minX)
  {
    if (ActiveFilterIndex < 0 || ActiveFilterIndex >= static_cast<int>(FilterList.size()))
      return;
    FilterList[ActiveFilterIndex]->MinX = minX;
    this->Modified();
  };
  void SetMinY(double minY)
  {
    if (ActiveFilterIndex < 0 || ActiveFilterIndex >= static_cast<int>(FilterList.size()))
      return;
    FilterList[ActiveFilterIndex]->MinY = minY;
    this->Modified();
  };
  void SetMinZ(double minZ)
  {
    if (ActiveFilterIndex < 0 || ActiveFilterIndex >= static_cast<int>(FilterList.size()))
      return;
    FilterList[ActiveFilterIndex]->MinZ = minZ;
    this->Modified();
  };

  void SetMaxX(double maxX)
  {
    if (ActiveFilterIndex < 0 || ActiveFilterIndex >= static_cast<int>(FilterList.size()))
      return;
    FilterList[ActiveFilterIndex]->MaxX = maxX;
    this->Modified();
  };
  void SetMaxY(double maxY)
  {
    if (ActiveFilterIndex < 0 || ActiveFilterIndex >= static_cast<int>(FilterList.size()))
      return;
    FilterList[ActiveFilterIndex]->MaxY = maxY;
    this->Modified();
  };
  void SetMaxZ(double maxZ)
  {
    if (ActiveFilterIndex < 0 || ActiveFilterIndex >= static_cast<int>(FilterList.size()))
      return;
    FilterList[ActiveFilterIndex]->MaxZ = maxZ;
    this->Modified();
  };

  void SetUseMinX(bool useMinX)
  {
    if (ActiveFilterIndex < 0 || ActiveFilterIndex >= static_cast<int>(FilterList.size()))
      return;
    FilterList[ActiveFilterIndex]->UseMinX = useMinX;
    this->Modified();
  };
  void SetUseMinY(bool useMinY)
  {
    if (ActiveFilterIndex < 0 || ActiveFilterIndex >= static_cast<int>(FilterList.size()))
      return;
    FilterList[ActiveFilterIndex]->UseMinY = useMinY;
    this->Modified();
  };
  void SetUseMinZ(bool useMinZ)
  {
    if (ActiveFilterIndex < 0 || ActiveFilterIndex >= static_cast<int>(FilterList.size()))
      return;
    FilterList[ActiveFilterIndex]->UseMinZ = useMinZ;
    this->Modified();
  };

  void SetUseMaxX(bool useMaxX)
  {
    if (ActiveFilterIndex < 0 || ActiveFilterIndex >= static_cast<int>(FilterList.size()))
      return;
    FilterList[ActiveFilterIndex]->UseMaxX = useMaxX;
    this->Modified();
  };

  void SetUseMaxY(bool useMaxY)
  {
    if (ActiveFilterIndex < 0 || ActiveFilterIndex >= static_cast<int>(FilterList.size()))
      return;
    FilterList[ActiveFilterIndex]->UseMaxY = useMaxY;
    this->Modified();
  };

  void SetUseMaxZ(bool useMaxZ)
  {
    if (ActiveFilterIndex < 0 || ActiveFilterIndex >= static_cast<int>(FilterList.size()))
      return;
    FilterList[ActiveFilterIndex]->UseMaxZ = useMaxZ;
    this->Modified();
  };

  void SetMinRGB(/*REPLACED*/ int r, /*REPLACED*/ int g, /*REPLACED*/ int b)
  {
    if (ActiveFilterIndex < 0 || ActiveFilterIndex >= static_cast<int>(FilterList.size()))
      return;
    FilterList[ActiveFilterIndex]->MinRGB[0] = r;
    FilterList[ActiveFilterIndex]->MinRGB[1] = g;
    FilterList[ActiveFilterIndex]->MinRGB[2] = b;
    this->Modified();
  };

  void SetMaxRGB(/*REPLACED*/ int r, /*REPLACED*/ int g, /*REPLACED*/ int b)
  {
    if (ActiveFilterIndex < 0 || ActiveFilterIndex >= static_cast<int>(FilterList.size()))
      return;
    FilterList[ActiveFilterIndex]->MaxRGB[0] = r;
    FilterList[ActiveFilterIndex]->MaxRGB[1] = g;
    FilterList[ActiveFilterIndex]->MaxRGB[2] = b;
    this->Modified();
  };

  void SetUseMinRGB(bool use)
  {
    if (ActiveFilterIndex < 0 || ActiveFilterIndex >= static_cast<int>(FilterList.size()))
      return;
    FilterList[ActiveFilterIndex]->UseMinRGB = use;
    this->Modified();
  };

  void SetUseMaxRGB(bool use)
  {
    if (ActiveFilterIndex < 0 || ActiveFilterIndex >= static_cast<int>(FilterList.size()))
      return;
    FilterList[ActiveFilterIndex]->UseMaxRGB = use;
    this->Modified();
  };

  //---------------Get Properties--------------------//
  bool GetInvert()
  {
    if (ActiveFilterIndex < 0 || ActiveFilterIndex >= static_cast<int>(FilterList.size()))
      return false;
    return FilterList[ActiveFilterIndex]->Invert;
  }

  bool GetUseFilter()
  {
    if (ActiveFilterIndex < 0 || ActiveFilterIndex >= static_cast<int>(FilterList.size()))
      return false;
    bool toReturn = FilterList[ActiveFilterIndex]->UseFilter ? true : false;
    return toReturn;
  }

  double GetMinX()
  {
    if (ActiveFilterIndex < 0 || ActiveFilterIndex >= static_cast<int>(FilterList.size()))
      return 0;
    return FilterList[ActiveFilterIndex]->MinX;
  };
  double GetMinY()
  {
    if (ActiveFilterIndex < 0 || ActiveFilterIndex >= static_cast<int>(FilterList.size()))
      return 0;
    return FilterList[ActiveFilterIndex]->MinY;
  };
  double GetMinZ()
  {
    if (ActiveFilterIndex < 0 || ActiveFilterIndex >= static_cast<int>(FilterList.size()))
      return 0;
    return FilterList[ActiveFilterIndex]->MinZ;
  };

  double GetMaxX()
  {
    if (ActiveFilterIndex < 0 || ActiveFilterIndex >= static_cast<int>(FilterList.size()))
      return 0;
    return FilterList[ActiveFilterIndex]->MaxX;
  };
  double GetMaxY()
  {
    if (ActiveFilterIndex < 0 || ActiveFilterIndex >= static_cast<int>(FilterList.size()))
      return 0;
    return FilterList[ActiveFilterIndex]->MaxY;
  };
  double GetMaxZ()
  {
    if (ActiveFilterIndex < 0 || ActiveFilterIndex >= static_cast<int>(FilterList.size()))
      return 0;
    return FilterList[ActiveFilterIndex]->MaxZ;
  };

  bool GetUseMinX()
  {
    if (ActiveFilterIndex < 0 || ActiveFilterIndex >= static_cast<int>(FilterList.size()))
      return false;
    return FilterList[ActiveFilterIndex]->UseMinX;
  };
  bool GetUseMinY()
  {
    if (ActiveFilterIndex < 0 || ActiveFilterIndex >= static_cast<int>(FilterList.size()))
      return false;
    return FilterList[ActiveFilterIndex]->UseMinY;
  };
  bool GetUseMinZ()
  {
    if (ActiveFilterIndex < 0 || ActiveFilterIndex >= static_cast<int>(FilterList.size()))
      return false;
    return FilterList[ActiveFilterIndex]->UseMinZ;
  };

  bool GetUseMaxX()
  {
    if (ActiveFilterIndex < 0 || ActiveFilterIndex >= static_cast<int>(FilterList.size()))
      return false;
    return FilterList[ActiveFilterIndex]->UseMaxX;
  };

  bool GetUseMaxY()
  {
    if (ActiveFilterIndex < 0 || ActiveFilterIndex >= static_cast<int>(FilterList.size()))
      return false;
    return FilterList[ActiveFilterIndex]->UseMaxY;
  };

  bool GetUseMaxZ()
  {
    if (ActiveFilterIndex < 0 || ActiveFilterIndex >= static_cast<int>(FilterList.size()))
      return false;
    return FilterList[ActiveFilterIndex]->UseMaxZ;
  };

  bool GetUseMinRGB()
  {
    if (ActiveFilterIndex < 0 || ActiveFilterIndex >= static_cast<int>(FilterList.size()))
      return false;
    return FilterList[ActiveFilterIndex]->UseMinRGB;
  };

  bool GetUseMaxRGB()
  {
    if (ActiveFilterIndex < 0 || ActiveFilterIndex >= static_cast<int>(FilterList.size()))
      return false;
    return FilterList[ActiveFilterIndex]->UseMaxRGB;
  };

  /*REPLACED*/ int GetMaxR()
  {
    if (ActiveFilterIndex < 0 || ActiveFilterIndex >= static_cast<int>(FilterList.size()))
      return 255;
    return FilterList[ActiveFilterIndex]->MaxRGB[0];
  };
  /*REPLACED*/ int GetMaxG()
  {
    if (ActiveFilterIndex < 0 || ActiveFilterIndex >= static_cast<int>(FilterList.size()))
      return 255;
    return FilterList[ActiveFilterIndex]->MaxRGB[0];
  };
  /*REPLACED*/ int GetMaxB()
  {
    if (ActiveFilterIndex < 0 || ActiveFilterIndex >= static_cast<int>(FilterList.size()))
      return 255;
    return FilterList[ActiveFilterIndex]->MaxRGB[0];
  };

  /*REPLACED*/ int GetMinR()
  {
    if (ActiveFilterIndex < 0 || ActiveFilterIndex >= static_cast<int>(FilterList.size()))
      return 0;
    return FilterList[ActiveFilterIndex]->MinRGB[0];
  };
  /*REPLACED*/ int GetMinG()
  {
    if (ActiveFilterIndex < 0 || ActiveFilterIndex >= static_cast<int>(FilterList.size()))
      return 0;
    return FilterList[ActiveFilterIndex]->MinRGB[1];
  };
  /*REPLACED*/ int GetMinB()
  {
    if (ActiveFilterIndex < 0 || ActiveFilterIndex >= static_cast<int>(FilterList.size()))
      return 0;
    return FilterList[ActiveFilterIndex]->MinRGB[2];
  };

  // Description:
  // Transform to apply to the pts being read in for determining whether the
  // data is in/out of the ReadBounds (if LimitReadToBounds is true), or for
  // transforming data for the output (or both);  Note, the transform is
  // ignored if neither LimitReadToBounds nor TransformOutputData is true.
  void SetTransform(vtkTransform* transform);
  vtkGetObjectMacro(Transform, vtkTransform);
  void SetTransform(double elements[16]);
  void ClearTransform() { this->SetTransform(static_cast<vtkTransform*>(0)); }

  // Description:
  // Whether or not to transform the data by this->Transform for the output
  vtkBooleanMacro(TransformOutputData, bool);
  vtkSetMacro(TransformOutputData, bool);
  vtkGetMacro(TransformOutputData, bool);

protected:
  vtkPointThresholdFilter();
  ~vtkPointThresholdFilter() override;

  int ActiveFilterIndex;

  vtkTransform* Transform;
  bool TransformOutputData;

  //BTX
  std::vector<PointThreshold*> FilterList;
  //ETX

  int RequestData(vtkInformation* request, vtkInformationVector** inputVector,
    vtkInformationVector* outputVector) override;

private:
  vtkPointThresholdFilter(const vtkPointThresholdFilter&); // Not implemented
  void operator=(const vtkPointThresholdFilter&);          //Not implemented
};

#endif
