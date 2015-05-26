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
Export script for Ground Water applications
"""

import sys
import smtk

# Check if adhcommon already loaded, and either import or reload
adh = sys.modules.get('adhcommon')
if adh is None:
  import adhcommon as adh
else:
  reload(adh)

# ---------------------------------------------------------------------
#
# Define placeholder/passthrough functions for custom writer functions
# Actual implementations, prefixed by "_", are further below
#
# ---------------------------------------------------------------------
def write_outputinterval(scope, item, card_format, context_id):
  return _write_outputinterval(scope, item, card_format, context_id)


# ---------------------------------------------------------------------
#
# Dictionary of formatters for each output card
#
# There are 3 card format types: val, idval, bc
# Arguments are (item name, opcode, comment=None, subitems=None)
# These might (should?) be migrated to adhcommon.py
#
# ---------------------------------------------------------------------
fmt = adh.CardFormat
format_table = {
  'Solvers': [
    fmt.val('MaxNonLinearIters', 'IP NIT', comment='! Maximum number of non-linear iterations'),
    fmt.val('NonLinearTolMaxNorm', 'IP NTL', comment='! Non-Linear absolute tolerance'),
    fmt.val('NonLinearTolMaxChange', 'IP ITL', comment='! Non-Linear incremental tolerance'),
    fmt.val('MaxLinearIters', 'IP MIT', comment='! Maximum number of linear iterations'),
    fmt.val('MemoryIncrementBlockSize', 'OP INC', comment='! Incremental memory size'),
    fmt.val('PreconditioningBlocks', 'OP BLK', comment='! Number of blocks per processor for pre-conditioner'),
    fmt.val('PreconditionerType', 'OP PRE', comment='! Preconditioner type'),
  ],

  'Time': [
    fmt.val('JulianDay', 'TC JUL', comment='! Julian day the simulation starts on, used for the MET file'),

    fmt.val('StartTime', 'TC T0', comment='! Starting time of the simulation', \
      subitem_names=['Value1', 'Value2']),
    fmt.val('EndTime', 'TC TF', comment='! Final time of the simulation', \
      subitem_names=['Value1', 'Value2']),
    fmt.val('TimestepSize', 'TC IDT', comment='! The XY Series that will control the time step size'),
    fmt.val('AdaptiveTimeStep', 'TC NDP  ! Non-Adaptive time steps: forces adaptive time stepping to be turned of', comment='! Non-Adaptive time steps: forces adaptive time stepping to be turned off'),
    fmt.val('OutputInterval', 'OC ', comment='! Output time interval', custom_writer=write_outputinterval),
    fmt.val('PrintAdaptedMeshes', 'OC ADP', comment='! Print out the adapted mesh'),
  ],

  'Globals': [
    fmt.multival(['RayToADHSocket', 'ADHToRaySocket'], 'OP SOC', \
      comment='! Socket numbers for adh/raycaster communication (in/out for adh)'),
    fmt.val('Gravity', 'MP G', comment='! Gravity, (m)/(hr^2)'),
    fmt.val('WaterSpecificHeat', 'MP SHW', comment='! Specific heat of water, Units = (W-hr)/(g K)'),
    fmt.val('AirSpecificHeat', 'MP SHG', comment='!  Specific heat of gas, Units = (W-hr)/(g K)'),
    fmt.val('WaterSpecificGravity', 'MP SGW', comment='! Specific gravity of water'),
    fmt.val('GasSpecificGravity', 'MP SGG', comment='! Specific gravity of gas'),
    fmt.val('WaterThermalConductivity', 'MP TKW', comment='! Thermal conductivity of water, Units = (W)/(m K)'),
    fmt.val('AirThermalConductivity', 'MP TKG', comment='! Thermal conductivity of gas, Units = (W)/(m K)'),
    fmt.val('ReferenceDensity', 'MP RHO', comment='! Reference density, g/m^3'),
    fmt.val('ReferenceViscosity', 'MP VIS', comment='! Reference viscosity, Units = ?'),
  ],

  'Material': [
    fmt.idval('Porosity', 'MP POR', comment='! Porosity'),
    fmt.idval('HydraulicConductivity', 'MP K', comment='! Hydraulic conductivity', \
      subitem_names=['Value1', 'Value2', 'Value3', 'Value4', 'Value5', 'Value6']),
    fmt.idval('PresSatCurveIndex', 'MP SAT', comment='! Pressure saturation curve, XY series number'),
    fmt.idval('PresRelCondCurveIndex', 'MP KR', comment='! Pressure relative conductivity curve, XY series number'),
    fmt.idval('SpecificStorage', 'MP SS', comment='! Specific storage'),
    fmt.idval('SolidSpecificHeat', 'MP SHS', comment='! Specific heat of solids'),
    fmt.idval('SolidSpecificGravity', 'MP SGS', comment='! Specific gravity of solids'),
    fmt.idval('Albedo', 'MP ALB', comment='! Albedo'),
    fmt.idval('BulkEmissivity', 'MP EMS', comment='! Bulk emissivity'),
    fmt.idval('DryThermalConductivity', 'MP TKD', comment='! Dry/Solid Thermal Conductivity'),
    fmt.idval('SatThermalConductivity', 'MP TKS', comment='! Sat Thermal Conductivity'),
    fmt.idval('AnisotropyTensor', 'MP TKA', comment='! Anisotropy tensor for thermal conductivity', \
      subitem_names=['Value1', 'Value2', 'Value3', 'Value4', 'Value5', 'Value6']),
    fmt.idval('FractionSandGravel', 'MP FSG', comment='! Mass fraction coarse grain (sand and gravel)'),
    fmt.idval('ResidualSaturation', 'MP RSD', comment='! Residual saturation by gravity drainage'),
    fmt.idval('QuartzFraction', 'MP QTZ', comment='! Mass fraction quartz'),
    fmt.idval('VanGenuchtenAlpha', 'MP VGA', comment='! van Genuchten curve alpha (1/L)'),
    fmt.idval('VanGenuchtenN', 'MP VGN', comment='! van Genuchten curve exponent'),
    fmt.idval('VanGenuchtenMaxCP', 'MP VGP', comment='! van Genuchten curve max capillary pressure head'),
    fmt.idval('VanGenuchtenNumXY', 'MP VGX', comment='! number of van Genuchten curve entries'),
    fmt.idval('Tortuosity', 'MP TOR', comment='! Tortuosity'),
    fmt.idval('LongitudinalDispersivity', 'MP DPL', comment='! Longitudinal dispersivity'),
    fmt.idval('TransverseDispersivity', 'MP DPT', comment='! Transverse dispersivity'),
    fmt.idval('MaxRefineLevels', 'MP ML', comment='! Refinement levels'),
    fmt.idval('FlowRefineTol', 'MP FRT', comment='! Refinement Tolerance'),
    fmt.idval('MaterialRGB', 'RGB', comment='! MaterialRGB'),
    fmt.idval('MaterialTran', 'TRAN', comment='! MaterialTran'),
    fmt.idval('MaterialEmit', 'EMIT', comment='! MaterialEmit'),
  ],

  # Boundary Conditions
  'SpecifiedHead': fmt.bc('Value', 'DB FLW', comment='! Dirichlet boundary condition for flow'),
  'SpecifiedFlux': fmt.bc('Value', 'NB FLW', comment='! Neumann boundary condition for flow'),
  'FlowInjectionWell': fmt.bc('Value', 'WL FLW', comment='! FlowInjectionWell'),
  'METData': fmt.bc('Value', 'NB MET'),
  'GroundSurfaceHeatFlux': fmt.bc('Value', 'NB HFX', comment='! Heat Flux boundary condition for temperature'),
  'RayCaster': fmt.bc('Value', 'NB RAY', comment='! RayCaster'),
  'BottomBoundaryTemp': fmt.bc('Value', 'DB TMP', comment='! Dirichlet boundary condition for temperature'),
}


# ---------------------------------------------------------------------
def ExportCMB(spec):
    '''Entry function, called by CMB to write export files

    Returns boolean indicating success
    Parameters
    ----------
    spec: Top-level object passed in from CMB
    '''
    #print 'Enter ExportCMB()'

    # Initialize scope instance to store spec values and other info
    scope = adh.init_scope(spec)
    if scope.logger.hasErrors():
      print scope.logger.convertToString()
      print 'FILES NOT WRITTEN because of errors'
      return False
    scope.format_table = format_table

    print 'Analysis types:', scope.analysis_types
    if not scope.analysis_types:
      msg = 'No analysis types selected'
      print 'WARNING:', msg
      scope.logger.addWarning(msg)
    else:
      print 'Categories:', sorted(list(scope.categories))

    if not scope.output_filename:
      msg = 'Cannot export -- output filename missing'
      print 'ERROR:', msg
      scope.logger.addError(msg)
      return False

    # Open output file and start exporting content
    completed = False
    with open(scope.output_filename, 'w') as scope.output:
      if 'Heat' in scope.categories:
        scope.output.write('OP HT\n')
      if 'Flow' in scope.categories:
        scope.output.write('OP GW\n')
      n = len(scope.constituent_dict)
      scope.output.write('OP TRN %d\n' % n)

      # Call write-content functions in specified top-level order
      att_type_list = [
        'Solvers', 'Time', 'Material', 'BoundaryCondition','Globals'
      ]
      for att_type in att_type_list:
        ok = adh.write_section(scope, att_type)
        #if not ok:
        #    break

      # Write material id cards
      adh.write_MID_cards(scope)

      # Write function attributes
      adh.write_functions(scope)

      # Write MTS cards (material id for each model domain)
      adh.write_MTS_cards(scope)

      # Write NDS & EGS cards for boundary conditions
      adh.write_bc_sets(scope)

      # Last line
      scope.output.write('END\n')
      print 'Wrote', scope.output_filename
      completed = True

    if not completed:
      print 'WARNING: Export terminated unexpectedly -- output might be invalid.'

    return completed


# ---------------------------------------------------------------------
def _write_outputinterval(scope, item, card_format, context_id):
  '''Writes OutputInterval card

  Can be be specified either of 2 ways:
    "OP INC" for fixed time interval
    "OP SRS" for interval specified by function
  '''
  if not item.isDiscrete:
    msg = 'Expected OutputInterval to be discrete item - ignoring'
    print 'ERROR:', msg
    scope.logger.addError(msg)
    return False

  output_list = list()

  # Fixed interval is index 0
  if 0 == item.discreteIndex(0):
    output_list.append('OC INT')

    # Time value
    subitem = item.activeChildItem(0)
    if 'FixedInterval' != subitem.name():
      msg = 'Unexpected subitem type \"%s\"' % subitem.name()
      print 'ERROR:', msg
      scope.logger.addError(msg)
    value_item = smtk.attribute.to_concrete(subitem)
    output_list += adh.get_values_as_strings(scope, value_item)

    # Time units
    subitem = item.activeChildItem(1)
    if 'FixedIntervalUnits' != subitem.name():
      msg = 'Unexpected subitem type \"%s\"' % subitem.name()
      print 'ERROR:', msg
      scope.logger.addError(msg)
    units_item = smtk.attribute.to_concrete(subitem)
    index = units_item.discreteIndex(0)
    output_list.append(str(index))

  # Function-specified interval is index 1
  elif 1 == item.discreteIndex(0):
    output_list.append('OC SRS')

    # Time function
    subitem = item.activeChildItem(0)
    if subitem.name() is not 'IntervalFunction':
      msg = 'Unexpected subitem type \"%s\"' % subitem.name()
      print 'ERROR:', msg
      scope.logger.addError(msg)
    value_item = smtk.attribute.to_concrete(subitem)
    output_list += adh.get_values_as_strings(scope, value_item)
  else:
    msg = 'Unexpected discrete value %d' % item.discreteIndex(0)
    print 'ERROR:', msg
    scope.logger.addError(msg)
    return False

  # Comment
  if card_format.comment is not None:
    output_list.append(card_format.comment)

  # Join output_list into one string
  output_string = ' '.join(output_list)
  scope.output.write(output_string)
  scope.output.write('\n')
  return True
