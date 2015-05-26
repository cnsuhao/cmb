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
Export script for Shallow Water 2D applications
"""
import imp
import os
import sys
import smtk

# Explicitly load adhcommon.py, so that it reloads each time
module_name = 'adhcommon'
abs_path = os.path.abspath(__file__)
abs_dir = os.path.dirname(abs_path)
module_args = imp.find_module(module_name, [abs_dir])
imp.load_module(module_name, *module_args)
adh = sys.modules.get(module_name)

# Define placeholder/passthrough functions for custom writer functions
# Actual implementations, prefixed by "_", are further below
def write_outputseries(scope, item, card_format, context_id):
  return _write_outputseries(scope, item, card_format, context_id)


# ---------------------------------------------------------------------
#
# Dictionary of formatters for each output card
#
# Card format types are: val, idval, bc, multival
# Arguments are: (item name, opcode, comment=None, subitems=None, \
#  custom_writer=None)
#
# ---------------------------------------------------------------------
fmt = adh.CardFormat
format_table = {
  'Solvers': [
    fmt.val('MemoryIncrementBlockSize', 'OP INC'),
    fmt.val('PreconditioningBlocks', 'OP BLK'),
    fmt.val('PreconditionerType', 'OP PRE'),
    fmt.val('TemporalSchemeCoefficient', 'OP TEM'),
    fmt.val('PetrovGalerkinCoefficient', 'OP TPG'),
    fmt.val('VesselMovement', 'OP BT'),
    fmt.val('VesselEntrainment', 'OP BTS'),
    fmt.val('SW2Gradients', 'OP NF2'),
    fmt.val('NonLinearTolMaxNorm', 'IP NTL'),
    fmt.val('MaxNonLinearIters', 'IP NIT'),
    fmt.val('MaxLinearIters', 'IP MIT'),
    fmt.val('NonLinearTolMaxChange', 'IP ITL'),
    fmt.val('RungeKuttaTol', 'IP RTL'),
  ],

  'Time': [
    fmt.val('TimestepSize', 'TC IDT'),
    fmt.val('StartTime', 'TC T0', subitem_names=['Value', 'Units']),
    fmt.val('EndTime', 'TC TF', subitem_names=['Value', 'Units']),
    fmt.val('SteadyStateSolveParams', 'TC STD'),
    fmt.val('QuasiUnsteadyParams', 'TC STH',
            subitem_names=['SteadyStateHydrodynamicCondition"', 'MaxIterations', 'InitialTimeStep']),
    fmt.val('AutoTimeStepFind', 'TC ATF', subitem_names=['InitialTimeStep', 'TimeSeries']),
    fmt.val('PrintAdaptedMeshes', 'PC ADP'),
    fmt.val('HotStartFile', 'PC HOT'),
    fmt.val('OutputSeries', 'OS', custom_writer=write_outputseries)
  ],

  'SolidMaterial': [
    fmt.idconval('TurbulentDiffusionRate', 'MP DF'),
    fmt.idval('KinematicEddyViscosity', 'MP EVS',
      subitem_names=['Value1', 'Value2', 'Value3']),
    fmt.idval('CoriolisLatitude', 'MP COR'),
    fmt.idval('MaxRefineLevels', 'MP ML'),
    fmt.idval('HydroRefineTol', 'MP SRT'),
    fmt.idconval('TransportRefineTol', 'MP TRT')
  ],

  # Boundary Conditions
  'VelocityBound':
    fmt.bc('DirichletVelocity', 'DB OVL', subitem_names=['Value1', 'Value2']),
  'VelocityAndDepthBound':
    fmt.bc('Dirichlet transport', 'DB TRN',
      subitem_names=['Value1', 'Value2', 'Value3']),
  'LidElevation':        fmt.bc('Value', 'DB LDE'),
  'WaterDepthLid':       fmt.bc('Value', 'DB LDH'),
  'FloatingStationary':  fmt.bc('Value', 'DB LID'),
  'TotalDischarge':      fmt.bc('Value', 'NB DIS'),
  'UnitFlow':            fmt.bc('Value', 'NB OVL'),
  'WaterSurfElev':       fmt.bc('Value', 'NB OTW'),
  'StageDischargeBound': fmt.bc('Value', 'NB SDR'),
  'SpillWayBound':       fmt.bc('Value', 'NB SPL'),
  'OutflowBound':        fmt.bc('Value', 'OB OF'),
  'DirichletTransport':  fmt.conbc('Value', 'DB TRN'),
  'NaturalTransport':    fmt.conbc('Value', 'NB TRN'),

  'FrictionType1': [ fmt.idval('Value', 'FR MNG') ],
  'FrictionType2': [ fmt.idval('Value', 'FR ERH') ],
  'FrictionType3': [ fmt.idval('Value', 'FR SAV') ],
  'FrictionType4': [ fmt.idval('Value', 'FR URV') ],

  'Globals': [
    fmt.val('Gravity', 'MP G'),
    fmt.val('KinMolViscosity', 'MP MU'),
    fmt.val('ReferenceDensity', 'MP RHO'),
    fmt.val('ManningsUnitConstant', 'MP MUC'),
    fmt.val('WetDryLimits', 'MP DTL')
  ],

  'GeneralConstituent':     [ fmt.idval('GenConstituentParams',  'CN CON',
    subitem_names=['ReferenceConcentration']) ],
  'SalinityConstituent':    [ fmt.idval('SalConstituentParams',  'CN SAL',
    subitem_names=['ReferenceConcentration']) ],
  'VorticityConstituent':   [ fmt.idval('VortConstituentParams', 'CN VOR',
    subitem_names=['NormalizationFactor', 'AsTerm', 'DsTerm']) ],
  'TemperatureConstituent': [ fmt.idval('TempConstituentParams', 'CN TEM',
    subitem_names=['ReferenceConcentration']) ],
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

    # Open output file and start exporting content
    completed = False
    with open(scope.output_filename, 'w') as scope.output:
      scope.output.write('OP SW2\n')
      n = len(scope.constituent_dict)
      scope.output.write('OP TRN %d\n' % n)

      # Call write-content functions in specified top-level order
      att_type_list = [
        'Solvers', 'Time', 'SolidMaterial', 'BoundaryCondition',
        'Friction', 'Globals', 'Constituent'
      ]
      for att_type in att_type_list:
        ok = adh.write_section(scope, att_type)
        #if not ok:
        #    break

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
def _write_outputseries(scope, item, card_format, context_id):
  '''Custom writer (implementation) for OutputSeries attribute

  Writes either OC or OS card, depending on item'd discrete value
    0 == OC (Output Control Series)
    1 == OS (Auto-Build Time Series)
  '''
  discrete_value = item.value(0)
  if 0 == discrete_value:
    output_item = adh.find_active_child(scope, item, 'OutputFunction')
    exp_att = output_item.expression(0)
    exp_id = adh.get_function_id(scope, exp_att)
    scope.output.write('OC %d\n' % exp_id)
  elif 1 == discrete_value:
    # Get function (series) ID for TimeSeriesData
    time_series_item = adh.find_active_child(scope, item, 'TimeSeriesData')
    series_id = adh.get_function_id(scope, time_series_item)

    # Get number of points
    n = time_series_item.numberOfGroups()

    # Get units
    units_item = adh.find_active_child(scope, item, 'OutputUnits')
    units_value = units_item.discreteIndex(0)

    scope.output.write('OS %d %d %d\n' % (series_id, n, units_value))

    # Process subgroup items, one line per
    if n < 1:  # but safety first
      return

    for i in range(n):
      #print 'i', i
      start_item = adh.find_subgroup_item(time_series_item, i, 'StartTime')
      start = start_item.value(0)

      end_item = adh.find_subgroup_item(time_series_item, i, 'EndTime')
      end = end_item.value(0)

      interval_item = adh.find_subgroup_item(time_series_item, i, 'TimeInterval')
      interval = interval_item.value(0)

      units_item = adh.find_subgroup_item(time_series_item, i, 'Units')
      units = units_item.value(0)

      scope.output.write('%8s %8s %8s   %s\n' % (start, end, interval, units))
  else:
    msg = 'Unexpected value for OutputSeries: %d (expecting 0 or 1)' % \
      discrete_value
    print 'WARNING:', msg
    scope.logger.addWarning(msg)
