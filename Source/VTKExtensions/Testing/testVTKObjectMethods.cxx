#include "vtkInstantiator.h"
//#include "vtkSMCMB_PluginInstantiator.h"
#include "stdio.h"
#include "vtkIndent.h"
#include "vtkPoints.h"

int main(int argc, char ** argv)
  {
    int returnCode = 0;
    for(int i = 1; i < argc; i++)
     {
        vtkObject* obj = vtkInstantiator::CreateInstance(argv[i]);
        obj->Print(std::cout);
        obj->DebugOn();
        obj->DebugOff();
        if( !obj->IsA(argv[i]) )
          {
            returnCode = 1;
            std::cerr << argv[i] << "'s IsA method said it was not itself.\n";
          }
        if( obj->IsA("poodle") )
          {
            returnCode = 1;
            std::cerr << argv[i] << "'s IsA method said it was a poodle.\n";
          }
        if( !obj->IsTypeOf("vtkObject") )
          {
            returnCode = 1;
            std::cerr << argv[i] << "'s IsTypeOf method said it was not a vtkObject.\n";
          }
        if( obj->IsTypeOf("spamandeggs") )
          {
            returnCode = 1;
            std::cerr << argv[i] << "'s IsTypeOf method said it was spamandeggs.\n";
          }

        obj->Delete();
      }
    return returnCode;
  }
