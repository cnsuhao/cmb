/*=========================================================================

Copyright (c) 1998-2005 Kitware Inc. 28 Corporate Drive, Suite 204,
Clifton Park, NY, 12065, USA.

All rights reserved. No part of this software may be reproduced,
distributed,
or modified, in any form or by any means, without permission in writing from
Kitware Inc.

IN NO EVENT SHALL THE AUTHORS OR DISTRIBUTORS BE LIABLE TO ANY PARTY FOR
DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
OF THE USE OF THIS SOFTWARE, ITS DOCUMENTATION, OR ANY DERIVATIVES THEREOF,
EVEN IF THE AUTHORS HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

THE AUTHORS AND DISTRIBUTORS SPECIFICALLY DISCLAIM ANY WARRANTIES,
INCLUDING,
BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
PARTICULAR PURPOSE, AND NON-INFRINGEMENT.  THIS SOFTWARE IS PROVIDED ON AN
"AS IS" BASIS, AND THE AUTHORS AND DISTRIBUTORS HAVE NO OBLIGATION TO
PROVIDE
MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.

=========================================================================*/
// .NAME vtkCMBMeshServerLauncher
// .SECTION Description
// Starts up a thread remus::server and reports back the host name
// and port that the server has bound to

#ifndef __vtkCMBMeshServerLauncher_h
#define __vtkCMBMeshServerLauncher_h

#include "vtkCMBMeshingModule.h" // For export macro
#include "vtkObject.h"
#include "vtkStdString.h" //needed for the HostName
#include "cmbSystemConfig.h"

namespace remus { namespace server { class Server; } }

class VTKCMBMESHING_EXPORT vtkCMBMeshServerLauncher : public vtkObject
{
public:
  //construction of this class will spawn
  //the CMBMeshServer
  vtkTypeMacro(vtkCMBMeshServerLauncher,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);
  static vtkCMBMeshServerLauncher *New();

  //create a CMBMeshServer
  //returns if we launched a server, if a server already exists
  //this will return success
  int Launch();

  //returns if we have a CMBMeshServer running
  int IsAlive();

  //kills the current mesh server if it exists;
  int Terminate();

  //get the host name of the server we created
  const char* GetHostName() const { return HostName.c_str(); }

  //get the port of the server we created
  vtkGetMacro(PortNumber,int)

protected:
  vtkCMBMeshServerLauncher();
  ~vtkCMBMeshServerLauncher();

private:
  vtkCMBMeshServerLauncher(const vtkCMBMeshServerLauncher&);  // Not implemented.
  void operator=(const vtkCMBMeshServerLauncher&);  // Not implemented.

  vtkStdString HostName;
  int PortNumber;
  bool Alive;
  remus::server::Server* Implementation;
};

#endif
