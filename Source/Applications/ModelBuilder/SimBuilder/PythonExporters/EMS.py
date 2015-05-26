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

import smtk

def ExportCMB(spec):
  '''
  Entry function, called by CMB to write export file
  '''
  manager = spec.getSimulationAttributes()
  grid_info = spec.getAnalysisGridInfo()
  output_filename = 'output.ems'  # default

  # Get export attributes
  export_manager = spec.getExportAttributes()
  export_spec_list = export_manager.findAttributes("ExportSpec")
  if not export_spec_list:
    print 'ERROR: No ExportSpec attribute - cannot export'
    return False
  export_spec = export_spec_list[0]

  item = export_spec.find('OutputFile')
  if item is None:
    print 'OutputFile attribute not found, using default: %s' % output_filename
  else:
    file_item = smtk.attribute.to_concrete(item)
    if file_item.isSet(0):
      value = file_item.value(0)
      if value != '':
        output_filename = value
    print 'Using output filename %s' % output_filename

  writer = open(output_filename, 'w')
  model = manager.refModel()

  ############# globals section
  writer.write('[globals]\n')
  global_att = manager.findAttribute('Globals')

  # Retrieve solvers att
  solver_att = global_att.find('Solvers')

  solver_group = smtk.attribute.GroupItem.CastTo(solver_att)

  rate = solver_group.find('et_leak_rate')
  rate_val = smtk.attribute.ValueItem.CastTo(rate)
  writer.write('et_leak_rate:  ' + rate_val.valueAsString() +'\n')

  tw = solver_group.find('total_weeks')
  tw_val = smtk.attribute.ValueItem.CastTo(tw)
  writer.write('total_weeks:  ' + tw_val.valueAsString() +'\n')

  # pond information
  domains = model.findGroupItems(smtk.model.Item.FACE)
  ponds = []
  for d in domains:
    atts = d.attributes()
    for att in atts:
      if att.type() == "Pond":
        ponds.append([d,att])
        break

  if len(ponds):
    writer.write('has_ponds: true\n')
  else:
    writer.write('has_ponds: false\n')

  # This uses the name specified in SimBuilder. The user can
  # modify this such that the names aren't unique. I'm leaving
  # it this way for now so that the user can determine which
  # pond is being reference here.
  writer.write('ponds:')
  for pond in ponds:
    writer.write(' ' + pond[1].name())
  writer.write('\n')

  ############# output section
  writer.write('\n[output]\n')
  # Retrieve  output att
  output_att = global_att.find('Output')
  output_group = smtk.attribute.GroupItem.CastTo(output_att)

  file_val = smtk.attribute.ValueItem.CastTo( output_group.find('file') )
  writer.write('file:  ' + file_val.valueAsString() + '\n')

  type_val = smtk.attribute.ValueItem.CastTo( output_group.find('type') )
  writer.write('type:  ' + type_val.valueAsString() + '\n')

  # get the number of salamander's per pond since we'll need that for each pond info
  agents_att = manager.findAttribute('Agents')
  salamanders_att = agents_att.find('Salamanders')
  salamanders_group = smtk.attribute.GroupItem.CastTo(salamanders_att)
  spp = smtk.attribute.IntItem.CastTo( salamanders_group.find('initial_per_pond' ) )
  spp_val = spp.value(0)

  # Initialize random number generator
  seed_item = salamanders_group.find('random_number_seed')
  if seed_item:
    seed_intitem = smtk.attribute.IntItem.CastTo(seed_item)
    seed_val = seed_intitem.value(0)
    random.seed(seed_val)

  ############# individual pond section
  status = smtk.model.GridInfo.ApiStatus()
  pondid = 1
  model = manager.refModel()
  for pondgroup, pondatt in ponds:
    writer.write('\n[' + pondatt.name() + ']\n')
    writer.write('id:  ' + str(pondid) + '\n')
    pondid += 1
    watershed_val = smtk.attribute.ValueItem.CastTo( pondatt.find("watershed_m") )
    writer.write('watershed_m:  ' + watershed_val.valueAsString() + '\n')

    max_depth_val = smtk.attribute.ValueItem.CastTo( pondatt.find("max_depth") )
    writer.write('max_depth:  ' + max_depth_val.valueAsString() + '\n')
    domain_id = pondgroup.id()

    pond_area = compute_surface_area(grid_info, domain_id)
    writer.write('area_m: %.3f\n' % pond_area)
    cell_ids = grid_info.analysisGridCells(domain_id, status)
    writer.write('elems:')
    for id in cell_ids:
      writer.write(' ' + str(id))
    writer.write('\n')
    locations = generate_locations(grid_info, domain_id, spp_val)
    formatted = ' '.join('%d %.6f %.6f' % loc for loc in locations)
    writer.write('===============salamanders initial coordinates and cell ids: ' + formatted + '\n')

  ############# agents section
  agents_att = manager.findAttribute('Agents')
  # Retrieve salamanders att
  writer.write('\n[adults]\n')
  salamanders_att = agents_att.find('Salamanders')

  rate = salamanders_group.find('yearly_survival_rate')
  rate_val = smtk.attribute.ValueItem.CastTo(rate)
  writer.write('yearly_survival_rate:  ' + rate_val.valueAsString() + '\n')

  mpp = salamanders_group.find('max_per_pond')
  mpp_val = smtk.attribute.ValueItem.CastTo(mpp)
  writer.write('max_per_pond:  ' + mpp_val.valueAsString() + '\n')

  ma = salamanders_group.find('max_age')
  ma_val = smtk.attribute.ValueItem.CastTo(ma)
  writer.write('max_age:  ' + ma_val.valueAsString() + '\n')

  # Retrieve eggs att
  writer.write('\n[eggs]\n')
  eggs_att = agents_att.find('Eggs')
  eggs_group = smtk.attribute.GroupItem.CastTo(eggs_att)
  rate = eggs_group.find('hatch_rate')
  rate_val = smtk.attribute.ValueItem.CastTo(rate)
  writer.write('hatch_rate:  ' + rate_val.valueAsString() + '\n')

  # Retrieve larvae att
  writer.write('\n[larvae]\n')
  larvae_att = agents_att.find('Larvae')
  larvae_group = smtk.attribute.GroupItem.CastTo(larvae_att)
  rate = larvae_group.find('survival_rate')
  rate_val = smtk.attribute.ValueItem.CastTo(rate)
  writer.write('survival_rate:  ' + rate_val.valueAsString() + '\n')

  ############# weather section
  writer.write('\n[weather]\n')
  weather_att = global_att.find('Weather')
  weather_group = smtk.attribute.GroupItem.CastTo(weather_att)

  file_val = smtk.attribute.ValueItem.CastTo( weather_group.find('file') )
  writer.write('file:  ' + file_val.valueAsString() + '\n')
  rain_val = smtk.attribute.ValueItem.CastTo( weather_group.find('rain_absorption_rate') )
  writer.write('rain_absorption_rate:  ' + rain_val.valueAsString() + '\n')
  rain_val = smtk.attribute.ValueItem.CastTo( weather_group.find('rain_adjust') )
  writer.write('rain_adjust:  ' + rain_val.valueAsString() + '\n')

  print 'Wrote file %s' % output_filename

#
# Below are utility functions that are used by ExportCMB()
#

def compute_surface_area(grid_info, domain_id):
  '''
  Computes surface area for (2D) model domain
  '''
  area = 0.0
  status = smtk.model.GridInfo.ApiStatus()
  cell_id_list = grid_info.analysisGridCells(domain_id, status)
  for cell_id in cell_id_list:
    area += compute_triangle_area(grid_info, cell_id)
  return area


def generate_locations(grid_info, domain_id, count):
  '''
  Generates set of random point locations on triangle mesh representing model domain (surface)

  Arguments:
    grid_info: GridInfo object provided by CMB
    domain_id: ModelDomainItem id
    count:     Number of point locations to generate
  Return value:
    list of 3-tuples, each representing (mesh_cell_id, x_coord, y_coord) for one location
  '''
  locations = list()  # return value
  status = smtk.model.GridInfo.ApiStatus()

  # Generate list with cumulative area computed for each mesh cell
  cell_id_list = grid_info.analysisGridCells(domain_id, status)
  cum_area_list = list()
  cum_area = 0.0
  for cell_id in cell_id_list:
    cum_area += compute_triangle_area(grid_info, cell_id)
    cum_area_list.append(cum_area)

  # Select cells at random, weighted by area
  for i in range(count):
    a = random.random() * cum_area
    index = bisect.bisect(cum_area_list, a)
    cell_id = cell_id_list[index]

    # Generate point at random location inside cell
    p = generate_point(grid_info, cell_id)

    # Save tuple with (id, x, y)
    datum = (cell_id, p[0], p[1])
    locations.append(datum)

  return locations


def compute_triangle_area(grid_info, cell_id):
  '''
  Computes area of 3D triangle using standard method found in many references
  e.g. http://en.wikipedia.org/wiki/Triangle#Computing_the_area_of_a_triangle

  Arguments:
    grid_info: GridInfo object provided by CMB
    cell_id:   Mesh cell id
  Return value:
    area of triangle (float)
  '''
  # Get point coords
  status = smtk.model.GridInfo.ApiStatus()
  pt_id_list = grid_info.cellPointIds(cell_id, status)
  pts = [grid_info.pointLocation(pt_id, status) for pt_id in pt_id_list]

  # Compute area
  factor = (pts[0][0] - pts[2][0]) * (pts[1][1] - pts[0][1]) - \
    (pts[0][0] - pts[1][0]) * (pts[2][1] - pts[0][1])
  area = 0.5 * math.fabs(factor)
  return area


def generate_point(grid_info, cell_id):
  '''
  Generates point at random xy coordinate within specified mesh cell
  Uses algorithm described at http://mathworld.wolfram.com/TrianglePointPicking.html

  Arguments:
    grid_info: GridInfo object provided by CMB
    cell_id:   Mesh cell id
  Return value:
    3-tuple representing point coordinates (x, y, z)
  '''
  status = smtk.model.GridInfo.ApiStatus()

  # Get point coords
  pt_id_list = grid_info.cellPointIds(cell_id, status)
  pts = [grid_info.pointLocation(pt_id, status) for pt_id in pt_id_list]

  # Generate random parameters
  r = random.random()
  s = random.random()
  # If sum is > 1 (outside triangle), reflect back inside
  if r + s > 1.0:
    r = 1.0 - r
    s = 1.0 - s

  # Generate two weighted vectors and add to pts[0]
  vec1 = [0.0] * 3
  vec2 = [0.0] * 3
  p = pts[0]
  for i in range(3):
    vec1[i] = r * (pts[1][i] - pts[0][i])
    vec2[i] = s * (pts[2][i] - pts[0][i])
    p[i] += (vec1[i] + vec2[i])

  return p
