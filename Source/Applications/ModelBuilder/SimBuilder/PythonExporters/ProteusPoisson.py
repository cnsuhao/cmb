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
"""Export script for ProteusPoisson applications"""

import os
import sys
import smtk

# so devs don't complain about pyc files in source
sys.dont_write_bytecode = True

# ---------------------------------------------------------------------
# Format info structures & initializers
class FormatInfo:
  def __init__(self, lvalue, attribute_type, item_path, format_string='%s', \
    comment=None, write_if_none=False, insert_blank_line=True, \
    standard_format=True):
    self.lvalue = lvalue
    self.attribute_type = attribute_type
    self.item_path = item_path
    self.standard_format = standard_format
    self.args = [lvalue, attribute_type, item_path] # shorthand
    self.kwargs = {
      'comment': comment,
      'format_string': format_string,
      'write_if_none': write_if_none,
      'insert_blank_line': insert_blank_line
    }
fmt = FormatInfo

# ---------------------------------------------------------------------
#
# Output format list for physics file
#
pfile_formats = [
  fmt('name', 'physicsname', 'name', format_string='\"%s\"'),
  fmt('T', 'physicsTime', 'T'),
  fmt('bcsTimeDependent', 'bcsTimeDependent', 'bcsTimeDependent'),
  fmt('genMesh', 'gridalternate', 'genMesh'),
  fmt('LevelModelType', 'LevelModelType', 'LevelModelType', \
    format_string='Transport.%s')
]

# ---------------------------------------------------------------------
#
# Output format list for numerics file
#
nfile_formats = [
  fmt('stepController', 'time', 'time/stepController', \
    format_string='StepControl.%s', comment='Step controller class'),

  fmt('timeIntegration', 'time', 'time/timeIntegration', \
    format_string='TimeIntegration.%s', comment='Time integration class'),

  fmt('DT', 'time', 'time/timestep', comment='The time step'),

  fmt('nDTout', 'time', 'time/numtimesteps',  \
    comment = '# Number Of Output Time Steps'),

  fmt('nltol_u', 'tolerancesalternate', 'nltol_u', \
    comment='Global tolerances'),
  fmt('ltol_u', 'tolerancesalternate', 'ltol_u', \
    insert_blank_line=False),
  fmt('nl_atol_res', 'tolerancesalternate', 'nl_atol_res', \
    insert_blank_line=False),
  fmt('l_atol_res', 'tolerancesalternate', 'l_atol_res', \
    insert_blank_line=False),

  fmt('femSpaces', 'femspaces', 'femSpaces', format_string='{0:%s}', \
    comment = '# Finite element space', write_if_none=True),

  fmt('fullNewtonFlag', 'solver', 'solver/fullNewtonFlag', \
    comment='Boolean to do full Newton or modified Newton'),

  fmt('quadrature', 'quadrature', None, standard_format=False),

  fmt('multilevelNonlinearSolver', 'solver', \
    'solver/multilevelNonlinearSolver', format_string='NonlinearSolvers.%s'),

  fmt('nonlinearSmoother', 'solver', 'solver/nonlinearSmoother', \
    format_string='NonlinearSolvers.%s'),

  fmt('massLumping', 'numerics_tools', 'massLumping', \
    comment='Boolean to dump mass matrix'),

  fmt('nStagesTime', 'time', 'time/numstages', \
    comment='The number of stages for the time discretization'),

  #fmt(['nnx', 'nny', 'nnz'], 'gridalternate', \
  #    'gridinput/geometry/{nnx,nny,nnz}', \
  #  comment ='# Number of nodes in xyz'),

  fmt('nLevels', 'multilevelmeshlevels', 'nLevels', \
    comment='# Number of levels in mesh'),

  fmt('nonlinearSolverNorm', 'solver', 'solver/nonlinearSolverNorm', \
    format_string='nonlinearSolverNorm(%s)', \
    comment='Norm to use for nonlinear algebraic residual'),

  fmt('reactionLumping', 'numerics_tools', 'reactionLumping', \
    comment='Boolean to lump reaction term'),

  fmt('runCFL', 'time', 'time/runCFL', \
    comment='The maximum CFL for the time step'),

  fmt('timeOrder', 'time', 'time/timeOrder', \
    comment='The order of time discretization'),

  fmt('triangleOptions', 'gridalternate', 'triangleOptions', \
    format_string='\"%s\"',
    comment='Options string for triangle or tetGen'),

  fmt('numericalFluxType', 'numerics_tools', 'numericalFluxType', \
    format_string='NumericalFlux.%s'),

  fmt('subgridError', 'numerics_tools', 'subgridError', \
    standard_format=False, comment='# Stability or shock capturing'),

  fmt('shockCapturing', 'numerics_tools', 'shockCapturing', \
    standard_format=False)
]


# ---------------------------------------------------------------------
def ExportCMB(spec):
    '''Entry function, called by CMB to write export files

    Returns boolean indicating success
    Parameters
    ----------
    spec: Top-level object passed in from CMB
    '''
    #print 'Enter ExportCMB()'

    # Create local class to store spec values and other info
    ExportScope = type('ExportScope', (object,), dict())
    scope = ExportScope()
    scope.manager = spec.getSimulationAttributes()
    scope.model = None
    if scope.manager is not None:
      scope.model = scope.manager.refModelManager()
    #scope.gridinfo = None
    scope.nd = 3  # model dimension
    # if scope.model is not None:
    #   scope.gridinfo = scope.model.gridInfo()
    #   api_status = smtk.model.GridInfo.ApiStatus()
    #   nd = scope.gridinfo.dimension(api_status)
    #   if smtk.model.GridInfo.OK != api_status.returnType:
    #     msg = 'ERROR calling GridInfo.dimension(): %s' % \
    #       api_status.errorMessage
    #     print msg
    #     logger.addError(msg)
    #   else:
    #     scope.nd = nd
    scope.export_manager = spec.getExportAttributes()
    scope.logger = spec.getLogger()

    # Generate lookup table for <attribute definition, UI panel>
    scope.view_table = create_view_table(scope)

    # Set filename defaults
    scope.pfilename = 'poisson_p.py'
    scope.nfilename = 'poisson_n.py'


    nfile_item = find_instance(scope, 'ExportSpec', 'NumericsFile', \
      scope.export_manager)

    # Write physics file
    if nfile_item is not None:
      noext = nfile_item.value(0).split('.')[0]
      # take the numerics filename, strip the '_n' if it's there, use '_p'
      if noext.endswith('_n') or noext.endswith('_N'):
        noext = noext[:-1] + 'p'
      else:
        noext = noext + '_p'
      scope.pfilename = noext + '.py'
    success = write_pfile(scope)
    print 'Completed pfile?', success
    print 'logger has %d records, hasErrors? %s'  % \
      (scope.logger.numberOfRecords(), scope.logger.hasErrors())

    # Write numerics file
    if nfile_item is not None:
      scope.nfilename = nfile_item.value(0)
    success &= write_nfile(scope)
    print 'Completed nfile?', success
    print 'logger has %d records, hasErrors? %s'  % \
      (scope.logger.numberOfRecords(), scope.logger.hasErrors())


    print
    print 'Wrote files:', scope.pfilename, scope.nfilename

    return success


# ---------------------------------------------------------------------
def write_pfile(scope):
  '''Generates physics file

  Parameters
  ----------
  scope: ExportScope
         Top-level objects & data
  '''
  pfile_complete = False
  with open(scope.pfilename, 'w') as scope.file:
    print 'Writing ', scope.pfilename

    pfile_header = [
      '# Proteus Poisson physics file',
      '# Generated by Kitware CMB'
    ]
    write_lines(scope, pfile_header, insert_blank_line=False)

    # Write python imports
    write_lines(scope, [
      'from proteus import *',
      'from proteus.default_p import *'
      ])

    # Model dimension
    write_lines(scope, [
      '# Spatial dimension',
      'nd = %d' % scope.nd
      ])

    # todo: disabled until write_polyfile and getNativeModelName are fixed
    #write_polyfile(scope)

    # Write items specified in pfile_formats
    for info in pfile_formats:
      print '  write', info.lvalue
      if info.standard_format:
        write_format_info_instance(scope, *info.args, **info.kwargs)
      else:
        msg = 'No code to handle custom type %s' % info.lvalue
        print 'WARNING:', msg
        logger.addWarning(msg)
        continue

    # initial conditions - check timeIntegration
    time_int_item = find_instance(scope, 'time', 'time/timeIntegration')
    if time_int_item is None:
      log_missing_item(scope, 'time', 'time/timeIntegration')
    else:
      time_int = time_int_item.valueAsString(0)
      #print 'timeIntegration value: ', time_int
      if time_int == 'NoIntegration':
        ti_lines = [
          '# Steady state, so no initial conditions',
          'initialConditions = None'
        ]
        write_lines(scope, ti_lines)
      else:
          write_lines(scope, ['# TODO Initial conditions'])

    # sparse diffusion representation
    sd_item = find_instance(scope, 'numerics_other', 'othernumerics/matrix')
    print '  write sd_item', sd_item.valueAsString(0)
    sd_value = None
    sd_lines = []
    if sd_item and sd_item.valueAsString(0) == 'SparseMatrix':
      sd_value = True
      sd_lines.append('# Using sparse matrix representation')
    else:
      sd_value = False
      sd_lines.append('# Using dense matrix representation')
    ui_comment = create_ui_comment(scope, 'numerics_other', sd_item)
    sd_lines.append(ui_comment)
    sd_lines.append('sd = %s' % sd_value)
    write_lines(scope, sd_lines)

    # identity tensor
    # Is this only needed when analyticSolution used?
    #write_lines(scope, [
    #  '# Identity tensor',
    #  'Ident = numpy.identity(nd, \'d\')'
    #  ])

    # Boundary conditions
    write_bcs(scope, 'dirichletConditions', 'dirichlet_bc', \
      comment='Boundary Conditions')
    write_bcs(scope, 'fluxBoundaryConditions', 'flux_bc')
    write_bcs(scope, 'weakDirichletConditions', 'weak_dirichlet_bc')

    # Also write empty dictionaries for unavailable BC's
    bc_lines = [
      'advectiveFluxBoundaryConditions = {}',
      'diffusiveFluxBoundaryConditions = {}',
      'stressFluxBoundaryConditions = {}'
    ]
    write_lines(scope, bc_lines)

    # Initial conditions
    write_ics(scope)

    # coefficient class
    coefficent_att = find_instance(scope, 'coefficients')
    if coefficent_att:
      # Write aOfX, fOfX, nc (nd written above)
      coef_lines = []
      coef_lines.append('# Number of components')
      coef_lines.append('nc = 1')

      # TODO: 'physics pde coefficient' and 'phyiscs rhs coefficient' cannot be found
      #     I don't think either exists the in the view right now
      # aOfX_item = find_item(scope, coefficent_att, 'physics pde coefficients')
      # aOfX = aOfX_item.valueAsString(0)
      # source = string_to_source_list(aOfX, 4)
      # coef_lines.extend(['', '# a(x)'])
      # ui_comment = create_ui_comment(scope, coefficent_att.type(), aOfX_item)
      # #print 'ui_comment', ui_comment
      # coef_lines.append(ui_comment)
      # coef_lines.append('def aOfX(x):')
      # coef_lines.extend(source)
      # coef_lines.append('aOfX_dict = {0: aOfX}')

      # fOfX_item = find_item(scope, coefficent_att, 'physics rhs coefficients')
      # #coef_lines.extend(['', '# f(x)', 'def fOfX():'])
      # #coef_lines.extend(source)
      # coef_lines.extend(['', '# f(x)'])
      # ui_comment = create_ui_comment(scope, coefficent_att.type(), aOfX_item)
      # #print 'ui_comment', ui_comment
      # coef_lines.append(ui_comment)
      # coef_lines.append('def fOfX(x):')
      # fOfX = fOfX_item.valueAsString(0)
      # source = string_to_source_list(fOfX, 4)
      # coef_lines.extend(source)
      # coef_lines.append('fOfX_dict = {0: fOfX}')

      physics_item = find_item(scope, coefficent_att, 'physics')
      ui_comment = create_ui_comment(scope, 'coefficients', physics_item)
      coef_lines.extend(['', '# equation coefficients', ui_comment])

      # Gather up args and kwargs
      args = ('aOfX_dict', 'fOfX_dict', 'nc', 'nd')
      # Keyword args specified by list of tuple
      # (argumnet name, item name, argument value)
      kwargs = [
        ('timeVaryingCoefficients', 'physics time varying coefficients', None)
      ]
      find_kwargs(scope, coefficent_att, kwargs)
      argument_string = build_argument_string(args, kwargs)

      # Same as above; cannot find physics_item
      # physics = physics_item.valueAsString(0)
      # coef = 'coefficients = TransportCoefficients.%s(%s)' % \
      #   (physics, argument_string)
      # coef_lines.append(coef)

      write_lines(scope, coef_lines)
    else:
      log_missing_item(scope, 'coefficient')


    pfile_complete = True
  return pfile_complete


# ---------------------------------------------------------------------
def write_nfile(scope):
  '''
  '''
  file_complete = False
  with open(scope.nfilename, 'w') as scope.file:
    print 'Writing ', scope.nfilename

    file_header = [
      '# Proteus Poisson numerics file',
      '# Generated by Kitware CMB'
    ]
    write_lines(scope, file_header, insert_blank_line=False)

    # Get filename of the _p file. We presume it is in the same dir
    pfilename = os.path.basename(scope.pfilename)

    # Write fixed imports
    python_imports =  [
      'from proteus import *',
      'from proteus.default_n import *',
      'from %s import *' % pfilename[:-3]  # strip ".py"
    ]
    write_lines(scope, python_imports)

    for info in nfile_formats:
      print '  write', info.lvalue
      if info.standard_format:
        write_format_info_instance(scope, *info.args, **info.kwargs)
      # Custom formatting logic:
      elif 'quadrature' == info.lvalue:
        write_quadrature(scope, info)
      elif 'shockCapturing' == info.lvalue:
        write_shockCapturing(scope, info)
      elif 'subgridError' == info.lvalue:
        write_subgridError(scope, info)
      else:
        msg = 'Unrecognized custom lvalue \"%s\"' % info.lvalue
        print 'ERROR: ', msg
        scope.logger.addError(msg)

    # Handle numerics_other as special case
    alt_name_dict = { 'tolFace': 'tolFac'}
    nu_item = find_instance(scope, 'numerics_other', 'othernumerics')
    if nu_item is not None:
      ui_comment = create_ui_comment(scope, 'numerics_other', nu_item)
      nu_list = [ui_comment]
      num_items = nu_item.numberOfItemsPerGroup()
      for i in range(num_items):
        subitem = smtk.to_concrete(nu_item.item(i))
        name = subitem.name()
        value = None
        if name in alt_name_dict:
          name = alt_name_dict.get(name)
        if subitem.type() == smtk.attribute.Item.GROUP:
          value = dict()
          num_components = subitem.numberOfItemsPerGroup()
          for j in range(num_components):
            comp_item = smtk.to_concrete(subitem.item(j))
            comp_value = get_item_value(comp_item)
            if comp_value is not None:
              value[comp_item.name()] = comp_value
        else:
          value = get_item_value(subitem)

        if value is not None:
          nu_list.append('%s = %s' % (name, value))
      write_lines(scope, nu_list)

  return True


# ---------------------------------------------------------------------
def write_polyfile(scope):
  '''Retrieves polyfile name (if any) from model and checks it exists
  '''
  # Get polyfile name
  polyfile_path = None
  polyfile_name = None
  polyfile_base = None
  polyfile_comment = None

  # Check if file specified in export attributes
  polyfile_item = find_instance(scope, 'ExportSpec', 'polyfile', \
    manager=scope.export_manager)
  if polyfile_item is not None:
    polyfile_name = polyfile_item.value(0)
    if len(polyfile_name.strip()) > 0:
      polyfile_comment = '# Using polyfile specified in export dialog'
      write_lines(scope, [polyfile_comment, 'polyfile = \"%s\"' % polyfile_name])
      return

  # Check if polyfile is the input/source for the cmb model
  if scope.model is not None:
    polyfile_path = scope.model.getNativeModelName()
    if not polyfile_path:
      msg = 'No polyfile (native model name) found'
      print 'WARNING:', msg
      scope.logger.addWarning(msg)
      polyfile_comment = '# ' + msg
      polyfile_path = None

  # Validate model_name as polyfile
  if polyfile_path is not None:
    polyfile_dir, polyfile_name = os.path.split(polyfile_path)
    polyfile_base, polyfile_ext = os.path.splitext(polyfile_name)

    # If .cmb file, substitute extension
    if polyfile_ext == '.cmb':
      polyfile_ext = '.poly'
      polyfile_name = polyfile_base + polyfile_ext
      polyfile_path = os.path.join(polyfile_dir, polyfile_name)

    msg = 'Looking for file %s' % polyfile_name
    print msg
    scope.logger.addDebug(msg)

    # Make list of directories to look for polyfile
    try_dirs = []
    # Full path
    if polyfile_dir is not None and polyfile_dir != '':
      try_dirs.append(polyfile_dir)
    # Current working directory
    cwd = os.getcwd()
    if cwd not in try_dirs:
      try_dirs.append(cwd)
    # Directory specified for physics file output
    pfile_dir = os.path.dirname(scope.pfilename)
    if pfile_dir is not None and pfile_dir != '' and pfile_dir not in try_dirs:
      try_dirs.append(pfile_dir)

    polyfile_found = False
    for try_dir in try_dirs:
      try_path = os.path.join(try_dir, polyfile_name)
      if os.path.isfile(try_path):
        polyfile_found = True
        #polyfile_comment = '# Found polyfile \"%s\" in directory %s' % \
        #  (polyfile_name, try_dir)
        # Sadly, leave path out of comment so that we can run tests
        polyfile_comment = '# Input polyfile'

    if not polyfile_found:
      msg = 'Unable to find polyfile \"%s\" in directories: %s' % \
        (polyfile_name, ', '.join(try_dirs))
      print 'WARNING:', msg
      polyfile_comment = '# Unable to find polyfile \"%s\" in directories:\n' % \
        polyfile_name
      for try_dir in try_dirs:
        polyfile_comment = polyfile_comment + '# %s\n' % try_dir
      polyfile_name = None

  polyfile_lines = [polyfile_comment]
  if polyfile_name is None:
    polyfile_lines.append('domain = None')
  else:
    polyfile_lines.extend([
      'domain = Domain.PiecewiseLinearComplexDomain()',
      'domain.readPoly(\"%s\")' % polyfile_base
      ])
  polyfile_lines.append('movingDomain = None')
  write_lines(scope, polyfile_lines)

# ---------------------------------------------------------------------
def write_bcs(scope, att_type, function_name, variable_name=None, comment=None):
  '''
  Writes one combined function for all boundary conditions of given type

  Parameters
  ----------
  att_type: string
  function_name: string
                  the name used in the function definition
  variable_name: string
                 the identifier that is assigned the function name
                 default is att_type
  comment: string optional
  '''
  bc_atts = scope.manager.findAttributes(att_type)
  #print 'bc_atts for type ', att_type, ': ', bc_atts
  if not bc_atts:
    return
  # Sort by attribute's id for consistent output
  bc_atts.sort(key=lambda att: att.id())

  fcn_list = list()
  if comment is not None:
    if not comment.startswith('#'):
      comment = '# %s' % comment.lstrip()
    fcn_list.append(comment)

  ui_comment = create_ui_comment(scope, att_type)
  fcn_list.append(ui_comment)

  fcn_list.append('def %s(x, flag):' % function_name)

  fcn_count = 0
  for bc_att in bc_atts:
    # Get list of model entity ids
    model_ent_list = bc_att.associations()
    if not model_ent_list:
      msg = 'No model entities associated with attribute %s - ignoring' % \
        bc_att.name()
      scope.logger.addWarning(msg)
      print 'WARNING: ', msg
      continue

    # Fetch function body
    item = bc_att.find('functionBody')
    if not item:
      msg = '%s item is not set - ignoring' % bc_att.name()
      scope.logger.addWarning(msg)
      print 'WARNING:', msg
      continue
    else:  
      body_item = smtk.attribute.to_concrete(item)

      if not body_item.isSet(0):
        msg = '%s item is not set - ignoring' % bc_att.name()
        scope.logger.addWarning(msg)
        print 'WARNING:', msg
        continue

    body = body_item.valueAsString(0)
    if not body:
      msg = 'No function body found for attribute %s - ignoring' % \
        bc_att.name()
      scope.logger.addWarning(msg)
      print 'WARNING: ', msg
      continue

    # Generate list of model entity ids
    native_id_list = list()
    for model_ent in model_ent_list:
      native_id = get_native_id(scope, model_ent)
      native_id_list.append(int(native_id))

    # Add conditional statement for this BC (indent 1 level)
    condx = 'if' if fcn_count < 1 else 'elif'
    fcn_list.append('    %s flag in %s:' % (condx, native_id_list))

    # Add function body for this BC (indent 2 levels)
    body_list = string_to_source_list(body, initial_indent=8)
    fcn_list.extend(body_list)

    fcn_count += 1

  if variable_name is None:
    variable_name = att_type
  if fcn_count > 0:
    fcn_list.append('%s = {0: %s}' % (variable_name, function_name))
  else:
    fcn_list.append('%s = {}' % variable_name)

  write_lines(scope, fcn_list)


# ---------------------------------------------------------------------
def write_ics(scope):
  '''Writes initial conditions object
  '''
  att_type = 'initialCondition'
  ic_att = find_instance(scope, att_type)
  if ic_att is None:
    return

  comment = '# Initial conditions'
  ui_comment = create_ui_comment(scope, att_type)
  output_list = [comment, ui_comment]

  class_name = 'InitialConditionsCMB'
  # Class header
  output_list.append('class %s:' % class_name)

  # Class body
  # todo: skipped for now...no classBody?
  # item = ic_att.find('classBody')
  # body_item = smtk.to_concrete(item)
  # body_value = body_item.valueAsString(0)
  # body_list = string_to_source_list(body_value, 4)
  # output_list.extend(body_list)

  # Object
  object_init = 'initialConditione = {0: %s()}' % class_name
  output_list.append(object_init)

  write_lines(scope, output_list)


# ---------------------------------------------------------------------
def write_quadrature(scope, format_info):
  '''Writes elementQuadrature and elementBoundaryQuadrature output
  '''
  info_sets = [
    ('elementQuadrature',
      'QuadratureMethod', 'QuadratureOrder',
      '# Element quadrature'
    ),
    ('elementBoundaryQuadrature',
      'BoundaryQuadratureMethod', 'BoundaryQuadratureOrder',
      '# Boundary element quadrature'
    )
  ]

  # List of quadrature methods that take nd as input
  ND_METHOD_NAMES = (
    'CubeGaussQuadrature',
    'SimplexGaussQuadrature',
    'SimplexLobattoQuadrature'
  )

  for info_set in info_sets:
    lvalue, method_name, order_name, comment = info_set
    method_item = find_instance(scope, format_info.attribute_type, \
      method_name)
    if method_item is None:
      continue

    # Begin assembling the output lines
    output_list = list()
    ui_comment = create_ui_comment(scope, 'quadrature', method_item)
    output_list.extend([comment, ui_comment])

    # Generate constructor arguments
    method = method_item.valueAsString(0)
    kwarg_list = []
    # Some quadrature types take nd as input
    if method in ND_METHOD_NAMES:
      kwarg = ('nd', None, str(scope.nd))
      kwarg_list.append(kwarg)

    # All quadrature items may take order as input
    order_item = find_instance(scope, format_info.attribute_type, order_name)
    if order_item is not None and order_item.isEnabled():
      kwarg = ('order', None, order_item.valueAsString(0))
      kwarg_list.append(kwarg)

    arg_string = build_argument_string([], kwarg_list)
    line = '%s = %s(%s)' % (lvalue, method, arg_string)
    output_list.append(line)
    write_lines(scope, output_list)


# ---------------------------------------------------------------------
def write_shockCapturing(scope, info):
  '''Writes shockCapturing output
  '''
  # Define constructor arguments
  args = ['coefficients', 'nd']
  kwargs = [
    ('shockCapturingFlag', 'shockCapturingFlag', None),
    ('shockCapturingFactor', 'shockCapturingFactor', None),
    ('uSC', 'uSC', None),
    ('lag', 'lag', None),
    ('gradLag', 'gradLag', None),
    ('betaPower', 'betaPower', None)
  ]
  write_object(scope, info, 'ShockCapturing', args, kwargs)


# ---------------------------------------------------------------------
def write_subgridError(scope, info):
  '''Writes subgridError output
  '''
  args = ['coefficients', 'nd']
  kwargs = [
    ('stabFlag', 'stabFlag', None),
    ('lag', 'lag', None),
    ('delayLagSteps', 'delayLagSteps', None),
    ('hFactor', 'hFactor', None),
    ('g', 'g', None)
  ]
  write_object(scope, info, 'SubgridError', args, kwargs)


# ---------------------------------------------------------------------
def write_object(scope, info, module_name, args_list, kwargs_list):
  '''Writes code to initialize object of given type & arguments
  '''
  item = find_instance(scope, info.attribute_type, info.item_path)
  if item is None:
    return

  output_list = list()
  comment = info.kwargs.get('comment')
  if comment is not None:
    output_list.append(comment)
  ui_comment = create_ui_comment(scope, info.attribute_type, item)
  output_list.append(ui_comment)
  class_name = item.valueAsString(0)

  find_kwargs(scope, item, kwargs_list)
  argument_string = build_argument_string(args_list, kwargs_list)
  line = '%s = %s.%s(%s)' % \
    (info.lvalue, module_name, class_name, argument_string)
  output_list.append(line)
  write_lines(scope, output_list)


# ---------------------------------------------------------------------
def get_native_id(scope, model_entity):
  '''Retrieves the native id for given model entity

  The current logic uses the model names, checking for standard prefixes
  assigned by the vthCMBModelBuilder class. If none are found, a value
  of 0 is returned.
  '''
  # Interim version parses entity name, which is set in vtkCMBModelBuilder
  name = model_entity.name()
  prefix_list = ['DomainSet', 'boundary set ']
  for prefix in prefix_list:
    if name.startswith(prefix):
      return name[len(prefix):]

  # else
  msg = 'Unable to get native id for model entity \"%s\"' % name
  print 'WARNING:', msg
  scope.logger.addWarning(msg)
  return '0'


# ---------------------------------------------------------------------
def string_to_source_list(input, initial_indent=0):
  '''Reformats input string into list of indented python strings

  This is intended to be used with stings stored in StringItem instances.
  '''
  indent = ''
  if initial_indent > 0:
    indent_list = [' '] * initial_indent
    indent = ''.join(indent_list)

  working_input = input.strip()
  input_list = working_input.split('\n')
  output_list = list()
  for line in input_list:
    if len(line) > 0:
      output_list.append(indent + line)
    else:
      output_list.append('')  # don't indent blank lines
  return output_list


# ---------------------------------------------------------------------
def write_format_info_instance(scope, lvalue, att_type, item_path, \
  format_string='%s', comment=None, insert_blank_line=True, \
  write_if_none=False):
  '''
  '''
  debug = False
  #if lvalue == 'numericalFluxType':
  #  debug = True
  #  print 'debug on for att_type', att_type

  output_list = list()
  item = find_instance(scope, att_type, item_path)
  if item is None and not write_if_none:
    if debug: print 'item not found for', att_type, item_path
    return

  if not item.isEnabled() and not write_if_none:
    return

  if comment is not None:
    if not comment.startswith('#'):
      comment = '# %s' % comment
    output_list.append(comment)

  ui_comment = create_ui_comment(scope, att_type, item)
  if ui_comment is not None:
    output_list.append(ui_comment)

  if item is None:
    output_list.append('%s = None' % lvalue)
  else:
    if isinstance(lvalue, basestring):
      lvalue = [lvalue]

    num_values = 0
    for i in range(len(lvalue)):
      value = 'VALUE_NOT_FOUND'
      if debug:
        print 'debug value %d: %s' % (i, item.valueAsString(i))
      if item.type() == smtk.attribute.Item.VOID:
        value = True
      elif not item.isSet(i):
        continue
      else:
        value = item.valueAsString(i)
      formatted_value = format_string % value
      formatted_line = '%s = %s' % (lvalue[i], formatted_value)
      output_list.append(formatted_line)
      num_values += 1

  if num_values > 0:
    write_lines(scope, output_list, insert_blank_line)


# ---------------------------------------------------------------------
def write_item_instance(scope, att_type, item_path, format, comment=None, \
  insert_blank_line=True, write_if_none=False):
  '''Writes item for single-instance attribute

    Parameters
    ----------
    scope: ExportScope
           Top-level objects & data
    att_type
    item_path
    format: string or list (list if item has multiple values)
    comment: optional
    insert_blank_line: optional
    write_if_none: optional
  '''
  output_list = list()
  item = find_instance(scope, att_type, item_path)
  if item is None and not write_if_none:
    return

  if comment is not None:
    output_list.append(comment)

  if item is None:
    output_list.append(write_if_none)
  else:
    if isinstance(format, basestring):
      format = [format]

    for i in range(len(format)):
      value = item.valueAsString(i)
      output_list.append(format[i] % value)

  write_lines(scope, output_list, insert_blank_line)


# ---------------------------------------------------------------------
def write_lines(scope, strings, insert_blank_line=True):
  '''Writes list of strings to output stream

  Parameters
  ----------
    strings: list(str)
             the input strings to be written
             do *not* include line endings in the input strings
    scope: ExportScope
           Top-level objects & data
    insert_blank_line: boolean optional
                       when true, insert a blank line before writing strings
  '''
  if insert_blank_line:
    strings.insert(0, '')
  strings.append('')
  scope.file.write('\n'.join(strings))

def write_comment(scope, comment):
  full_line = "\n# " + comment + '\n'
  scope.file.write(full_line)


# ---------------------------------------------------------------------
def find_instance(scope, att_type, item_path=None, manager=None):
  '''Finds singleton/instance attribute or item based on inputs
  If no item_path is specified, returns the attribute  or None if not found.
  If item_path specified, returns the item or None if not found

  Parameters
  ----------
    scope: ExportScope
           Top-level objects & data
    att_type: smtk.attribute.Attribute
              the attribute type to find (should be 1 and only 1)
    item_path: string optional
               string specifying item names to traverse below attribute
    manager: smtk.attribute.Manager optional
             alternate attribute manager to use (instead of scope.manager)
  '''
  if manager is None:
    manager = scope.manager
  att_list = manager.findAttributes(att_type)
  if not att_list:
    return None

  if len(att_list) > 1:
    msg = 'Attribute type \"', att_type, '\" is NOT a singleton.'
    scope.logger.addError(msg)
    print 'ERROR: ', msg
    return None

  att = att_list[0]
  if item_path is None:
    return att

  # else
  return find_item(scope, att, item_path)


# ---------------------------------------------------------------------
def find_item(scope, att, item_path):
  '''Finds Item based on specified path

  Parameters
  ----------
    scope: ExportScope
           Top-level objects & data
    att: smtk.attribute.Attribute
    item_path: string
               specifies item names, using forward-slash as delimiter
  '''
  item = None  # return value
  # Uses the fact that Attribute and GroupItem both have find() method
  parent = att
  item_list = item_path.split('/')

  for name in item_list:
    if hasattr(parent, 'find'):  # attribute & group items
      item = parent.find(name)
    elif hasattr(parent, 'activeChildItem'):  # value items
      # Search active children
      item = None
      for i in range(parent.numberOfActiveChildrenItems()):
        if parent.activeChildItem(i).name() == name:
          item = parent.activeChildItem(i)
          break
    if item is None:
      return None
    # else
    parent = smtk.attribute.to_concrete(item)

  # Return parent because it has already been converted to concrete object
  return parent


# ---------------------------------------------------------------------
def get_item_value(item, index=0):
  '''
  TODO handle RefItem type
  '''
  value = 'VALUE_NOT_FOUND'
  if item.type() == smtk.attribute.Item.VOID:
    value = item.isEnabled()
  elif not item.isEnabled():
    value = None
  elif index > item.numberOfValues():
    value = None
  elif not item.isSet(index):
    value = None
  else:
    value = item.value(index)

  return value

# ---------------------------------------------------------------------
def find_kwargs(scope, parent, kwargs):
  '''
  Updates kwargs by retrieving items from parent instance

  Parameters
  ----------
  parent: smtk.attribute.Attribute or smtk.attribute.Item
          parent object to optional items
  kwargs: list of tuples
          format is (name, item name, value)
  '''
  for i,kwarg in enumerate(kwargs):
    kwname, item_name, kwvalue = kwarg
    item = find_item(scope, parent, item_name)
    if item and item.isEnabled():
      value = get_item_value(item)
      if value is not None:
        kwargs[i] = kwname, item_name, value


# ---------------------------------------------------------------------
def build_argument_string(args, kwargs, include_parens=False):
  '''Builds string representing arguments input to class or function

  Current code only works for scalar items (numberOfValues == 1)

  Parameters
  ----------
  args: list of values
  kwargs: list of tuples
          format is (name, item name, value)
  include_parens: boolean optional

  Does *not* include kwargs with value of None
  '''
  arg_list = []
  if include_parens:
    arg_list.append('(')

  # Positional arguments
  for arg in args:
    arg_list.append('%s' % arg)

  # Keyword arguments
  for kwtuple in kwargs:
    kwname, item_name, kwvalue = kwtuple
    if kwvalue is None:
      continue
    if isinstance(kwvalue, basestring):
      arg_list.append('%s=\"%s\"' % (kwname, kwvalue))
    else:
      arg_list.append('%s=%s' % (kwname, kwvalue))

  if include_parens:
    arg_list.append(')')

  return ', '.join(arg_list)


# ---------------------------------------------------------------------
def log_missing_item(scope, att_type, item_path=''):
  '''Constructs error message
  '''
  msg = 'Missing attribute or item, path: %s:%s' % (att_type, item_path)
  scope.logger.addError(scope.logger, msg)
  print 'ERROR - ', msg


# ---------------------------------------------------------------------
def create_view_table(scope):
  '''Constructs dictionary of <attribute type, views> for UI feedback

  Traverses TopLevel View and descendants to build lookup table
  '''
  #table = dict()
  #view = scope.manager.findTopLevelView()
  #update_view_info(table, view)

  """
  print
  print '\"attribute_type: [view_path]\"'
  def_types = table.keys()
  def_types.sort()
  for def_type in def_types:
    print '%s: %s' % (def_type, table[def_type])
    print
  print
  """
  #return table

  return scope.manager.views()

# ---------------------------------------------------------------------
def update_view_info(table, view, view_path=[]):
  '''Updates contents of table or view_path
  '''
  #print 'Enter update_view_info, view', str(view)
  if hasattr(view, 'numberOfSubViews'):
    # If a GroupView, process sub views recursively
    for i in range(view.numberOfSubViews()):
      subview = view.subView(i)
      next_view = smtk.to_concrete(subview)
      next_path = view_path[:]
      if subview.title() != '':
        next_path.append(subview.title())
      update_view_info(table, next_view, next_path)
  elif hasattr(view, 'numberOfInstances'):
    # If InstanceView, update table with attribute types
    for i in range(view.numberOfInstances()):
      instance = view.instance(i)
      att_type = instance.type()
      if att_type in table:
        print 'WARNING: Found same attribute type twice: ' + att_type
        continue
      table[instance.type()] = view_path
    #print table
  elif hasattr(view, 'numberOfDefinitions'):
    # If AttributeView, update table with definition types
    for i in range(view.numberOfDefinitions()):
      defn = view.definition(i)
      def_type = defn.type()
      if def_type in table:
        print 'WARNING: Found same definition type twice: ' + def_type
        continue
      table[def_type] = view_path
  else:
      print 'Skipping view with title ' + view.title() + ', type' + view.type()
  #print 'Exit update_view_info'

# ---------------------------------------------------------------------
def create_ui_comment(scope, att_type, item=None):
  '''
  '''
  path = None
  defn = scope.manager.findDefinition(att_type)
  if defn is None:
    return '# UI path not found for attribute type %s' % att_type

  path = scope.view_table.get(defn.type())
  while path is None:
    defn = defn.baseDefinition()
    if defn is None:
      break
    path = scope.view_table.get(defn.type())

  if path is None:
    return '# UI path not found for attribute type %s' % att_type

  comment = '# UI Tab %s:%s' % (path[0], path[1])

  if item is not None:
    label = item.definition().label()
    if not label:
      label = item.name()
    comment = comment + ', Label \"%s\"' % label

    if item.definition().advanceLevel(0) > 0:
      comment = comment + ' [Advanced]'

  return comment
