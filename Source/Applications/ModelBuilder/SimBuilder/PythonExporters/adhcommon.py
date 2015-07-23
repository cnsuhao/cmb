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
Common functions for all ADH exporters
* surface water, aka shallow water 2D, aka ADH 2D
* shallow water
* ground water, aka GEOTACS, aka CTB

"""
import os
import smtk

# Mapping from VTK's tet face ordering to ADH's tet face
# ordering. Note that it also includes the c to fortran indexing.
vtkToAdhFaceMapping = [3, 1, 2, 4]

class CardType:
  '''Enumeration of ADH card types
  '''
  VAL   = 'val'    # value
  IDVAL = 'idval'  # id plus value
  BC    = 'bc'     # boundary condition format
  MULTIVAL = 'multival'  # output spans multiple items
  CONBC = 'conbc'   # "constituent" BC (i.e., includes constituent index)
  IDCONVAL = 'idconval'  # material item that references consituent with a value


# ---------------------------------------------------------------------
class CardFormat:
  '''Format information for one card and corresponding attribute item
  '''
  def __init__(self, item_name, opcode, comment, subitem_names, custom_writer):
    self.card_type = None
    self.item_name = item_name
    self.opcode = opcode
    self.comment = comment
    self.subitem_names = subitem_names
    self.custom_writer = custom_writer

  @classmethod
  def val(cls, item_name, opcode, comment=None, subitem_names=None, \
      custom_writer=None):
    '''Returns CardFormat instance initialized as type VAL
    '''
    info = cls(item_name, opcode, comment, subitem_names, custom_writer)
    info.card_type = CardType.VAL
    return info

  @classmethod
  def idval(cls, item_name, opcode, comment=None, subitem_names=None, \
      custom_writer=None):
    '''Returns CardFormat instance initialized as type IDVAL
    '''
    info = cls(item_name, opcode, comment, subitem_names, custom_writer)
    info.card_type = CardType.IDVAL
    return info

  @classmethod
  def bc(cls, item_name, opcode, comment=None, subitem_names=None, \
      custom_writer=None):
    '''Returns CardFormat instance initialized as type BC
    '''
    if opcode is None:
      opcode = item_name
    info = cls(item_name, opcode, comment, subitem_names, custom_writer)
    info.card_type = CardType.BC
    return info

  @classmethod
  def multival(cls, item_names, opcode, comment=None):
    '''Returns CardFormat instance initialized as type MULTIVAL

    Note that "item_names" arg should be a LIST.
    And custom_writer not currently supported
    '''
    subitem_names = None
    custom_writer = None
    info = cls(item_names, opcode, comment, subitem_names, custom_writer)
    info.card_type = CardType.MULTIVAL
    return info

  @classmethod
  def conbc(cls, item_name, opcode, comment=None, subitem_names=None, \
      custom_writer=None):
    '''Returns CardFormat instance initialized as type CONBC
    '''
    info = cls(item_name, opcode, comment, subitem_names, custom_writer)
    info.card_type = CardType.CONBC
    return info

  @classmethod
  def idconval(cls, item_name, opcode, comment=None, subitem_names=None, \
      custom_writer=None):
    '''Returns CardFormat instance initialized as type IDCONVAL
    '''
    info = cls(item_name, opcode, comment, subitem_names, custom_writer)
    info.card_type = CardType.IDCONVAL
    return info


# ---------------------------------------------------------------------
# Returns integer id for input string, intended to be used with UUID
# strings. The ExportScope object should be pass in as the first argument.
def get_unique_id(self, tag):
  '''
  '''
  if not hasattr(self, 'uid_dict'):
    self.uid_dict = dict()

  uid = self.uid_dict.get(tag)
  if uid is None:
    uid = len(self.uid_dict)
    self.uid_dict[tag] = uid
  return uid


# ---------------------------------------------------------------------
ExportScope = type('ExportScope', (object,), dict())
def init_scope(spec):
  '''Returns ExportScope object initialized to input spec

  Contains:
  * logger
  * manager
  * model
  * gridinfo
  * export_manager
  * output_directory
  * output_file
  '''
  scope = ExportScope()
  scope.logger = spec.getLogger()
  scope.manager = spec.getSimulationAttributes()
  scope.model = None
  if scope.manager is not None:
    scope.model = scope.manager.refModelManager()
  scope.gridinfo = None
  # if scope.model is not None:
  #   scope.gridinfo = scope.model.gridInfo()
  scope.output_filename = 'output.bc'  # default
  scope.output_directory = os.getcwd() # default
  scope.analysis_types = list()
  scope.categories = list()

  # Traverse export attributes
  scope.export_manager = spec.getExportAttributes()
  if scope.export_manager is not None:
    att_list = scope.export_manager.findAttributes('ExportSpec')
    if len(att_list) > 1:
      msg = 'More than one ExportSpec instance -- ignoring all'
      print 'WARNING:', msg
      scope.logger.addWarning(msg)
    else:
      att = att_list[0]

      item = att.find('OutputFile')
      if item is not None:
        file_item = smtk.to_concrete(item)
        scope.output_filename = file_item.value(0)

      item = att.find('OutputDirectory')
      if item is not None:
        dir_item = smtk.to_concrete(item)
        scope.output_directory = dir_item.value(0)

      item = att.find('AnalysisTypes')
      if item is not None:
        types_item = smtk.to_concrete(item)
        for i in range(types_item.numberOfValues()):
          scope.analysis_types.append(types_item.value(i))

  # Make categories set
  categories = set()
  for analysis in scope.analysis_types:
    categories.update(scope.manager.analysisCategories(analysis))
  scope.categories = list(categories)

  # Initialize ID dictionaries (material, bc, constituent, function)
  scope.material_dict = dict()
  material_index = 0
  material_att_list = scope.manager.findAttributes('Material')
  material_att_list += scope.manager.findAttributes('SolidMaterial')
  material_att_list.sort(key=lambda att:att.id())

  for material_att in material_att_list:
    # print 'material_att', material_att.name()
    # print 'associations: ', material_att.associations()
    # print 'material_att.associatedModelEntityIds', material_att.associatedModelEntityIds()

    model_ent_item = material_att.associations()
    # Skip materials not associated with any model entities
    if model_ent_item.numberOfValues() == 0:
      continue

    material_index += 1
    for i in range(model_ent_item.numberOfValues()):
      uuid_string = model_ent_item.valueAsString(i)
      model_ent_id = get_unique_id(scope, uuid_string)
      scope.material_dict[model_ent_id] = material_index
      print 'Added Material ID %d for model ent %d (%s)' % \
        (material_index, model_ent_id, uuid_string)

  scope.bc_dict = dict()
  bc_index = 0
  bc_att_list = scope.manager.findAttributes('BoundaryCondition')
  bc_att_list.sort(key=lambda bc: bc.id())
  for bc_att in bc_att_list:
    model_ent_item = bc_att.associations()
    # Skip BCs not associated with any model entities
    if model_ent_item is None:
      continue

    bc_index += 1
    scope.bc_dict[bc_att.id()] = bc_index
    #print 'Added BC ID %d for attribute %s (%d)' % \
    #    (bc_index, bc_att.name(), bc_att.id())

  # Constituent dictionary <constituent_att.id(), constituent_index>
  # i.e., key == smtk attribute id, value == output index
  scope.constituent_dict = dict()
  constituent_att_list = scope.manager.findAttributes('Constituent')
  constituent_att_list.sort(key=lambda att: att.id())
  for constituent_att in constituent_att_list:
    con_index = len(scope.constituent_dict) + 1
    scope.constituent_dict[constituent_att.id()] = con_index
    #print 'Added ID %d for constituent %s' % (con_index, constituent_att.name())

  # Define function dict only, initialized adhoc
  scope.function_dict = dict()

  return scope



# ---------------------------------------------------------------------
#
# The following functions adapted from ShallowWaterExporter.py
#
# ---------------------------------------------------------------------


# ---------------------------------------------------------------------
def write_section(scope, att_type):
  '''
  Writes one section of output file
  '''
  print 'DEBUG: Writing section for attribute type: %s' % att_type

  att_list = scope.manager.findAttributes(att_type)
  if att_list is None:
    msg = 'no %s attribute found' % att_type
    print 'WARNING:', msg
    scope.logger.addWarning(msg)
    return False

  # Sort list by id
  att_list.sort(key=lambda x: x.id())
  for att in att_list:
    #print 'att', att.name(), 'mask',
    format_list = scope.format_table.get(att.type())
    if format_list is None:
      msg = 'empty format list for %s' % att.type()
      print 'WARNING:', msg
      scope.logger.addWarning(msg)
      continue

    # Convert single format config to list
    if isinstance(format_list, (CardFormat,)):
      format_list = [format_list]

    if att.definition().associationMask() == 0x0:
      write_items(scope, att, format_list)

    model_ent_item = att.associations()
    if model_ent_item is None:
      print 'Expecting model association for attribute', att.name()
      return True

    # Special handling for material models
    #  - Skip materials not associated with any domains
    #  - Write comment line with attribute name
    if att.type() in ['Material', 'SolidMaterial']:
      scope.output.write('! material -- %s\n' % att.name())

    for i in range(model_ent_item.numberOfValues()):
      ent_uuid = model_ent_item.valueAsString(i)
      ent_id = get_unique_id(scope, ent_uuid)
      ok = write_items(scope, att, format_list, ent_id)

  return True


# ---------------------------------------------------------------------
def render_card(scope, item, card_format, context_id=None, group_index=0):
  '''Generates one line (card) of output for input item

  '''
  # Initially generate a list of strings
  output_list = list()
  output_list.append(card_format.opcode)
  if context_id is not None:
    output_list.append(context_id)

  # Append Constituent id for some cards
  constituent_item = None
  if card_format.card_type == CardType.CONBC:
    constituent_id = 0
    att = item.attribute()
    constituent_item = att.find('Constituent')
    constituent_ref = smtk.attribute.to_concrete(constituent_item)
    constituent = constituent_ref.value(0)
    if constituent is None:
      msg = 'No consitituent specified for attribute %s' % att.name()
      print 'Warning:', msg
      scope.logger.addWarning(msg)
    else:
      constituent_id = scope.constituent_dict.get(constituent.id())
    output_list.append(str(constituent_id))
  elif card_format.card_type == CardType.IDCONVAL:
    constituent_item = find_subgroup_item(item, group_index, 'Constituent')
    if constituent_item is None:
      msg = 'No consitituent subitem for %s' % item.name()
      print 'Warning:', msg
      scope.logger.addWarning(msg)
    constituent_ref = smtk.attribute.to_concrete(constituent_item)
    constituent = constituent_ref.value(0)
    if constituent is None:
      msg = 'No consitituent specified for attribute %s' % att.name()
      print 'Warning:', msg
      scope.logger.addWarning(msg)
    else:
      constituent_id = scope.constituent_dict.get(constituent.id())
    output_list.append(str(constituent_id))

  # Process item or subitems
  if card_format.subitem_names is not None:
    for name in card_format.subitem_names:
      if group_index is None:
        sub = item.find(name)
      else:
        sub = find_subgroup_item(item, group_index, name)
      if sub is None:
        msg = 'Did not find item %s' % name
        print 'WARNING:', msg
        scope.logger.addWarning(msg)
        return None

      concrete_sub = smtk.attribute.to_concrete(sub)
      sub_list = get_values_as_strings(scope, concrete_sub)
      output_list += sub_list

  else:
    item_list = get_values_as_strings(scope, item)
    output_list += item_list

  if card_format.comment is not None:
    output_list.append(card_format.comment)

  # Join output_list into one string
  output_text = ' '.join(output_list)
  output_text += '\n'
  return output_text


# ---------------------------------------------------------------------
def write_items(scope, att, format_list, ent_id=None):
  '''
  Writes items for a given attribute and list of formatters
  '''
  # Set context id
  context_id = None
  if 'BoundaryCondition' in att.types():
    context_id = scope.bc_dict.get(att.id(), 0)
  elif 'Constituent' in att.types():
    context_id = scope.constituent_dict.get(att.id(), 0)
  elif ent_id is not None:
    context_id = scope.material_dict.get(ent_id, 0)
  if context_id is not None:
    context_id = str(context_id)

  for card_format in format_list:
    #print 'card_format', card_format.item_name

    # Handle multi-value as special case
    if card_format.card_type == CardType.MULTIVAL:
      write_multivalue_card(scope, att, card_format)
      continue

    # Following logic writes one item to one card
    item = att.find(card_format.item_name)
    if item is None:
      msg = 'item %s not found' % card_format.item_name
      print 'WARNING:', msg
      scope.logger.addWarning(msg)
      continue

    if not item.isEnabled():  # skip items that aren't enabled
      continue

    # Check that item is in an enabled category
    if not item.isMemberOf(scope.categories):
      continue

    #print 'item', item.name()
    concrete_item = smtk.attribute.to_concrete(item)
    if concrete_item is None:
      msg = 'Unrecognized type for item %s' % item.name()
      print 'WARNING:', msg
      scope.logger.addWarning(msg)
      continue

    # Check for custom writer
    if card_format.custom_writer is not None:
      #print 'Calling custom writer: %s()' % card_format.custom_writer.__name__
      card_format.custom_writer(scope, concrete_item, card_format, context_id)
      continue

    # For IDCONVAL cards, call separate function
    if card_format.card_type == CardType.IDCONVAL:
      write_idconval_cards(scope, concrete_item, card_format, context_id)
      continue

    # Check if item is group with multiple sub groups
    if item.type() == smtk.attribute.Item.GROUP:
      n = concrete_item.numberOfGroups()
      for i in range(n):
        output_text = render_card(scope, concrete_item, card_format, \
          context_id=context_id, group_index=i)
        if output_text is not None:
          scope.output.write(output_text)
      continue

    output_text = render_card(scope, concrete_item, card_format, context_id)
    if output_text is not None:
      scope.output.write(output_text)
  return True


# ---------------------------------------------------------------------
def write_idconval_cards(scope, item, card_format, context_id):
  '''Writes idconval cards, one for each constituent

  Value comes either from "DefaultValue" or "IndividualValue" items.
  This function is hard-coded to a specific Item name hierachy
  '''
  # Get default value
  default_value = 0.0
  child_item = item.find('DefaultValue')
  if child_item is not None:
    value_item = smtk.attribute.to_concrete(child_item)
    default_value = value_item.value(0)

  # Create dictionary of <constituent id, value>
  # Set each entry to default_value
  val_dict = dict()
  for att_id in scope.constituent_dict.keys():
    val_dict[att_id] = default_value

  # Traverse individual values (if any) and overwrite default values
  child_item = item.find('IndividualValue')
  group_item = smtk.attribute.to_concrete(child_item)
  n = group_item.numberOfGroups()
  for i in range(n):
    constituent_ref = find_subgroup_item(group_item, i, 'Constituent')
    constituent = constituent_ref.value(0)
    constituent_id = constituent.id()

    value_item = find_subgroup_item(group_item, i, 'Value')
    value = value_item.value(0)
    val_dict[constituent_id] = value

  # Traverse constituents in *output* index order
  sorted_by_index = sorted(scope.constituent_dict.items(), key=lambda t:t[1])
  for con_id, con_index in sorted_by_index:
    val = val_dict[con_id]
    output_text = '%s %s %s %s\n' % \
      (card_format.opcode, context_id, con_index, val)
    scope.output.write(output_text)


# ---------------------------------------------------------------------
def write_multivalue_card(scope, att, card_format):
  '''Writes one card using data from a list of attribute items
  '''
  # card_format.item_name is a list for multivalue cards, so
  output_list = list()
  output_list.append(card_format.opcode)

  for item_name in card_format.item_name:
    item = att.find(item_name)
    if item is None:
      msg = 'item %s not found' % card_format.item_name
      print 'WARNING:', msg
      scope.logger.addWarning(msg)
      return False

    concrete_item = smtk.attribute.to_concrete(item)
    output_list += get_values_as_strings(scope, concrete_item)

  # Join output_list into one string
  output_string = ' '.join(output_list)
  scope.output.write(output_string)
  scope.output.write('\n')
  return True


# ---------------------------------------------------------------------
def get_function_id(scope, exp_att):
  '''
  Returns function #ID for smtk.attribute.Attribute of type SimExpression,
  creating one if needed.
  '''
  name = exp_att.name()
  fcn_id = scope.function_dict.get(name)
  if fcn_id is None:
    fcn_id = len(scope.function_dict) + 1
    #print 'Assign %d to function %s' % (fcn_id, name)
    scope.function_dict[name] = fcn_id
  return fcn_id


# ---------------------------------------------------------------------
def write_functions(scope):
  '''
  Writes functions, using info captured in scope.function_dict
  '''
  if not scope.function_dict:  # empty
    return

  # Get TimestepSize expression
  timestep_expression = None
  time_att = scope.manager.findAttribute('Time')
  if time_att:
    item = time_att.find('TimestepSize')
    if item:
      ts_item = smtk.attribute.to_concrete(item)
      if ts_item.isExpression(0):
        timestep_expression = ts_item.expression(0)

  fcn_list = scope.manager.findAttributes('SimExpression')
  fcn_list.sort(key=lambda f: f.id())
  for f in fcn_list:
    # Skip special case of TimeSeriesData
    if f.type() == 'TimeSeriesData':
      continue

    if f.type() != 'PolyLinearFunction':
      msg = 'Unrecognized function type %s - not written' % f.type()
      print 'WARNING:', msg
      scope.logger.addWarning(msg)
      continue

    name = f.name()
    fcn_id = scope.function_dict.get(name)
    if fcn_id is None:
      continue

    scope.output.write('! function -- %s\n' % name)

    # Check if this is the TimeStepSize expression
    # Compare ids since they are wrapped objects
    if timestep_expression and timestep_expression.id() == f.id():
      scope.output.write('XYT 1 1.0e20\n')

    val_pairs_item = f.find('ValuePairs')
    val_pairs_group = smtk.attribute.GroupItem.CastTo(val_pairs_item)
    x_item = val_pairs_group.find('X')
    x_item = smtk.attribute.DoubleItem.CastTo(x_item)
    val_item = val_pairs_group.find('Value')
    val_item = smtk.attribute.DoubleItem.CastTo(val_item)
    num_vals = x_item.numberOfValues()
    scope.output.write('XY1 %d %d 0 0\n' % (fcn_id, num_vals))
    for i in range(num_vals):
      scope.output.write('%f %f\n' % (x_item.value(i), val_item.value(i)))


# ---------------------------------------------------------------------
def write_MTS_cards(scope):
    '''
    Writes material id for each domain
    '''
    mts_list = list()
    att_list = scope.manager.findAttributes('SolidMaterial')
    for att in att_list:
      #ent_list = att.associatedEntitiesSet()
      model_ent_item = att.associations()
      if model_ent_item is None:
        continue

      for i in range(model_ent_item.numberOfValues()):
        ent_uuid = model_ent_item.valueAsString(i)
        ent_id = get_unique_id(scope, ent_uuid)
        material_id = scope.material_dict.get(ent_id, 0)
        #print 'Retrieved material id %d for entity %d' % \
        #  (material_id, ent_id)
        t = (material_id, ent_id)
        mts_list.append(t)
    mts_list.sort()
    for t in mts_list:
      scope.output.write('MTS %d %d\n' % t)


# ---------------------------------------------------------------------
def write_bc_sets(scope):
  '''
  Writes node/element set for each boundary condition attribute
  '''
  #print 'items', BC_ID_dict.items()
  bc_list = sorted(scope.bc_dict.items())
  #print 'sorted', bc_list

  # Get model dimension
  api_status = smtk.model.GridInfo.ApiStatus()

  print 'Todo get dimension from gridInfo'
  dimension = 2
  """
  dimension = scope.gridinfo.dimension(api_status)
  """

  for bc_id, bc_index in bc_list:
    bc_att = scope.manager.findAttribute(bc_id)
    #print 'nds for att, id, index', bc_att.name(), bc_id, bc_index
    if bc_att.definition().isNodal():
      write_NDS_cards(scope, bc_att)
    elif 2 == dimension:
      write_EGS_cards(scope, bc_att)
    elif 3 == dimension:
      write_FCS_cards(scope, bc_att)


# ---------------------------------------------------------------------
def write_EGS_cards(scope, bc_att):
  '''
  Writes edge element cards for given boundary condition attribute
  '''
  print 'Write EGS cards for att ', bc_att.name()
  bc_id = scope.bc_dict.get(bc_att.id())
  if bc_id is None:
    print 'WARNING: No id found for BC %s' % bc_att.name()

  # Get grid items
  api_status = smtk.model.GridInfo.ApiStatus()
  grid_item_set = set()
  #model_ent_list = bc_att.associatedEntitiesSet()
  model_ent_item = bc_att.associations()
  if model_ent_item is None:
    return

  for i in range(model_ent_item.numberOfValues()):
    ent_uuid = model_ent_item.valueAsString(i)
    ent_id = get_unique_id(scope, ent_uuid)

    print 'Todo need grid edges for model entity ', ent_id
    """
    ent_grid_items = scope.gridinfo.edgeGridItems(ent_id, api_status)
    #print 'grid items', len(ent_grid_items)
    for grid_item in ent_grid_items:
      scope.output.write('EGS %d %d %d\n' % \
        (grid_item[0]+1, grid_item[1]+1, bc_id))
    """

# ---------------------------------------------------------------------
def write_FCS_cards(scope, bc_att):
  '''
  Writes face element cards for given boundary condition attribute
  '''
  bc_id = scope.bc_dict.get(bc_att.id())
  if bc_id is None:
    print 'WARNING: No id found for BC %s' % bc_att.name()

  # Get grid items
  api_status = smtk.model.GridInfo.ApiStatus()
  grid_item_set = set()
  model_ent_item = bc_att.associations()
  if model_ent_item is None:
    return

  for i in range(model_ent_item.numberOfValues()):
    ent_uuid = model_ent_item.valueAsString(i)
    ent_id = get_unique_id(scope, ent_uuid)

    print 'Todo need grid boundary for model entity ', ent_id
    """
    ent_grid_items = scope.gridinfo.boundaryItemsOf(model_ent.id(), api_status)
    if api_status.returnType != smtk.model.GridInfo.OK:
      msg = 'GridInfo error: %s' % api_status.errorMessage
      print 'WARNING:', msg
      scope.logger.addWarning(msg)

    #print 'model ent', model_ent.id(), 'grid_items', ent_grid_items
    for grid_item in ent_grid_items:
      adh_face_num = vtkToAdhFaceMapping[grid_item[1]]
      scope.output.write('FCS %d %d %d\n' % \
        (grid_item[0]+1, adh_face_num, bc_id))
    """

# ---------------------------------------------------------------------
def write_MID_cards(scope):
  '''
  Writes material id cards for all model regions
  '''
  #print scope.material_dict
  for ent_id, mat_index in scope.material_dict.items():
    scope.output.write('MP MID %s %s\n' % (mat_index, ent_id))


# ---------------------------------------------------------------------
def write_NDS_cards(scope, bc_att):
  '''
  Writes node cards for given boundary condition attribute
  '''
  bc_id = scope.bc_dict.get(bc_att.id())
  if bc_id is None:
    msg = 'No id found for BC %s' % bc_att.name()
    print 'WARNING:', msg
    scope.logger.addWarning(msg)
    return

  api_status = smtk.model.GridInfo.ApiStatus()
  node_id_set = set()
  model_ent_item = bc_att.associations()
  if model_ent_item is None:
    return

  for i in range(model_ent_item.numberOfValues()):
    ent_uuid = model_ent_item.valueAsString(i)
    ent_id = get_unique_id(scope, ent_uuid)

    print 'Todo need grid boundary for model entity ', ent_id
    """
    vertex_id_list = list()
    point_id_list = scope.gridinfo.pointIds(model_ent.id(), \
      smtk.model.GridInfo.ALL_POINTS, api_status)
    if api_status.returnType != smtk.model.GridInfo.OK:
      msg = 'GridInfo error: %s' % api_status.errorMessage
      print 'WARNING:', msg
      scope.logger.addWarning(msg)
      continue
    node_id_set.update(point_id_list)
    """
  for node_id in sorted(node_id_set):
    scope.output.write('NDS %d %d\n' % (node_id+1, bc_id))


# ---------------------------------------------------------------------
def get_values_as_strings(scope, item):
  '''
  Returns list of strings for input item
  '''
  output_list = list()

  # Traverse data contained in this item
  # Each datum can be a value, discrete index, or function id
  if hasattr(item, 'numberOfValues'):
    n = item.numberOfValues()
    for i in range(n):
      if hasattr(item, 'isExpression') and item.isExpression(i):
        exp_att = item.expression(i)
        exp_id = get_function_id(scope, exp_att)
        output_list.append('%d' % exp_id)
      elif hasattr(item, 'isDiscrete') and item.isDiscrete():
        index = item.discreteIndex(i)
        output_list.append('%d' % index)
      else:
        output_list.append('%s' % item.value(i))

  return output_list


# ---------------------------------------------------------------------
def find_active_child(scope, item, name):
  '''Searches item's active children for given name

  Returns None if not found
  '''
  if not hasattr(item, 'activeChildItem'):
    msg = 'Item %s has no active children' % item.name()
    print 'WARNING:', msg
    scope.logger.addWarning(msg)
    return None

  n = item.numberOfActiveChildrenItems()
  for i in range(n):
    child = item.activeChildItem(i)
    if child.name() == name:
      concrete_child = smtk.attribute.to_concrete(child)
      return concrete_child
  # If not found, return None
  return None


# ---------------------------------------------------------------------
def find_subgroup_item(group_item, group_index, item_name):
  '''Finds item in one subgroup.

  Returns None if not found.
  This function is needed because GroupItem.find(size_t, std::string)
  returns a const Item, and we need a non-const Item
  '''
  n = group_item.numberOfItemsPerGroup()
  for i in range(n):
    item = group_item.item(group_index, i)
    if item.name() == item_name:
      return smtk.attribute.to_concrete(item)
  # else
  return None
