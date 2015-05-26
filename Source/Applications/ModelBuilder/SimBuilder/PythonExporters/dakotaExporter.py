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
import bisect
import math
import random
import string

import smtk

def writeMethodItem(writer, i, indent):
  if i.isEnabled():
     if i.name() == "id_method":
        writer.write("%s%s = '%s'\n"%(indent,i.name(),i.valueAsString()))
     elif i.type() == smtk.attribute.Item.Type.VOID:
        writer.write("%s%s\n"%(indent,i.name()))
     else:
        writer.write("%s%s = %s\n"%(indent,i.name(),i.valueAsString()))

def writeMethod(writer, method):
  writer.write("method\n")
  print dir(method)
  gvars = ["id_method", "max_iterations", "max_function_evaluations" ]
  lvars = list()
  for count in range(0,method.numberOfItems()):
    i = method.item(count)
    if i.name() in gvars:
      tmp = smtk.attribute.to_concrete(i)
      writeMethodItem(writer, tmp, "   ")
    else:
      lvars.append(smtk.attribute.to_concrete(i))
  writer.write("   "+method.type()+"\n")
  for i in lvars:
    writeMethodItem(writer, i, "      ")

def writeVariables(writer, vars):
  cont_var = []
  print len(vars)
  for v in vars:
    print(v.name())
    if "continuous_variable" in v.type():
      cont_var.append(v)
  count = len(cont_var)
  var_init = []
  var_min = []
  var_max = []
  var_id = []
  print count
  for c in cont_var:
    print c
    for j in range(0,c.numberOfItems()):
       i = smtk.attribute.to_concrete(c.item(j))
       if i.name() == "name":
         var_id.append(i.valueAsString())
       if i.name() == "init" and i.isEnabled():
         var_init.append(i.valueAsString())
       if i.name() == "min" and i.isEnabled():
         var_min.append(i.valueAsString())
       if i.name() == "max" and i.isEnabled():
         var_max.append(i.valueAsString())
    #print "find name"
    #name = smtk.attribute.to_concrete(c.find("name"))
    #var_id.append(name.valueAsString())
    #init = smtk.attribute.to_concrete(c.find("init"))
    #if init.isEnabled():
    #  var_init.append(init.valueAsString())
    #init = None
    #minv = smtk.attribute.to_concrete(c.find("min"))
    #if minv.isEnabled():
    #  var_min.append(minv.valueAsString())
    #minv = None
    #maxv = smtk.attribute.to_concrete(c.find("max"))
    #if maxv.isEnabled():
    #  var_max.append(maxv.valueAsString())
  print var_init
  print var_min
  print var_max
  if (len(var_id) != 0 and len(var_id) != count) or (len(var_min) != 0 and len(var_min) != count) or (len(var_max) != 0 and len(var_max) != count):
    print "ERROR: could not create the variable matrix for continious_design"
  else:
    writer.write("variables\n")
    writer.write("   continuous_design = %d\n"%(count))
    writer.write("      descriptor '%s'\n"%("' '".join(var_id)))
    if len(var_init) != 0:
      writer.write("      initial_point %s\n"%(" ".join(var_init)))
    if len(var_min) != 0:
      writer.write("      lower_bounds %s\n"%(" ".join(var_min)))
    if len(var_max) != 0:
      writer.write("      upper_bounds %s\n"%(" ".join(var_max)))

def writeInterface(writer, interfaces):
  for i in interfaces:
    writer.write("interface\n")
    items = []
    for j in range(0,i.numberOfItems()):
      t = i.item(j)
      items.append(smtk.attribute.to_concrete(t))
    type = i.type()
    type = type[0:string.find(type,'_interface')]
    writer.write("   %s\n"%type)
    for j in items:
         writer.write("      %s = '%s'\n"%(j.name(), j.valueAsString()))

def writeResponces(writer, responces):
  for r in responces:
    writer.write("responses\n")
    num_objective_functions = smtk.attribute.to_concrete(r.find("num_objective_functions"))
    if num_objective_functions.isEnabled():
      writer.write("   num_objective_functions = %s\n"%num_objective_functions.valueAsString())
    calibration_terms = smtk.attribute.to_concrete(r.find("calibration_terms"))
    if calibration_terms.isEnabled():
      calibration_data_file = smtk.attribute.to_concrete(r.find("calibration_data_file"))
      calibration_file_form = smtk.attribute.to_concrete(r.find("form"))
      if not calibration_data_file.isEnabled():
        print "ERROR: should have a calibration file"
        return
      writer.write("   calibration_terms = %s\n"%calibration_terms.valueAsString())
      writer.write("      calibration_data_file = '%s'\n"%calibration_data_file.valueAsString())
      if calibration_file_form.isEnabled():
        writer.write("        %s\n"%calibration_file_form.valueAsString())
    gradiants = smtk.attribute.to_concrete(r.find("gradiants"))
    if gradiants.valueAsString() == "None":
      writer.write("   no_gradients\n")
    elif gradiants.valueAsString() == "Analytic":
      writer.write("   analytic_gradients\n")
    else:
      writer.write("   numerical_gradients\n")
    hessians = smtk.attribute.to_concrete(r.find("hessians"))
    print hessians.valueAsString()
    if hessians.valueAsString() == "None":
      writer.write("   no_hessians\n")
    elif hessians.valueAsString() == "Analytic":
      writer.write("   analytic_hessians\n")
    else:
      writer.write("   numerical_hessians\n")

def ExportCMB(spec):
  '''
  Entry function, called by CMB to write export file
  '''
  print "Export"
  manager = spec.getSimulationAttributes()
  export_manager = spec.getExportAttributes()

  att_list = export_manager.findAttributes('ExportSpec')
  if len(att_list) < 1:
      print 'ERROR - missing ExportSpec attribute'
      return False
  spec_att = att_list[0]

  analysis_name = 'Default'  # default
  item = spec_att.find('AnalysisTypes')
  if item is not None:
    types_item = smtk.attribute.to_concrete(item)
    if types_item.isSet(0):
      analysis_name = types_item.value(0)

  output_file_name = 'output.txt'  # default
  item = spec_att.find('OutputFile')
  if item is not None:
      output_item = smtk.attribute.to_concrete(item)
      if output_item.isSet(0):
        value = output_item.value(0)
        if value != '':
          output_file_name = value

  writer = open(output_file_name, 'w')
  model = manager.refModel()

  #Write the strategy section
  strategy_att = manager.findAttribute('strategy')
  print strategy_att
  writer.write("strategy\n")
  hybrid_type = smtk.attribute.to_concrete(strategy_att.find("hybrid_type"))
  writer.write("  hybrid = " + hybrid_type.valueAsString() + "\n")
  method_list = smtk.attribute.to_concrete(strategy_att.find("method_list"))
  writer.write("     method_list = " + method_list.valueAsString()+ "\n")

  #write methods
  methods = manager.findAttributes("method")
  print len(methods)
  for m in methods:
    writeMethod(writer, m)

  #write variables
  variables = manager.findAttributes("variable")
  writeVariables(writer, variables)

  #write interface
  interfaces = manager.findAttributes("interface")
  writeInterface(writer, interfaces)

  #write responces
  responces = manager.findAttributes("responses")
  writeResponces(writer,responces)

  writer.close()

#
# Below are utility functions that are used by ExportCMB()
#

def generate_locations(grid_info, domain_id, count):
  print "get location"
  return (0,0,0)


def compute_triangle_area(grid_info, cell_id):
  print "get triangle area"
  return 0


def generate_point(grid_info, cell_id):
  print "generate point"
  return (0,0,0)
