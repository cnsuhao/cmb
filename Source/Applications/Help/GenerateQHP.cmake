##=============================================================================
##
##  Copyright (c) Kitware, Inc.
##  All rights reserved.
##  See LICENSE.txt for details.
##
##  This software is distributed WITHOUT ANY WARRANTY; without even
##  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
##  PURPOSE.  See the above copyright notice for more information.
##
##=============================================================================
# Script used to generate cmbsuite.qhp file.

file(GLOB files RELATIVE "${DOCUMENTATION_DIR}" "${DOCUMENTATION_DIR}/*.*")

SET (DOCUMENTATION_FILES)

foreach (file ${files})
  set (DOCUMENTATION_FILES 
    "${DOCUMENTATION_FILES}\n          <file>${file}</file>")
endforeach (file)

configure_file(${INPUT} ${OUTPUT})
