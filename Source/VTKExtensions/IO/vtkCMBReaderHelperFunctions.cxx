#include "vtkCMBReaderHelperFunctions.h"
#include <cstring>

#define MAX_LINE 512

//Helper function used to make reading lines easier
namespace ReaderHelperFunctions
  {
  //Takes an ifstream and populates the stringstream
  //line with the information from the next line
  bool readNextLine(std::ifstream& file, std::stringstream& line)
    {
    char line_cstr[MAX_LINE];
    if(file.getline(line_cstr,MAX_LINE).fail())
      {
      return false;
      }
    int str_len = strlen(line_cstr);
    //Remove endline characters
    if(line_cstr[str_len-2] == '\r')
      {
      line_cstr[str_len-2] = '\0';
      }
    else if(line_cstr[str_len-1] == '\n' || line_cstr[str_len-1] == '\r')
      {
      line_cstr[str_len-1] = '\0';
      }
    //skip blank lines
    if (strcmp(line_cstr,"") == 0)
      {
      return readNextLine(file,line);
      }
    line.clear();
    line.flush();
    line.str(line_cstr);
    return true;
    }
  //Adds an additional argument to read the first identifier
  //in the line as a command card
  bool readNextLine(std::ifstream& file, std::stringstream& line, std::string& card)
    {
    bool toReturn = readNextLine(file,line);
    line >> card;
    return toReturn;
    }
  }
