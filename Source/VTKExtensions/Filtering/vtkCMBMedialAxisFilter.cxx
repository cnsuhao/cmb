//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "vtkCMBMedialAxisFilter.h"

#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkImageData.h"
#include "vtkLine.h"
#include "vtkPoints.h"

#include <opencv2/imgproc.hpp>

#include <limits>

#include <vector>
#include <map>
#include <set>
#include <algorithm>
#include <cmath>
#include <cassert>

namespace cv
{
  static bool operator<(const Point_<float>& pt1, const Point_<float>& pt2)
  {
    return pt1.x < pt2.x || (pt1.x == pt2.x && pt1.y < pt2.y);
  }
}//namespace cv

namespace
{

  struct node
  {
    cv::Point2f pt;
    double medialDistance;
    std::vector<node*> neighbors;
    node(cv::Point2f pin, double md = std::numeric_limits<float>::max())
    :pt(pin)
    {
      medialDistance = md;
    }
    node()
    :medialDistance(std::numeric_limits<float>::max())
    {
    }
  };

  struct graph
  {
  public:
    ~graph()
    {
      for(std::map<cv::Point2f, node*>::iterator iter = nodes.begin();
          iter!=nodes.end(); ++iter)
      {
        delete iter->second;
      }
      nodes.clear();
    }

    node* findNode(cv::Point2f const& pt) const
    {
      node* result = NULL;
      std::map<cv::Point2f, node*>::const_iterator iter = nodes.find(pt);
      if(iter != nodes.end())
      {
        result = iter->second;
      }
      return result;
    }

    node* addPoint(cv::Point2f const& pt)
    {
      std::map<cv::Point2f, node*>::iterator iter = nodes.find(pt);
      node* result = NULL;
      if(iter != nodes.end())
      {
        result = iter->second;
      }
      else
      {
        result = new node(pt);
        nodes[pt] = result;
      }
      return result;
    }

    void connect(node* n1, node* n2)
    {
      if(n1 == NULL || n2 == NULL || n1 == n2) return;
      if(std::find(n1->neighbors.begin(), n1->neighbors.end(), n2) ==
         n1->neighbors.end())
      {
        n1->neighbors.push_back(n2);
        assert(std::find(n1->neighbors.begin(), n1->neighbors.end(), n2) !=
               n1->neighbors.end());
      }
      if(std::find(n2->neighbors.begin(), n2->neighbors.end(), n1) ==
         n2->neighbors.end())
      {
        n2->neighbors.push_back(n1);
        assert(std::find(n2->neighbors.begin(), n2->neighbors.end(), n1) !=
               n2->neighbors.end());
      }
    }

    void clearUnconnectedNodes()
    {
      for(std::map<cv::Point2f, node*>::iterator iter = nodes.begin();
          iter!=nodes.end(); )
      {
        if(iter->second->neighbors.empty())
        {
          iter = nodes.erase(iter);
        }
        else
        {
          ++iter;
        }
      }
    }

    std::vector<node*> getLeafs() const
    {
      std::vector<node*> leafs;
      for(std::map<cv::Point2f, node*>::const_iterator iter = nodes.begin();
          iter!=nodes.end(); ++iter)
      {
        if(iter->second->neighbors.size() == 1)
        {
          leafs.push_back(iter->second);
        }
      }
      return leafs;
    }

    void removeNode(node* n)
    {
      if(n == NULL)
      {
        return;
      }
      std::map<cv::Point2f, node*>::const_iterator ci = nodes.find(n->pt);
      if(ci == nodes.end())
      {
        return;
      }
      nodes.erase(ci);
      for( std::vector<node*>::iterator vi = n->neighbors.begin();
          vi != n->neighbors.end(); ++vi)
      {
        for( std::vector<node*>::iterator vj = (*vi)->neighbors.begin();
             vj != (*vi)->neighbors.end(); )
        {
          if (*vj == n)
          {
            vj = (*vi)->neighbors.erase(vj);
          }
          else
          {
            ++vj;
          }
        }
      }
    }

    node* findMajorNodeFromLeaf(node* leaf)
    {
      if(leaf->neighbors.size() != 1)
      {
        return NULL;
      }
      bool hasMore = true;
      node* at = leaf;
      std::set<node*> visited;
      while(hasMore)
      {
        hasMore = false;
        if (at->neighbors.size() > 2)
        {
          return at;
        }
        else if(visited.find(at) != visited.end())
        {
          return NULL;
        }
        assert(at->neighbors.size() <= 2);
        visited.insert(at);
        if(at->neighbors.size() >= 1 &&
           visited.find(at->neighbors[0]) == visited.end())
        {
          at = at->neighbors[0];
          hasMore = true;
        }
        else if(at->neighbors.size() == 2 &&
                visited.find(at->neighbors[1]) == visited.end())
        {
          at = at->neighbors[1];
          hasMore = true;
        }
      }
      return NULL;
    }

    void createPolydata(vtkSmartPointer<vtkPolyData> poly,
                        double * origin, double * spacing) const
    {
      vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
      points->SetDataTypeToFloat();
      vtkSmartPointer<vtkCellArray> cells = vtkSmartPointer<vtkCellArray>::New();

      points->SetNumberOfPoints(nodes.size());

      double lpt[3] = {0.0, 0.0, 0.0};
      int count = 0;
      std::map<node*, int> toIds;
      for( std::map<cv::Point2f, node*>::const_iterator n = nodes.begin();
          n != nodes.end(); ++n )
      {
        std::cout << n->second->pt.x << "," << n->second->pt.y << "," << n->second->neighbors.size() << "," << n->second->medialDistance << std::endl;
        lpt[0] = origin[0] + n->second->pt.x*spacing[0];
        lpt[1] = origin[1] + n->second->pt.y*spacing[1];
        //lpt[2] = n->second->medialDistance/1000;
        //if(lpt[2]>30) lpt[2]=30;
        points->SetPoint(count, lpt);
        toIds[n->second] = count;
        count++;
      }

      for( std::map<node*, int>::const_iterator n = toIds.begin(); n != toIds.end(); ++n)
      {
        node* cnode = n->first;
        int cid = n->second;
        for( unsigned int o = 0; o < cnode->neighbors.size(); ++o)
        {
          int oid = toIds[cnode->neighbors[o]];
          vtkSmartPointer<vtkLine> l = vtkSmartPointer<vtkLine>::New();
          l->GetPointIds()->SetId(0, cid);
          l->GetPointIds()->SetId(1, oid);
          cells->InsertNextCell(l);
        }
      }

      poly->SetPoints(points);
      poly->SetLines(cells);
    }
  private:
    std::map<cv::Point2f, node*> nodes;

  };
}

vtkStandardNewMacro(vtkCMBMedialAxisFilter);

vtkCMBMedialAxisFilter::vtkCMBMedialAxisFilter()
:ScaleFactor(1.1)
{
  this->SetNumberOfInputPorts(2);
}

vtkCMBMedialAxisFilter::~vtkCMBMedialAxisFilter()
{
}

int
vtkCMBMedialAxisFilter::RequestData(vtkInformation *vtkNotUsed(request),
                                    vtkInformationVector ** inputVector,
                                    vtkInformationVector *  outputVector)
{
  vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);
  vtkInformation *maskInfo = inputVector[1]->GetInformationObject(0);
  vtkInformation *outPolyDataInfo = outputVector->GetInformationObject(0);
  // get the info and input data
  vtkPolyData *inputPD =
    vtkPolyData::SafeDownCast(inInfo->Get(vtkDataObject::DATA_OBJECT()));
  vtkImageData *maskData =
    vtkImageData::SafeDownCast(maskInfo->Get(vtkDataObject::DATA_OBJECT()));

  cv::Point2f maskSpacing = cv::Point2f(maskData->GetSpacing()[0],
                                        maskData->GetSpacing()[1]);
  cv::Point2f maskOrigin  = cv::Point2f(maskData->GetOrigin()[0],
                                        maskData->GetOrigin()[1]);

  std::cout << maskSpacing << std::endl;
  std::cout << maskOrigin << std::endl;

  int * s = maskData->GetDimensions();
  cv::Rect rect(0,0,s[0],s[1]);

  cv::Subdiv2D subdiv(rect);
  std::map<int, unsigned int> idToIndex;

  //std::cout << inputPD->GetNumberOfCells() << std::endl;
  for(unsigned int i = 0; i < inputPD->GetNumberOfCells(); ++i)
  {
    vtkCell * l = inputPD->GetCell(i);
    double pt1[3], pt2[3];
    l->GetPoints()->GetPoint(0, pt1);
    l->GetPoints()->GetPoint(1, pt2);
    cv::Point2f tmp(static_cast<float>((((pt1[0]+pt2[0])*0.5) - maskOrigin.x)/maskSpacing.x),
                    static_cast<float>((((pt1[1]+pt2[1])*0.5) - maskOrigin.y)/maskSpacing.y));
    int id = subdiv.insert(tmp);
    if(idToIndex.find(id) == idToIndex.end())
    {
      idToIndex[id] = i;
    }
  }

  std::vector< int > idx;
  std::vector< std::vector< cv::Point2f > > facetList;
  std::vector< cv::Point2f > facetCenters;
  subdiv.getVoronoiFacetList(idx, facetList, facetCenters);
  std::cout << facetCenters.size() << std::endl;

  graph g;
  for( unsigned int i = 0; i < facetList.size(); ++i)
  {
    int kat = idToIndex[subdiv.findNearest(facetCenters[i])];
    vtkCell * line = inputPD->GetCell(kat);
    double tpt1[3];
    line->GetPoints()->GetPoint(0, tpt1);
    double pt1[3] = {(tpt1[0]-maskOrigin.x)/maskSpacing.x,
                     (tpt1[1]-maskOrigin.y)/maskSpacing.y, 0};
    double tpt2[3];
    line->GetPoints()->GetPoint(1, tpt2);
    double pt2[3] = {(tpt2[0]-maskOrigin.x)/maskSpacing.x,
                     (tpt2[1]-maskOrigin.y)/maskSpacing.y, 0};
    double pt3[3] = {0,0,0};
    for(unsigned int j = 0; j < facetList[i].size(); ++j)
    {
      cv::Point2f pt = facetList[i][j];
      int x1 = int(round(pt.x));
      int y1 = int(round(pt.y));
      if( x1 >= 0 && x1 < s[0] && y1 >= 0 && y1 < s[1] &&
          maskData->GetScalarComponentAsFloat(x1, y1, 0, 0) == 255)
      {
        node * n = g.addPoint(pt);
        pt3[0] = pt.x; pt3[1] = pt.y;
        double t;
        double dpt = std::sqrt(vtkLine::DistanceToLine(pt3, pt1, pt2, t));
        if( dpt < n->medialDistance )
        {
          n->medialDistance = dpt;
        }
      }
    }
  }

  for( unsigned int i = 0; i < facetList.size(); ++i)
  {
    std::vector< cv::Point2f > const& f = facetList[i];
    node * n1 = g.findNode(*(f.rbegin()));
    for(unsigned int j = 0; j < f.size(); ++j)
    {
      node * n2 = g.findNode(f[j]);
      if(n1 != NULL && n2 != NULL)
      {
        g.connect(n1,n2);
      }
      n1 = n2;
    }
  }

  bool has_been_modified = true;
  while (has_been_modified)
  {
    has_been_modified = false;
    std::vector<node*> leafs = g.getLeafs();
    for(unsigned int i = 0; i < leafs.size(); ++i)
    {
      node* l = leafs[i];
      node* mp = g.findMajorNodeFromLeaf(l);
      if(mp == NULL) continue;
      double ptDist = cv::norm(l->pt - mp->pt);
      double cDist = mp->medialDistance;
      if(ptDist < ScaleFactor*cDist)
      {
        g.removeNode(l);
        has_been_modified = true;
      }
    }
  }

  vtkSmartPointer<vtkPolyData> poly = vtkSmartPointer<vtkPolyData>::New();
  g.createPolydata(poly, maskData->GetOrigin(), maskData->GetSpacing());

  vtkPolyData * outputPoly =
      vtkPolyData::SafeDownCast(outPolyDataInfo->Get(vtkDataObject::DATA_OBJECT()));
  outputPoly->DeepCopy(poly);
  return 1;
}

int
vtkCMBMedialAxisFilter::FillInputPortInformation(int port,
                                                 vtkInformation * vi)
{
  if (port == 0)
  {
    vi->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkPolyData");
    return 1;
  }
  else if (port == 1)
  {
    vi->Set(vtkAlgorithm::INPUT_IS_REPEATABLE(), 0);
    vi->Set(vtkAlgorithm::INPUT_IS_OPTIONAL(), 0);
    vi->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkImageData");
    return 1;
  }
  return 0;
}

int vtkCMBMedialAxisFilter::FillOutputPortInformation(int port, vtkInformation *info)
{
  if (port == 0)
  {
    info->Set(vtkDataObject::DATA_TYPE_NAME(), "vtkPolyData");
    return 1;
  }
  return 0;
}
