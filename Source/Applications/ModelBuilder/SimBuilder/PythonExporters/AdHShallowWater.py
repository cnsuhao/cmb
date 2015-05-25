#=============================================================================
#
#  Copyright (c) Kitware, Inc.
#  All rights reserved.
#  See LICENSE.txt for details.
#
#  This software is distributed WITHOUT ANY WARRANTY; without even
#  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
#  PURPOSE.  See the above copyright notice for more information.
#
#=============================================================================
"""
Export script for Shallow Water 3D applications

This script:
* First runs the *surface water* export script to generate 2D .bc files
* Then runs the adh_2d_3d extrusion software
"""

import imp
import os
import shutil
import subprocess
import sys

# For debug
#print
#print 'environ:'
#print 'PYTHONPATH', os.environ.get('PYTHONPATH')
#print 'ADH_2D_3D_EXE', os.environ.get('ADH_2D_3D_EXE')
#print


import smtk


# Explicitly load adhcommon and AdHSurfaceWater modules
# So that they reload each time
module_list = ['adhcommon', 'AdHSurfaceWater']
abs_path = os.path.abspath(__file__)
abs_dir = os.path.dirname(abs_path)
for module_name in module_list:
  module_args = imp.find_module(module_name, [abs_dir])
  imp.load_module(module_name, *module_args)
adh = sys.modules.get('adhcommon')
AdHSurfaceWater = sys.modules.get('AdHSurfaceWater')


# ---------------------------------------------------------------------
def ExportCMB(spec):
    '''Entry function, called by CMB to write export files

    Returns boolean indicating success
    Parameters
    ----------
    spec: Top-level object passed in from CMB
    '''
    #print 'Enter ExportCMB()'

    # Run *surface* water exporter to generate 2D bc file
    ok = AdHSurfaceWater.ExportCMB(spec)
    if not ok:
      print 'ERROR writing 2D boundary condition file -- exiting'
      return false

    # Init scope object to get output directory and base filename
    scope = adh.init_scope(spec)

    # Override output directory if file is specified as full path
    head, tail = os.path.split(scope.output_filename)
    if head is not None:
      scope.output_directory = head
    print 'Using \"%s\" as output directory' % scope.output_directory
    #print 'output file', scope.output_filename
    root, ext = os.path.splitext(scope.output_filename)
    scope.output_basename = root
    print 'Using basename \"%s\"" for output files' % scope.output_basename


    # Get Extrusion attributes
    ext_atts = scope.manager.findAttributes('Extrusion')
    if not ext_atts:
      print 'No extrusion attributes specified -- exiting'
      return False

    # Create dictionary of <mat id, num layers>
    ext_dict = dict()
    for ext_att in ext_atts:
      item = ext_att.find('NumberOfLayers')
      int_item = smtk.attribute.to_concrete(item)
      num_layers = int_item.value(0)
      model_ent_list = ext_att.associatedEntitiesSet()
      for model_ent in model_ent_list:
        ext_dict[model_ent.id()] = num_layers
    #print 'ext_dict', ext_dict

    # Construct .mt file with layer specifications
    mt_filename = '%s.mt' % scope.output_basename
    mt_path = os.path.join(scope.output_directory, mt_filename)
    print 'Writing', mt_path
    write_complete = False
    with open(mt_filename, 'w') as f:
      for t in sorted(ext_dict.items()):
        f.write('%s %s\n' % t)
      write_complete = True

    if not write_complete:
      print 'ERROR writing %s -- exiting' % mt_path
      return False

    # Get ExportSpec attribute
    att_list = scope.export_manager.findAttributes('ExportSpec')
    if len(att_list) > 1:
      msg = 'More than one ExportSpec instance -- using first one'
      print 'WARNING:', msg
      scope.logger.addWarning(msg)
    else:
      export_spec_att = att_list[0]

    # Copy .2dm file to output_directory if needed
    mesh_file = '%s.2dm' % scope.output_basename
    mesh_path = os.path.join(scope.output_directory, mesh_file)
    if not os.path.isfile(mesh_path):
      # Look up 2dm file in export attributes
      item = export_spec_att.find('2DMeshFile')
      file_item = smtk.attribute.to_concrete(item)
      source = file_item.value(0)
      if not os.path.isfile(source):
        print 'Unable to find source mesh file at %s -- exiting' % source
        return False
      print 'Copying %s to %s' % (source, mesh_path)
      shutil.copyfile(source, mesh_path)

    # Copy host start file to output directory if needed
    hotstart_file = '%s.hot' % scope.output_basename
    hotstart_path = os.path.join(scope.output_directory, hotstart_file)
    if not os.path.isfile(hotstart_path):
      # Look up .hot file in export attributes
      item = export_spec_att.find('HotStartFile')
      file_item = smtk.attribute.to_concrete(item)
      source = file_item.value(0)
      if not os.path.isfile(source):
        print 'Unable to find source hotstart file at %s -- exiting' % source
        return False
      print 'Copying %s to %s' % (source, hotstart_path)
      shutil.copyfile(source, hotstart_path)

    # Find the adh_2d_3d executable
    adh_exe_name = 'adh_2d_3d'
    if 'win32' == sys.platform:
      adh_exe_name += '.exe'
    exe_path = None

    # Keep list of paths that were tried
    msg_list = list()

    # Look in same directory as current executable
    exe_path = None
    python_path = sys.executable
    if python_path is None:
      msg = 'Warning: executable directory not defined'
      msg_list.append(msg)
      print msg
      scope.logger.addWarning(msg)
    else:
      # Check same directory as current executable
      python_dir = os.path.dirname(python_path)
      try_path = os.path.join(python_dir, adh_exe_name)
      if os.path.isfile(try_path):
        exe_path = try_path
      else:
        msg = 'Extrusion executable not found at \"%s\"' % try_path
        msg_list.append(msg)
        scope.logger.addDebug(msg)

      # Check platform-specific install paths
      if exe_path is None and 'darwin' == sys.platform:
        try_path = os.path.join(python_dir, os.pardir, 'bin', adh_exe_name)
        if os.path.isfile(try_path):
          exe_path = try_path
        else:
          msg = 'Extrusion executable not found at \"%s\"' % try_path
          msg_list.append(msg)
          scope.logger.addDebug(msg)

    # Also check env var, which was added for testing
    if exe_path is None:
      try_path = os.environ.get('ADH_2D_3D_EXE')
      if try_path is not None:
        if os.path.isfile(try_path):
          exe_path = try_path
        else:
          msg = 'Extrusion executable not found at \"%s\"' % try_path
          msg_list.append(msg)
          scope.logger.addDebug(msg)

    if exe_path is None:
      msg = "Cannot locate extrusion executable (%s) -- aborting" % \
        adh_exe_name
      print msg
      for msg in msg_list:
        print msg
      scope.logger.addError(msg)
      return False


    msg = 'Using extrusion executable at %s' % exe_path
    print msg
    scope.logger.addDebug(msg)

    # Save current working directory
    orig_cwd = os.getcwd()

    # Switch to output_directory
    os.chdir(scope.output_directory)

    # Open temp files to capture stdout, stderr from adh_2d_3d executable
    filename = '%s.stderr' % adh_exe_name
    path = os.path.join(scope.output_directory, filename)
    stderr_file = open(path, 'w')
    filename = '%s.stdout' % adh_exe_name
    path = os.path.join(scope.output_directory, filename)
    stdout_file = open(path, 'w')

    # Run the extrusion code
    print 'Run', exe_path
    ret = subprocess.call([exe_path, scope.output_basename], \
      stdout=stdout_file, stderr=stderr_file)

    # Close the temp files
    stdout_file.close()
    stderr_file.close()

    if ret != 0:
      print 'ERROR: %s returned errors.' % adh_exe_name,
      print ' Check .stdout & .stdout files in output directory'

    # Switch back to original working directory
    os.chdir(orig_cwd)

    print 'Wrote files to directory', scope.output_directory
    return ok
