<?xml version="1.0"?>
<SMTK_AttributeManager Version="1">
  <!--**********  Category and Analysis Infomation ***********-->
  <Categories Default="Mechanics">
    <Cat>Time</Cat>
    <Cat>Mechanics</Cat>
  </Categories>
  <Analyses>
    <Analysis Type="Solid Mechanics" Default="Mechanics">
      <Cat>Mechanics</Cat>
      <Cat>Time</Cat>
    </Analysis>
  </Analyses>
  <!--**********  Attribute Definitions ***********-->
  <Definitions>
    <!--***  Problem definition - assuming Mechanics 3D  ***-->
    <AttDef Type="ProblemDefinition" BaseType="" Version="0" Unique="" Associations="">
      <ItemDefinitions>
        <String Name="SolutionMethod" Label="Solution Method" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
          <DiscreteInfo DefaultIndex="0">
            <Value Enum="Steady">Steady</Value>
            <Value Enum="Transient">Transient</Value>
            <Value Enum="Continuation">Continuation</Value>
            <Value Enum="Multi-Problem">Multi-Problem</Value>
          </DiscreteInfo>
        </String>
        <Int Name="PhalanxGraphVisualizationDetails" Label="Phalanx Graph Visualization Detail" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
          <RangeInfo>
            <Min Inclusive="true">0</Min>
          </RangeInfo>
          <Categories>
            <Cat>Mechanics</Cat>
          </Categories>
        </Int>
      </ItemDefinitions>
    </AttDef>

    <AttDef Type="AdaptationDefinition" BaseType="" Version="0" Unique="" Associations="">
      <ItemDefinitions>
        <String Name="Method" Label="Adaptation Method" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
          <DefaultValue>Unif Size</DefaultValue>
        </String>
        <Int Name="RemeshStepNumber" Label="Remesh Step Number" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
          <DefaultValue>3</DefaultValue>
        </Int>
        <Int Name="MaxNumberOfSTKAdaptIterations" Label="Max Number of STK Adapt Iterations" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
          <DefaultValue>10</DefaultValue>
        </Int>
        <String Name="RefinerPattern" Label="Refiner Pattern" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
          <DefaultValue>Local_Tet4_Tet4_N</DefaultValue>
        </String>
        <Double Name="TargetElementSize" Label="Target Element Size" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
          <DefaultValue>0.0005</DefaultValue>
        </Double>
      </ItemDefinitions>
    </AttDef>

    <!--***  Material Definitions ***-->
    <AttDef Type="Material" BaseType="" Version="0" Unique="true" Associations="r">
      <ItemDefinitions>
        <Double Name="ElasticModulus" Label="Elastic Modulus" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
          <RangeInfo>
            <Min Inclusive="false">0</Min>
          </RangeInfo>
          <Categories>
            <Cat>Mechanics</Cat>
          </Categories>
        </Double>
        <Double Name="PoissonsRatio" Label="Poisson's Ratio" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
          <RangeInfo>
            <Min Inclusive="true">0</Min>
            <Max Inclusive="true">0.5</Max>
          </RangeInfo>
          <Categories>
            <Cat>Mechanics</Cat>
          </Categories>
        </Double>
      </ItemDefinitions>
    </AttDef>
    <AttDef Type="NeohookeanMaterial" Label="Neohookean Material" BaseType="Material" Version="0" Unique="true" Associations="r">
      <ItemDefinitions/>
    </AttDef>
    <AttDef Type="J2Material" Label="J2 Material" BaseType="Material" Version="0" Unique="true" Associations="r">
      <ItemDefinitions>
        <Double Name="YieldStrength" Label="Yield Strength" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
          <RangeInfo>
            <Min Inclusive="false">0</Min>
          </RangeInfo>
          <Categories>
            <Cat>Mechanics</Cat>
          </Categories>
        </Double>
        <Double Name="HardeningModulus" Label="Hardening Modulus" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
          <RangeInfo>
            <Min Inclusive="false">0</Min>
          </RangeInfo>
          <Categories>
            <Cat>Mechanics</Cat>
          </Categories>
        </Double>
        <Double Name="SaturationModulus" Label="Saturation Modulus" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
          <Categories>
            <Cat>Mechanics</Cat>
          </Categories>
        </Double>
        <Double Name="SaturationExponent" Label="Saturation Exponent" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
          <Categories>
            <Cat>Mechanics</Cat>
          </Categories>
        </Double>
      </ItemDefinitions>
    </AttDef>

    <!--***  BoundaryCondition Definitions ***-->
    <AttDef Type="BoundaryCondition" BaseType="" Abstract="1" Version="0" Unique="false" Associations="f" />
    <!-- BC structure to enforce uniqueness/prevent overspecification of BCs on a boundary, alternatively we
         could use allowExpressions so that the user can either set a constant value or a function which
         would be assumed to be time varying. then the exporter would be responsible for handling the different
         boundary condition types (static or dynamic) based on the value type -->

    <AttDef Type="XBoundaryCondition" BaseType="BoundaryCondition" Abstract="1" Version="0" Unique="true" Associations="f" />
    <AttDef Type="YBoundaryCondition" BaseType="BoundaryCondition" Abstract="1" Version="0" Unique="true" Associations="f" />
    <AttDef Type="ZBoundaryCondition" BaseType="BoundaryCondition" Abstract="1" Version="0" Unique="true" Associations="f" />

    <AttDef Type="StaticDOFXDirichlet" Label="Static DOF X Dirichlet" BaseType="XBoundaryCondition" Version="0" Unique="true" Nodal="true" Associations="f">
      <ItemDefinitions>
        <Double Name="Value" Label="Dirichlet for DOF X" Version="0" NumberOfRequiredValues="1">
          <DefaultValue>0</DefaultValue>
          <Categories>
            <Cat>Mechanics</Cat>
          </Categories>
        </Double>
      </ItemDefinitions>
    </AttDef>
    <AttDef Type="StaticDOFYDirichlet" Label="Static DOF Y Dirichlet" BaseType="YBoundaryCondition" Version="0" Unique="true" Nodal="true" Associations="f">
      <ItemDefinitions>
        <Double Name="Value" Label="Dirichlet for DOF Y" Version="0" NumberOfRequiredValues="1">
          <DefaultValue>0</DefaultValue>
          <Categories>
            <Cat>Mechanics</Cat>
          </Categories>
        </Double>
      </ItemDefinitions>
    </AttDef>
    <AttDef Type="StaticDOFZDirichlet" Label="Static DOF Z Dirichlet" BaseType="ZBoundaryCondition" Version="0" Unique="true" Nodal="true" Associations="f">
      <ItemDefinitions>
        <Double Name="Value" Label="Dirichlet for DOF Z" Version="0" NumberOfRequiredValues="1">
          <DefaultValue>0</DefaultValue>
          <Categories>
            <Cat>Mechanics</Cat>
          </Categories>
        </Double>
      </ItemDefinitions>
    </AttDef>

    <AttDef Type="DynamicDOFXDirichlet" Label="Dynamic DOF X Dirichlet" BaseType="XBoundaryCondition" Version="0" Unique="true" Nodal="true" Associations="f">
      <ItemDefinitions>
        <Double Name="Value" Label="Time Dependent Dirichlet for DOF X" Version="0" NumberOfRequiredValues="1">
          <ExpressionType>PolyLinearFunction</ExpressionType>
          <Categories>
            <Cat>Mechanics</Cat>
          </Categories>
        </Double>
      </ItemDefinitions>
    </AttDef>
    <AttDef Type="DynamicDOFYDirichlet" Label="Dynamic DOF Y Dirichlet" BaseType="YBoundaryCondition" Version="0" Unique="true" Nodal="true" Associations="f">
      <ItemDefinitions>
        <Double Name="Value" Label="Time Dependent Dirichlet for DOF Y" Version="0" NumberOfRequiredValues="1">
          <ExpressionType>PolyLinearFunction</ExpressionType>
          <Categories>
            <Cat>Mechanics</Cat>
          </Categories>
        </Double>
      </ItemDefinitions>
    </AttDef>
    <AttDef Type="DynamicDOFZDirichlet"  Label="Dynamic DOF Z Dirichlet" BaseType="ZBoundaryCondition" Version="0" Unique="true" Nodal="true" Associations="f">
      <ItemDefinitions>
        <Double Name="Value" Label="Time Dependent Dirichlet for DOF Z" Version="0" NumberOfRequiredValues="1">
          <ExpressionType>PolyLinearFunction</ExpressionType>
          <Categories>
            <Cat>Mechanics</Cat>
          </Categories>
        </Double>
      </ItemDefinitions>
    </AttDef>

    <!--***  Discretization Definitions ***-->
    <AttDef Type="Discretization" BaseType="" Version="0" Unique="true" Associations="">
      <ItemDefinitions>
        <Int Name="WorksetSize" Label="Workset Size" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
          <DefaultValue>1</DefaultValue>
          <RangeInfo>
            <Min Inclusive="false">0</Min>
          </RangeInfo>
        </Int>
        <Int Name="CubatureDegree" Label="Cubature Degree" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
          <DefaultValue>3</DefaultValue>
          <RangeInfo>
            <Min Inclusive="false">0</Min>
          </RangeInfo>
        </Int>
        <Int Name="SeparateEvaluatorsByElementBlock" Label="Separate Evaluators By Element Block" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
          <DiscreteInfo DefaultIndex="1">
            <Value Enum="false">0</Value>
            <Value Enum="true">1</Value>
          </DiscreteInfo>
        </Int>
        <String Name="ExodusInputFileName" Label="Exodus Input File Name" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
        </String>
        <String Name="ExodusOutputFileName" Label="Exodus Output File Name" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
        </String>
        <String Name="ExodusSolutionName" Label="Exodus Solution Name" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
        </String>
        <String Name="ExodusResidualName" Label="Exodus Residual Name" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1" />
        <Int Name="UseSerialMeshType" Label="Use Serial Mesh Type" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
          <DiscreteInfo DefaultIndex="1">
            <Value Enum="false">0</Value>
            <Value Enum="true">1</Value>
          </DiscreteInfo>
        </Int>
      </ItemDefinitions>
    </AttDef>

    <!--***  Expression Definitions ***-->
    <!--    <AttDef Type="SimExpression" Abstract="1" Association="None"/> -->
    <AttDef Type="SimExpression" Abstract="1"/>
    <AttDef Type="SimInterpolation" BaseType="SimExpression" Abstract="1"/>
    <AttDef Type="PolyLinearFunction" Label="Polylinear Function" BaseType="SimInterpolation" Version="0" Unique="true" Associations="">
      <ItemDefinitions>
        <Group Name="ValuePairs" Label="Value Pairs" NumberOfRequiredGroups="1">
          <ItemDefinitions>
            <Double Name="X" Version="0" AdvanceLevel="0" NumberOfRequiredValues="0"/>
            <Double Name="Value" Version="0" AdvanceLevel="0" NumberOfRequiredValues="0"/>
          </ItemDefinitions>
        </Group>
        <!-- acbauer check on name here -->
        <String Name="Sim1DLinearExp" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1" />
      </ItemDefinitions>
    </AttDef>

    <!--***  Solvers Definitions ***-->
    <!--***  Continuation Solvers Definitions ***-->
    <AttDef Type="ContinuationSolver" Label="Continuation Solver" BaseType="" Abstract="0" Version="0" Unique="true" Associations="">
      <!-- http://trilinos.sandia.gov/packages/docs/r11.4/packages/nox/doc/html/loca_parameters.html -->
      <ItemDefinitions>
        <Group Name="Stepper" Label="Stepper" NumberOfRequiredGroups="1">
          <ItemDefinitions>
            <String Name="ContinuationType" Label="Continuation Type" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
              <DiscreteInfo DefaultIndex="1">
                <Value Enum="Natural">Natural</Value>
                <Value Enum="Arc Length">Arc Length</Value>
              </DiscreteInfo>
            </String>
            <String Name="ContinuationParameter" Label="Continuation Parameter" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
            </String>
            <Double Name="InitialValue" Label="Initial Value" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1"/>
            <Double Name="MaxValue" Label="Maximum Value" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1"/>
            <Double Name="MinValue" Label="Minimum Value" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1"/>
            <Int Name="MaxSteps" Label="Max Steps" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
              <DefaultValue>100</DefaultValue>
            </Int>
            <Int Name="MaxNonlinearIterations" Label="Max Nonlinear Iterations" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
              <DefaultValue>15</DefaultValue>
            </Int>
            <Int Name="SkipParametersDerivative" Label="Skip Parameters Derivative" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
              <DiscreteInfo DefaultIndex="1">
                <Value Enum="false">0</Value>
                <Value Enum="true">1</Value>
              </DiscreteInfo>
            </Int>
            <Int Name="EnableArcLengthScaling" Label="Enable Arc Length Scaling" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
              <DiscreteInfo DefaultIndex="1">
                <Value Enum="false">0</Value>
                <Value Enum="true">1</Value>
              </DiscreteInfo>
            </Int>
            <Double Name="GoalArcLengthParameterContribution" Label="Goal Arc Length Parameter Contribution" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
              <DefaultValue>0.5</DefaultValue>
            </Double>
            <Double Name="MaxArcLengthParameterContribution" Label="Max Arc Length Parameter Contribution" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
              <DefaultValue>0.8</DefaultValue>
            </Double>
            <Double Name="InitialScaleFactor" Label="Initial Scale Factor" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
              <DefaultValue>1.0</DefaultValue>
            </Double>
            <Double Name="MinScaleFactor" Label="Min Scale Factor" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
              <DefaultValue>1.0e-3</DefaultValue>
            </Double>
            <Int Name="EnableTangentFactorStepSizeScaling" Label="Enable Tangent Factor Step Size Scaling" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
              <DiscreteInfo DefaultIndex="0">
                <Value Enum="false">0</Value>
                <Value Enum="true">1</Value>
              </DiscreteInfo>
            </Int>
            <Double Name="MinTangentFactor" Label="Min Tangent Factor" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
              <DefaultValue>0.1</DefaultValue>
            </Double>
            <Double Name="TangentFactorExponent" Label="Tangent Factor Exponent" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
              <DefaultValue>1.0</DefaultValue>
            </Double>
            <String Name="BorderedSolverMethod" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
              <DiscreteInfo DefaultIndex="0">
                <Value Enum="Bordering">Bordering</Value>
                <Value Enum="Nested">Nested</Value>
              </DiscreteInfo>
            </String>
            <Int Name="ComputeEigenvalues" Label="Compute EigenValues" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
              <DiscreteInfo DefaultIndex="0">
                <Value Enum="false">0</Value>
                <Value Enum="true">1</Value>
              </DiscreteInfo>
            </Int>
            <String Name="Eigensolver" Label="Eigensolver" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
              <DiscreteInfo DefaultIndex="0">
                <Value Enum="Default">Default</Value>
                <Value Enum="Anasazi">Anasazi</Value>
              </DiscreteInfo>
            </String>
            <String Name="AnasaziOperator" Label="Anasazi Operator" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
              <DiscreteInfo DefaultIndex="0">
                <Value Enum="Jacobian Inverse">Jacobian Inverse</Value>
                <Value Enum="Shift-Invert (ACB-needs value)">Shift-Invert (ACB-needs value)</Value>
                <Value Enum="Cayley (ACB-needs 2 values)">Cayley (ACB-needs 2 values)</Value>
              </DiscreteInfo>
            </String>
            <Int Name="AnasaziBlockSize" Label="Anasazi Block Size" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
              <DefaultValue>1</DefaultValue>
            </Int>
            <Int Name="AnasaziNumBlocks" Label="Anasazi Num Blocks" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
              <DefaultValue>30</DefaultValue>
            </Int>
            <Int Name="AnasaziNumEigenvalues" Label="Anasazi Num Eigenvalues" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
              <DefaultValue>4</DefaultValue>
            </Int>
            <Double Name="AnasaziConvergenceTolerance" Label="Anasazi Convergence Tolerance" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
              <DefaultValue>1.0e-7</DefaultValue>
            </Double>
            <Double Name="AnasaziLinearSolveTolerance" Label="Anasazi Linear Solve Tolerance (ACB-default is same value as AnasaziConvergenceTolerance" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
              <DefaultValue>1.0e-7</DefaultValue>
            </Double>
            <Int Name="AnasaziStepSize" Label="Anasazi Step Size" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
              <DefaultValue>1</DefaultValue>
            </Int>
            <Int Name="AnasaziMaximumRestarts" Label="Anasazi Maximum Restarts" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
              <DefaultValue>1</DefaultValue>
            </Int>
            <Int Name="AnasaziSymmetric" Label="Anasazi Symmetric" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
              <DiscreteInfo DefaultIndex="0">
                <Value Enum="false">0</Value>
                <Value Enum="true">1</Value>
              </DiscreteInfo>
            </Int>
            <Int Name="AnasaziNormalizeEigenvectorsWithMassMatrix" Label="Anasazi Normalize Eigenvectors With Mass Matrix" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
              <DiscreteInfo DefaultIndex="0">
                <Value Enum="false">0</Value>
                <Value Enum="true">1</Value>
              </DiscreteInfo>
            </Int>
            <String Name="AnasaziVerbosity" Label="Anasazi Verbosity" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
              <DefaultValue>Anasazi::Errors+Anasazi::Warnings+Anasazi::FinalSummary</DefaultValue>
            </String>
            <String Name="AnasaziSortingOrder" Label="Anasazi Sorting Order" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
              <DiscreteInfo DefaultIndex="0">
                <Value Enum="Largest magnitude">LM</Value>
                <Value Enum="Largest real component">LR</Value>
                <Value Enum="Largest imaginary component">LI</Value>
                <Value Enum="Smallest magnitude">SM</Value>
                <Value Enum="Smallest real component">SR</Value>
                <Value Enum="Smallest imaginary component">SI</Value>
                <Value Enum="Largest real part of inverse Cayley transformation">CA</Value>
              </DiscreteInfo>
            </String>
          </ItemDefinitions>
        </Group>
        <AttributeRef Name="BifurcationCalculation" Label="Bifurcation Calculation" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
          <AttDef>LOCABifurcationAlgorithm</AttDef>
        </AttributeRef>
        <Group Name="Predictor" Label="Predictor" NumberOfRequiredGroups="1">
          <ItemDefinitions>
            <String Name="Method" Label="Method" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
              <DiscreteInfo DefaultIndex="1">
                <Value Enum="Constant">Constant</Value>
                <Value Enum="Secant">Secant</Value>
                <Value Enum="Tangent">Tangent</Value>
                <Value Enum="Random">Random</Value>
              </DiscreteInfo>
            </String>
            <Double Name="Epsilon" Label="Epsilon" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
              <DefaultValue>1.0e-e3</DefaultValue>
            </Double>
            <!-- acbauer - not sure how to do "First Step Predictor" or "Last Step Predictor" -->
          </ItemDefinitions>
        </Group>
        <Group Name="StepSize" Label="Step Size" NumberOfRequiredGroups="1">
          <ItemDefinitions>
            <String Name="Method" Label="Method" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
              <DiscreteInfo DefaultIndex="1">
                <Value Enum="Constant">Constant</Value>
                <Value Enum="Adaptive">Adaptive</Value>
              </DiscreteInfo>
            </String>
            <Double Name="InitialStepSize" Label="Initial Step Size" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
              <DefaultValue>1.0</DefaultValue>
            </Double>
            <Double Name="MinStepSize" Label="Min Step Size" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
              <DefaultValue>1.0e-12</DefaultValue>
            </Double>
            <Double Name="MaxStepSize" Label="Max Step Size" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
              <DefaultValue>1.0e+12</DefaultValue>
            </Double>
            <Double Name="FailedStepReductionFactor" Label="FailedStepReductionFactor" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
              <DefaultValue>0.5</DefaultValue>
            </Double>
            <Double Name="SuccessfulStepIncreaseFactor" Label="Successful Step Increase Factor" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
              <DefaultValue>1.26</DefaultValue>
            </Double>
            <Double Name="Aggressiveness" Label="Aggressiveness" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
              <DefaultValue>0.5</DefaultValue>
            </Double>
          </ItemDefinitions>
        </Group>
        <!-- acbauer - skipping Constraints section -->
      </ItemDefinitions>
    </AttDef>
    <!--*** LOCA bifurcation calculation ***-->
    <AttDef Type="LOCABifurcationAlgorithm" BaseType="" Abstract="1" Version="0" Unique="true" Associations="" />
    <!-- acbauer - eventually add in turning point, pitchfork, hopf -->

    <!--***  Nonlinear Solvers Definitions ***-->
    <AttDef Type="NonlinearSolver" BaseType="" Abstract="1" Version="0" Unique="true" Associations="" />
    <AttDef Type="NOX" Label="NOX" BaseType="NonlinearSolver" Abstract="0" Version="0" Unique="true" Associations="">
      <!-- Info on NOX at http://trilinos.sandia.gov/packages/docs/r11.4/packages/nox/doc/html/parameters.html -->
      <ItemDefinitions>

        <String Name="NOXNonlinearSolver" Label="Nonlinear solver type" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
          <ChildrenDefinitions>
            <Double Name="FullStep" Label="Full step" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
              <DefaultValue>1</DefaultValue>
            </Double>
            <Double Name="MinimumStep" Label="Minimum step" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
              <DefaultValue>1e-12</DefaultValue>
            </Double>
            <Double Name="DefaultStep" Label="Default step" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
              <DefaultValue>1</DefaultValue>
            </Double>
            <Double Name="RecoveryStep" Label="Recovery step" Version="0" Optional="true" AdvanceLevel="0" NumberOfRequiredValues="1">
              <BriefDescription>Defaults to value for "Default step"</BriefDescription>
              <DefaultValue>1</DefaultValue>
            </Double>
            <Double Name="MaximumStep" Label="Maximum step" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
              <DefaultValue>1</DefaultValue>
            </Double>
            <Int Name="MaxIters" Label="Maximum iterations" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1" />
            <Double Name="ReductionFactor" Label="Reduction factor" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1" />
            <String Name="InterpolationType" Label="Interpolation type" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
              <DiscreteInfo DefaultIndex="3">
                <Value Enum="Quadratic">Quadratic</Value>
                <Value Enum="Quadratic3">Quadratic3</Value>
                <Value Enum="Cubic">Cubic</Value>
              </DiscreteInfo>
            </String>
            <Double Name="MinBoundsFactor" Label="Minimum bounds factor" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
              <DefaultValue>.1</DefaultValue>
            </Double>
            <Double Name="MaxBoundsFactor" Label="Maximum bounds factor" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
              <DefaultValue>.5</DefaultValue>
            </Double>
            <String Name="SufficientDecreaseCondition" Label="Sufficient decrease condition" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
              <DiscreteInfo DefaultIndex="0">
                <Value Enum="Armiijo-Goldstein">Armijo-Goldstein</Value>
                <Value Enum="Ared/Pred">Ared/Pred</Value>
                <Value Enum="None">None</Value>
              </DiscreteInfo>
            </String>
            <Double Name="AphaFactor" Label="Alpha factor" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
              <DefaultValue>1e-4</DefaultValue>
            </Double>
            <Int Name="ForceInterpolation" Label="Force interpolation" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
              <DiscreteInfo DefaultIndex="0">
                <Value Enum="False">0</Value>
                <Value Enum="True">1</Value>
              </DiscreteInfo>
            </Int>
            <Int Name="User counters" Label="Use counters" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
              <DiscreteInfo DefaultIndex="1">
                <Value Enum="False">0</Value>
                <Value Enum="True">1</Value>
              </DiscreteInfo>
            </Int>
            <Int Name="MaximumIterationForIncrease" Label="Maximum iteration for increase" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
              <DefaultValue>0</DefaultValue>
              <RangeInfo>
                <Min Inclusive="true">0</Min>
              </RangeInfo>
            </Int>
            <Int Name="AllowedRelativeIncrease" Label="Allowed relative increase" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
              <DefaultValue>100</DefaultValue>
            </Int>
            <String Name="MTSufficientDecreaseCondition" Label="Sufficient decrease condition" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
              <DiscreteInfo DefaultIndex="0">
                <Value Enum="Armiijo-Goldstein">Armijo-Goldstein</Value>
                <Value Enum="Ared/Pred">Ared/Pred</Value>
              </DiscreteInfo>
            </String>
            <Double Name="MTSufficientDecrease" Label="Sufficient decrease" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
              <DefaultValue>1e-4</DefaultValue>
            </Double>
            <Double Name="MTCurvatureCondition" Label="Curvature condition" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
              <DefaultValue>.9999</DefaultValue>
            </Double>
            <Int Name="MTOptimizeSlopeCalculation" Label="Optimize slope calculation" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
              <DiscreteInfo DefaultIndex="0">
                <Value Enum="False">0</Value>
                <Value Enum="True">1</Value>
              </DiscreteInfo>
            </Int>
            <!-- ignoring user defined norm and merit function -->
            <Double Name="MTIntervalWidth" Label="Interval width" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
              <DefaultValue>1e-15</DefaultValue>
            </Double>
            <Double Name="MTMaximumStep" Label="Maximum step" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
              <DefaultValue>1e6</DefaultValue>
            </Double>
            <Double Name="MTMinimumStep" Label="Minimum step" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
              <DefaultValue>1e-12</DefaultValue>
            </Double>
            <Int Name="MTMaxIters" Label="Maximum iterations" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
              <DefaultValue>20</DefaultValue>
            </Int>
            <Double Name="MTDefaultStep" Label="Default step" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
              <DefaultValue>1</DefaultValue>
            </Double>
            <AttributeRef Name="CauchyDirection" Label="Cauchy search direction method" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
              <BriefDescription>Used to specify the second direction in dogleg trust regions methods.</BriefDescription>
              <AttDef>NOXDirection</AttDef>
            </AttributeRef>
            <Double Name="MinimumTrustRegionRadius" Label="Minimum trust region radius" Version="0" Optional="true" AdvanceLevel="0" NumberOfRequiredValues="1">
              <DefaultValue>1e-6</DefaultValue>
            </Double>
            <Double Name="MaximumTrustRegionRadius" Label="Maximum trust region radius" Version="0" Optional="true" AdvanceLevel="0" NumberOfRequiredValues="1">
              <DefaultValue>1e10</DefaultValue>
            </Double>
            <Double Name="MinimumImprovementRatio" Label="Minimum improvement ratio" Version="0" Optional="true" AdvanceLevel="0" NumberOfRequiredValues="1">
              <DefaultValue>1e-4</DefaultValue>
            </Double>
            <Double Name="ContractionTriggerRatio" Label="Contraction trigger ratio" Version="0" Optional="true" AdvanceLevel="0" NumberOfRequiredValues="1">
              <DefaultValue>.1</DefaultValue>
            </Double>
            <Double Name="ContractionFactor" Label="Contraction factor" Version="0" Optional="true" AdvanceLevel="0" NumberOfRequiredValues="1">
              <DefaultValue>.25</DefaultValue>
            </Double>
            <Double Name="ExpansionTriggerRatio" Label="Expansion trigger ratio" Version="0" Optional="true" AdvanceLevel="0" NumberOfRequiredValues="1">
              <DefaultValue>.75</DefaultValue>
            </Double>
            <Double Name="ExpansionFactor" Label="Expansion factor" Version="0" Optional="true" AdvanceLevel="0" NumberOfRequiredValues="1">
              <DefaultValue>4</DefaultValue>
            </Double>
            <Int Name="UseAredPredRatioCalculation" Label="Use Ared/Pred ratio calculation" Version="0" Optional="true" AdvanceLevel="0" NumberOfRequiredValues="1">
              <DiscreteInfo DefaultIndex="0">
                <Value Enum="false">0</Value>
                <Value Enum="true">1</Value>
              </DiscreteInfo>
            </Int>
            <String Name="InnerIterationMethod" Label="Inner iteration method" Version="0" Optional="true" AdvanceLevel="0" NumberOfRequiredValues="1">
              <DiscreteInfo DefaultIndex="0">
                <Value Enum="Standard Trust Region">Standard Trust Region</Value>
                <Value Enum="Inexact Trust Region">Inexact Trust Region</Value>
              </DiscreteInfo>
            </String>
            <Int Name="UseCauchyInNewtonDirection" Label="Use Cauchy in Newton Direction" Version="0" Optional="true" AdvanceLevel="0" NumberOfRequiredValues="1">
              <DiscreteInfo DefaultIndex="0">
                <Value Enum="false">0</Value>
                <Value Enum="true">1</Value>
              </DiscreteInfo>
            </Int>
            <Int Name="UseDoglegSegmentMinimization" Label="Use dogleg segment minimization" Version="0" Optional="true" AdvanceLevel="0" NumberOfRequiredValues="1">
              <DiscreteInfo DefaultIndex="0">
                <Value Enum="false">0</Value>
                <Value Enum="true">1</Value>
              </DiscreteInfo>
            </Int>
            <Int Name="UseCounters" Label="Use counters" Version="0" Optional="true" AdvanceLevel="0" NumberOfRequiredValues="1">
              <DiscreteInfo DefaultIndex="1">
                <Value Enum="false">0</Value>
                <Value Enum="true">1</Value>
              </DiscreteInfo>
            </Int>
            <Int Name="WriteOutputParameters" Label="Write output parameters" Version="0" Optional="true" AdvanceLevel="0" NumberOfRequiredValues="1">
              <DiscreteInfo DefaultIndex="1">
                <Value Enum="false">0</Value>
                <Value Enum="true">1</Value>
              </DiscreteInfo>
            </Int>


          </ChildrenDefinitions>

          <DiscreteInfo>
            <Structure>
              <Value Enum="NOX Full Step Line Search">NOXLineaSearchFullStep</Value>
              <Items>
                <Item>FullStep</Item>
              </Items>
            </Structure>
            <Structure>
              <Value Enum="NOX Backtrack Line Search">NOXLineSearchBacktrack</Value>
              <Items>
                <Item>MinimumStep</Item>
                <Item>DefaultStep</Item>
                <Item>RecoveryStep</Item>
                <Item>MaximumStep</Item>
                <Item>MaxIters</Item>
              </Items>
            </Structure>
            <Structure>
              <Value Enum="NOX Polynomial Line Search">NOXLineSearchPolynomial</Value>
              <Items>
                <Item>DefaultStep</Item>
                <Item>MaxIters</Item>
                <Item>MinimumStep</Item>
                <Item>RecoveryStep</Item>
                <Item>InterpolationType</Item>
                <Item>MinBoundsFactor</Item>
                <Item>MaxBoundsFactor</Item>
                <Item>SufficientDecreaseCondition</Item>
                <Item>AlphaFactor</Item>
                <Item>ForceInterpolation</Item>
                <Item>User counters</Item>
                <Item>MaximumIterationForIncrease</Item>
                <Item>AllowedRelativeIncrease</Item>
              </Items>
            </Structure>
            <Structure>
              <Value Enum="NOX More Thuente Search">NOXLineSearchMoreThuente</Value>
              <Items>
                <Item>MTSufficientDecreaseCondition</Item>
                <Item>MTSufficientDecrease</Item>
                <Item>MTCurvatureDecrease</Item>
                <Item>MTOptimizeSlopeCalculation</Item>
                <Item>MTIntervalWidth</Item>
                <Item>MTMaximumStep</Item>
                <Item>MTMinimumStep</Item>
                <Item>MTMAxIters</Item>
                <Item>MTDefaultStep</Item>
                <Item>RecoveryStep</Item>
              </Items>
            </Structure>
            <Structure>
              <Value Enum="NOX Trust Region">NOXTrustRegion</Value>
              <Items>
                <Item>RecoveryStep</Item>
                <Item>CauchyDirection</Item>
                <Item>MinimumTrustRegionRadius</Item>
                <Item>MaximumTrustRegionRadius</Item>
                <Item>MinimumImprovementRatio</Item>
                <Item>ContractionTriggerRatio</Item>
                <Item>ContractionFactor</Item>
                <Item>ExpansionTriggerRatio</Item>
                <Item>ExpansionFactor</Item>
                <Item>UseAredPredRatioCalculation</Item>
              </Items>
            </Structure>
            <Structure>
              <Value Enum="NOX InexactTrust Region">NOXInexactTrustRegion</Value>
              <Items>
                <Item>CauchyDirection</Item>
                <Item>InnerIterationMethod</Item>
                <Item>MinimumTrustRegionRadius</Item>
                <Item>MaximumTrustRegionRadius</Item>
                <Item>MinimumImprovementRatio</Item>
                <Item>ContractionTriggerRatio</Item>
                <Item>ContractionFactor</Item>
                <Item>ExpansionTriggerRatio</Item>
                <Item>ExpansionFactor</Item>
                <Item>RecoveryStep</Item>
                <Item>UseAredPredRatioCalculation</Item>
                <Item>UseCaucnyInNextonDirection</Item>
                <Item>UseDoglegSegmentMinimization</Item>
                <Item>UseCounters</Item>
                <Item>WriteOutputParameters</Item>
              </Items>
            </Structure>
            <Value Enum="NOX Tensor">NOXTensor</Value>
          </DiscreteInfo>
        </String>

        <AttributeRef Name="NOXDirection" Label="Primary search direction method" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
          <AttDef>NOXDirection</AttDef>
        </AttributeRef>
        <!-- acbauer - skipping user defined pre/post operator -->
        <String Name="StatusTestCheckType" Label="Status test check type" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
          <DiscreteInfo DefaultIndex="0">
            <Value Enum="Complete">Complete</Value>
            <Value Enum="Minimal">Minimal</Value>
            <Value Enum="None">None</Value>
          </DiscreteInfo>
        </String>
        <!-- acbauer - skipping user defined merit functions -->
        <Group Name="Printing" Label="Printing" Version="0" AdvanceLevel="0" NumberOfRequiredGroups="1">
          <Int Name="Error" Label="Print error" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
            <DiscreteInfo DefaultIndex="0">
              <Value Enum="False">0</Value>
              <Value Enum="True">1</Value>
            </DiscreteInfo>
            <Categories>
              <Cat>Mechanics</Cat>
            </Categories>
          </Int>
          <Int Name="Warning" Label="Print warning" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
            <DiscreteInfo DefaultIndex="0">
              <Value Enum="False">0</Value>
              <Value Enum="True">1</Value>
            </DiscreteInfo>
          </Int>
          <Int Name="OuterIteration" Label="Print outer iteration" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
            <DiscreteInfo DefaultIndex="0">
              <Value Enum="False">0</Value>
              <Value Enum="True">1</Value>
            </DiscreteInfo>
          </Int>
          <Int Name="InnerIteration" Label="Print inner iteration" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
            <DiscreteInfo DefaultIndex="0">
              <Value Enum="False">0</Value>
              <Value Enum="True">1</Value>
            </DiscreteInfo>
          </Int>
          <Int Name="Parameters" Label="Print parameters" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
            <DiscreteInfo DefaultIndex="0">
              <Value Enum="False">0</Value>
              <Value Enum="True">1</Value>
            </DiscreteInfo>
          </Int>
          <Int Name="Details" Label="Print details" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
            <DiscreteInfo DefaultIndex="0">
              <Value Enum="False">0</Value>
              <Value Enum="True">1</Value>
            </DiscreteInfo>
          </Int>
          <Int Name="OuterIterationStatusTest" Label="Print outer iteration status test" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
            <DiscreteInfo DefaultIndex="0">
              <Value Enum="False">0</Value>
              <Value Enum="True">1</Value>
            </DiscreteInfo>
          </Int>
          <Int Name="LinearSolverDetails" Label="Print linear solver details" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
            <DiscreteInfo DefaultIndex="0">
              <Value Enum="False">0</Value>
              <Value Enum="True">1</Value>
            </DiscreteInfo>
          </Int>
          <Int Name="TestDetails" Label="Print test details" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
            <DiscreteInfo DefaultIndex="0">
              <Value Enum="False">0</Value>
              <Value Enum="True">1</Value>
            </DiscreteInfo>
          </Int>
          <Int Name="StepperIteration" Label="Print stepper iteration" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
            <DiscreteInfo DefaultIndex="0">
              <Value Enum="False">0</Value>
              <Value Enum="True">1</Value>
            </DiscreteInfo>
          </Int>
          <Int Name="StepperDetails" Label="Print stepper details" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
            <DiscreteInfo DefaultIndex="0">
              <Value Enum="False">0</Value>
              <Value Enum="True">1</Value>
            </DiscreteInfo>
          </Int>
          <Int Name="StepperParameters" Label="Print stepper parameters" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
            <DiscreteInfo DefaultIndex="0">
              <Value Enum="False">0</Value>
              <Value Enum="True">1</Value>
            </DiscreteInfo>
          </Int>
          <Int Name="Debug" Label="Print debug" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
            <DiscreteInfo DefaultIndex="0">
              <Value Enum="False">0</Value>
              <Value Enum="True">1</Value>
            </DiscreteInfo>
          </Int>
          <Int Name="OutputProcessor" Label="Output processor" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
            <DefaultValue>0</DefaultValue>
          </Int>
          <Int Name="OutputPrecision" Label="Output precision" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
            <DefaultValue>4</DefaultValue>
          </Int>
          <!-- acbauer - skipping output stream and error stream -->
        </Group>
      </ItemDefinitions>
    </AttDef>

    <!--*** NOX Directions Definitions ***-->
    <AttDef Type="NOXDirection" BaseType="" Abstract="1" Version="0" Unique="true" Associations="" />
    <AttDef Type="NOXDirectionNewton" Label="NOX Newton Direction" BaseType="NOXDirection" Abstract="0" Version="0" Unique="true" Associations="">
      <ItemDefinitions>
        <String Name="ForcingTermMethod" Label="Forcing Term Method" Version="0" AdvanceLevel="1" NumberOfRequiredValues="1">
          <DiscreteInfo DefaultIndex="0">
            <Value Enum="Constant">Constant</Value>
            <Value Enum="Type 1">Type 1</Value>
            <Value Enum="Type 2">Type 2</Value>
          </DiscreteInfo>
        </String>
        <Double Name="ForcingTermInitialTolerance" Label="Forcing Term Initial Tolerance" Version="0" AdvanceLevel="1" Optional="true" NumberOfRequiredValues="1">
          <DefaultValue>0.1</DefaultValue>
        </Double>
        <Double Name="ForcingTermMinimumTolerance" Label="Forcing Term Minimum Tolerance" Version="0" AdvanceLevel="1" Optional="true" NumberOfRequiredValues="1">
          <DefaultValue>10e-6</DefaultValue>
        </Double>
        <Double Name="ForcingTermMaximumTolerance" Label="Forcing Term Maximum Tolerance" Version="0" AdvanceLevel="1" Optional="true" NumberOfRequiredValues="1">
          <DefaultValue>.01</DefaultValue>
        </Double>
        <Double Name="ForcingTermAlpha" Label="Forcing Term Alpha (used only by Type 2)" Version="0" AdvanceLevel="1" Optional="true" NumberOfRequiredValues="1">
          <DefaultValue>1.5</DefaultValue>
        </Double>
        <Double Name="ForcingTermGamma" Label="Forcing Term Gamma (used only by Type 2)" Version="0" AdvanceLevel="1" Optional="true" NumberOfRequiredValues="1">
          <DefaultValue>.9</DefaultValue>
        </Double>
        <Double Name="LinearSolverTolerance" Label="Linear Solver Tolerance" Version="0" AdvanceLevel="1" Optional="true" NumberOfRequiredValues="1">
          <DefaultValue>10e-10</DefaultValue>
        </Double>
      </ItemDefinitions>
    </AttDef>
    <AttDef Type="NOXDirectionSteepestDescent" Label="NOX Steepest Descent Direction" BaseType="NOXDirection" Abstract="0" Version="0" Unique="true" Associations="">
      <ItemDefinitions>
        <String Name="ScalingType" Label="ScalingType" Version="0" AdvanceLevel="1" NumberOfRequiredValues="1">
          <DiscreteInfo DefaultIndex="0">
            <Value Enum="2-Norm">2-Norm</Value>
            <Value Enum="Quadratic Model Min">Quadratic Model Min</Value>
            <Value Enum="F 2-Norm">F 2-Norm</Value>
            <Value Enum="None">None</Value>
          </DiscreteInfo>
        </String>
      </ItemDefinitions>
    </AttDef>
    <AttDef Type="NOXDirectionNonlinearCG" Label="NOX Nonlinear CG Direction" BaseType="NOXDirection" Abstract="0" Version="0" Unique="true" Associations="">
      <ItemDefinitions>
        <String Name="Orthogonalize" Label="Orthogonalize" Version="0" AdvanceLevel="1" NumberOfRequiredValues="1">
          <DiscreteInfo DefaultIndex="0">
            <Value Enum="Fletcher-Reeves">Fletcher-Reeves</Value>
            <Value Enum="Polar-Ribiere">Polar-Ribiere</Value>
          </DiscreteInfo>
        </String>
        <String Name="Precondition" Label="Precondition" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
          <DiscreteInfo DefaultIndex="0">
            <Value Enum="Off">Off</Value>
            <Value Enum="On">On</Value>
          </DiscreteInfo>
        </String>
        <Int Name="RestartFrequency" Label="Restart frequency" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
          <DefaultValue>10</DefaultValue>
          <RangeInfo>
            <Min Inclusive="true">1</Min>
          </RangeInfo>
        </Int>
      </ItemDefinitions>
    </AttDef>
    <AttDef Type="Broyden" Label="NOX Broyden Direction" BaseType="NOXDirection" Abstract="0" Version="0" Unique="true" Associations="">
      <ItemDefinitions>
        <Int Name="RestartFrequency" Label="Restart frequency" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
          <DefaultValue>10</DefaultValue>
          <RangeInfo>
            <Min Inclusive="true">1</Min>
          </RangeInfo>
        </Int>
        <Double Name="MaxConvergenceRate" Label="Max Convergence Rate" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
          <DefaultValue>1</DefaultValue>
          <RangeInfo>
            <Min Inclusive="false">0</Min>
          </RangeInfo>
        </Double>
        <Int Name="Memory" Label="Memory" Optional="1" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
          <DetailedDescription>The maximum number of past updates that can be saved in memory. Defaults to the value of "Restart Frequency".</DetailedDescription>
          <BriefDescription>The maximum number of past updates that can be saved in memory.</BriefDescription>
          <DefaultValue>10</DefaultValue>
          <RangeInfo>
            <Min Inclusive="true">1</Min>
          </RangeInfo>
        </Int>
        <Double Name="LinearSolverTolerance" Label="Linear solver tolerance" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
          <DefaultValue>1e-4</DefaultValue>
          <RangeInfo>
            <Min Inclusive="false">0</Min>
          </RangeInfo>
        </Double>
      </ItemDefinitions>
    </AttDef>
    <AttDef Type="Tensor" Label="NOX Tensor Direction" BaseType="NOXDirection" Abstract="0" Version="0" Unique="true" Associations="">
      <BriefDescription>Prerelease only</BriefDescription>
    </AttDef>
    <AttDef Type="ModifiedNewton" Label="NOX Modified Newton Direction" BaseType="NOXDirection" Abstract="0" Version="0" Unique="true" Associations="">
      <BriefDescription>Prerelease only</BriefDescription>
    </AttDef>
    <AttDef Type="QuasiNewton" Label="NOX QuasiNewton Direction" BaseType="NOXDirection" Abstract="0" Version="0" Unique="true" Associations="">
      <BriefDescription>Prerelease only</BriefDescription>
    </AttDef>

    <!--***  Linear Solvers Definitions ***-->
    <!-- http://trilinos.sandia.gov/packages/docs/dev/packages/stratimikos/doc/html/classStratimikos_1_1DefaultLinearSolverBuilder.html -->
    <!-- types of solvers are Amesos, AztecOO and Belos -->
    <AttDef Type="LinearSolver" BaseType="" Abstract="1" Version="0" Unique="true" Associations="" >
      <ItemDefinitions>
        <Int Name="EnableDelayedSolverConstruction" Label="Enable delayed solver construction" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
          <DiscreteInfo DefaultIndex="0">
            <Value Enum="false">0</Value>
            <Value Enum="true">1</Value>
          </DiscreteInfo>
        </Int>
      </ItemDefinitions>
    </AttDef>

    <AttDef Type="AmesosLinearSolver" Label="Amesos Linear Solver" BaseType="LinearSolver" Abstract="0" Version="0" Unique="true" Associations="">
      <ItemDefinitions>
        <String Name="RefactorizationPolicy" Label="Refactorization policy" Version="0" AdvanceLevel="1" NumberOfRequiredValues="1">
          <DefaultValue>RepivotOnRefactorization</DefaultValue>
        </String>
      </ItemDefinitions>
      <ItemDefinitions>
        <String Name="SolverType" Label="Solver type" Version="0" AdvanceLevel="1" NumberOfRequiredValues="1">
          <DefaultValue>Klu</DefaultValue>
        </String>
        <Int Name="ThrowOnPreconditionerInput" Label="Throw on preconditioner input" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
          <DiscreteInfo DefaultIndex="1">
            <Value Enum="false">0</Value>
            <Value Enum="true">1</Value>
          </DiscreteInfo>
        </Int>

      </ItemDefinitions>
    </AttDef>

    <AttDef Type="AztecOOSolver" Label="AxtecOO Solver" BaseType="LinearSolver" Abstract="0" Version="0" Unique="true" Associations="">
      <ItemDefinitions>
        <Int Name="OutputEveryRHS" Label="Output every RHS" Version="0" AdvanceLevel="1" NumberOfRequiredValues="1">
          <DiscreteInfo DefaultIndex="0">
            <Value Enum="false">0</Value>
            <Value Enum="true">1</Value>
          </DiscreteInfo>
        </Int>
        <Void Name="AdjointSolve" Label="Adjoint Solve" Version="0" AdvanceLevel="0" Optional="true" NumberOfRequiredValues="0"/>
        <Int Name="MaximumIterations" Label="Maximum Iterations" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
          <DefaultValue>400</DefaultValue>
          <RangeInfo>
            <Min Inclusive="true">1</Min>
          </RangeInfo>
        </Int>
        <Double Name="Tolerance" Label="Tolerance" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
          <DefaultValue>1.e-6</DefaultValue>
          <RangeInfo>
            <Min Inclusive="false">0</Min>
          </RangeInfo>
        </Double>
        <String Name="SolverAlgorithm" Label="Solver Algorithm" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
          <DiscreteInfo DefaultIndex="1">
            <Value Enum="CG">CG</Value>
            <Value Enum="GMRES">GMRES</Value>
            <Value Enum="CGS">CGS</Value>
            <Value Enum="TFQMR">TFQMR</Value>
            <Value Enum="BiCGStab">BiCGStab</Value>
            <Value Enum="LU">LU</Value>
          </DiscreteInfo>
        </String>
        <String Name="ConvergenceTest" Label="Convergence Test" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
          <DiscreteInfo DefaultIndex="0">
            <Value Enum="r0">r0</Value>
            <Value Enum="rhs">rhs</Value>
            <Value Enum="Anorm">Anorm</Value>
            <Value Enum="no scaling">no scaling</Value>
            <Value Enum="sol">sol</Value>
          </DiscreteInfo>
        </String>
        <Int Name="OutputFrequency" Label="Output Frequency" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
          <DefaultValue>0</DefaultValue>
          <RangeInfo>
            <Min Inclusive="true">0</Min>
          </RangeInfo>
        </Int>
        <String Name="VerbosityLevel" Label="Verbosity Level" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
          <DiscreteInfo DefaultIndex="0">
            <Value Enum="default">default</Value>
            <Value Enum="none">none</Value>
            <Value Enum="low">low</Value>
            <Value Enum="medium">medium</Value>
            <Value Enum="high">high</Value>
            <Value Enum="extreme">extreme</Value>
          </DiscreteInfo>
        </String>
      </ItemDefinitions>
    </AttDef>

    <AttDef Type="BelosLinearSolver" Label="Belos Linear Solver" BaseType="LinearSolver" Abstract="0" Version="0" Unique="true" Associations="">
      <ItemDefinitions>
        <AttributeRef Name="BelosAlgorithm" Label="Solver Algorithm" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1" Optional="true">
          <AttDef>BelosAlgorithm</AttDef>
        </AttributeRef>
      </ItemDefinitions>
    </AttDef>



    <AttDef Type="BelosAlgorithm" BaseType="" Abstract="1" Version="0" Unique="true" Associations="">
      <ItemDefinitions>
        <Int Name="AdaptiveBlockSize" Label="Use Adaptive Block Size" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
          <DiscreteInfo DefaultIndex="1">
            <Value Enum="false">0</Value>
            <Value Enum="true">1</Value>
          </DiscreteInfo>
        </Int>
        <Int Name="BlockSize" Label="Block Size" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
          <DiscreteInfo DefaultIndex="1">
            <Value Enum="false">0</Value>
            <Value Enum="true">1</Value>
          </DiscreteInfo>
        </Int>
        <Double Name="ConvergenceTolerance" Label="Convergence Tolerance" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
          <DefaultValue>1e-8</DefaultValue>
          <RangeInfo>
            <Min Inclusive="false">0</Min>
          </RangeInfo>
        </Double>
        <Int Name="MaximumIterations" Label="Maximum Iterations" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
          <DefaultValue>1000</DefaultValue>
          <RangeInfo>
            <Min Inclusive="false">0</Min>
          </RangeInfo>
        </Int>
        <String Name="Orthogonalization" Label="Orthogonalization Method" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
          <DiscreteInfo DefaultIndex="0">
            <Value Enum="DGKS">DGKS</Value>
            <Value Enum="ICGS">ICGS</Value>
            <Value Enum="IMGS">IMGS</Value>
          </DiscreteInfo>
        </String>
        <!-- acbauer - ignoring:
             Orthogonalization Constant : double = -1
             # The constant used by DGKS orthogonalization to determine
             # whether another step of classical Gram-Schmidt is necessary.
        -->
        <Int Name="OutputFrequency" Label="Output Frequency" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
          <DefaultValue>-1</DefaultValue>
          <RangeInfo>
            <Min Inclusive="false">-2</Min>
          </RangeInfo>
        </Int>
        <Int Name="OutputStyle" Label="Output Style" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
          <DiscreteInfo DefaultIndex="0">
            <Value Enum="General">General</Value>
            <Value Enum="Brief">Brief</Value>
          </DiscreteInfo>
        </Int>
        <!-- ignoring:
             Output Stream : Teuchos::RCP<std::ostream> = Teuchos::RCP<std::ostream>{ptr=0x53b820,node=0x544990,count=4}
             # A reference-counted pointer to the output stream where all
             # solver output is sent.
        -->
        <Void Name="ShowMaximumResidualNormOnly" Label="Show Maximum Residual Only" Version="0" AdvanceLevel="0" Optional="true">
        </Void>
        <String Name="TimerLabel" Label="String prefix for timer labels" Version="0" AdvanceLevel="0" Optional="true" NumberOfRequiredValues="1">
          <DefaultValue>Belos</DefaultValue>
        </String>
        <Int Name="Verbosity" Label="Verbosity" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
          <DefaultValue>0</DefaultValue>
          <RangeInfo>
            <Min Inclusive="true">0</Min>
          </RangeInfo>
        </Int>
      </ItemDefinitions>
    </AttDef>

    <AttDef Type="BelosBlockCGAlgorithm" Label="Belos Block CG Algorithm" BaseType="BelosAlgorithm" Abstract="0" Version="0" Unique="true" Associations=""/>
    <AttDef Type="BelosBlockGMRESAlgorithm" Label="Belos Block GMRES Algorithm" BaseType="BelosAlgorithm" Abstract="0" Version="0" Unique="true" Associations="">
      <ItemDefinitions>
        <String Name="ExplicitResidualScaling" Label="Explicit Residual Scaling" Version="0" AdvanceLevel="1" NumberOfRequiredValues="1">
          <DefaultValue>Norm of Initial Residual</DefaultValue>
        </String>
        <String Name="ImplicitResidualScaling" Label="Implicit Residual Scaling" Version="0" AdvanceLevel="1" NumberOfRequiredValues="1">
          <DefaultValue>Norm of Preconditioned Initial Residual</DefaultValue>
        </String>
        <Void Name="ExplicitResidualTest" Label="Use Explicit Residual Test" Version="0" AdvanceLevel="0" Optional="true"/>
        <Void Name="Flexible GMRES" Label="Use Flexible GMRES" Version="0" AdvanceLevel="0" Optional="true"/>
        <Int Name="MaximumRestarts" Label="Maximum Restarts" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
          <DefaultValue>20</DefaultValue>
          <RangeInfo>
            <Min Inclusive="true">0</Min>
          </RangeInfo>
        </Int>
        <Int Name="NumBlocks" Label="Maximum Number of Blocks" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
          <DefaultValue>300</DefaultValue>
        </Int>
      </ItemDefinitions>
    </AttDef>
    <AttDef Type="BelosPseudoBlockGMRESAlgorithm" Label="Belos Pseudo-Block GMRES" BaseType="BelosBlockGMRESAlgorithm" Abstract="0" Version="0" Unique="true" Associations="">
      <ItemDefinitions>
        <Int Name="DeflationQuorum" Label="Deflation Quorum" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
          <DefaultValue>1</DefaultValue>
          <RangeInfo>
            <Min Inclusive="true">1</Min>
            <!-- maximum should be less than or equal to the block size - acbauer -->
          </RangeInfo>
        </Int>
      </ItemDefinitions>
    </AttDef>
    <AttDef Type="BelosBlockGCRODRAlgorithm" Label="Belos Block GCRODR" BaseType="BelosAlgorithm" Abstract="0" Version="0" Unique="true" Associations=""/>
    <AttDef Type="BelosPseudoBlockCGAlgorithm" Label="Belos Pseudo-Block CG" BaseType="BelosAlgorithm" Abstract="0" Version="0" Unique="true" Associations="">
      <ItemDefinitions>
        <Int Name="DeflationQuorum" Label="Deflation Quorum" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
          <DefaultValue>1</DefaultValue>
          <RangeInfo>
            <Min Inclusive="true">1</Min>
            <!-- maximum should be less than or equal to the block size - acbauer -->
          </RangeInfo>
        </Int>
      </ItemDefinitions>
    </AttDef>





    <!--

Amesos ->
Amesos Settings ->
AddToDiag : double = 0
AddZeroToDiag : bool = 0
ComputeTrueResidual : bool = 0
ComputeVectorNorms : bool = 0
DebugLevel : int = 0
MatrixProperty : string = general
MaxProcs : int = -1
NoDestroy : bool = 0
OutputLevel : int = 1
PrintTiming : bool = 0
RcondThreshold : double = 1e-12
Redistribute : bool = 0
Refactorize : bool = 0
Reindex : bool = 0
ScaleMethod : int = 0
TrustMe : bool = 0
Lapack ->
Equilibrate : bool = 1
Mumps ->
ColScaling : double* = 0
Equilibrate : bool = 1
RowScaling : double* = 0
Pardiso ->
IPARM(1) : int = 0
IPARM(10) : int = 0
IPARM(11) : int = 0
IPARM(18) : int = 0
IPARM(19) : int = 0
IPARM(2) : int = 0
IPARM(21) : int = 0
IPARM(3) : int = 0
IPARM(4) : int = 0
IPARM(8) : int = 0
MSGLVL : int = 0
Scalapack ->
2D distribution : bool = 1
grid_nb : int = 32
Superludist ->
ColPerm : string = NOT SET
Equil : bool = 0
Fact : string = SamePattern
IterRefine : string = NOT SET
PrintNonzeros : bool = 0
ReplaceTinyPivot : bool = 1
ReuseSymbolic : bool = 0
RowPerm : string = NOT SET
perm_c : int* = 0
perm_r : int* = 0
VerboseObject ->
Output File : string = none
Verbosity Level : string = default
AztecOO ->
Output Every RHS : bool = 0
Adjoint Solve ->
Max Iterations : int = 400
Tolerance : double = 1e-06
AztecOO Settings ->
Aztec Preconditioner : string = ilu
Aztec Solver : string = GMRES
Convergence Test : string = r0
Drop Tolerance : double = 0
Fill Factor : double = 1
Graph Fill : int = 0
Ill-Conditioning Threshold : double = 1e+11
Orthogonalization : string = Classical
Output Frequency : int = 0
Overlap : int = 0
Polynomial Order : int = 3
RCM Reordering : string = Disabled
Size of Krylov Subspace : int = 300
Steps : int = 3
Forward Solve ->
Max Iterations : int = 400
Tolerance : double = 1e-06
AztecOO Settings ->
Aztec Preconditioner : string = ilu
Aztec Solver : string = GMRES
Convergence Test : string = r0
Drop Tolerance : double = 0
Fill Factor : double = 1
Graph Fill : int = 0
Ill-Conditioning Threshold : double = 1e+11
Orthogonalization : string = Classical
Output Frequency : int = 0
Overlap : int = 0
Polynomial Order : int = 3
RCM Reordering : string = Disabled
Size of Krylov Subspace : int = 300
Steps : int = 3
VerboseObject ->
Output File : string = none
Verbosity Level : string = default
Belos ->
Solver Type : string = Block GMRES
Solver Types ->
Block CG ->
Adaptive Block Size : bool = 1
Block Size : int = 1
Convergence Tolerance : double = 1e-08
Maximum Iterations : int = 1000
Orthogonalization : string = DGKS
Orthogonalization Constant : double = -1
Output Frequency : int = -1
Output Style : int = 0
Output Stream : Teuchos::RCP<std::ostream> = Teuchos::RCP<std::ostream>{ptr=0x53b820,node=0x544990,count=4}
Show Maximum Residual Norm Only : bool = 0
Timer Label : string = Belos
Verbosity : int = 0
Block GMRES ->
Adaptive Block Size : bool = 1
Block Size : int = 1
Convergence Tolerance : double = 1e-08
Explicit Residual Scaling : string = Norm of Initial Residual
Explicit Residual Test : bool = 0
Flexible Gmres : bool = 0
Implicit Residual Scaling : string = Norm of Preconditioned Initial Residual
Maximum Iterations : int = 1000
Maximum Restarts : int = 20
Num Blocks : int = 300
Orthogonalization : string = DGKS
Orthogonalization Constant : double = -1
Output Frequency : int = -1
Output Style : int = 0
Output Stream : Teuchos::RCP<std::ostream> = Teuchos::RCP<std::ostream>{ptr=0x53b820,node=0x544730,count=4}
Show Maximum Residual Norm Only : bool = 0
Timer Label : string = Belos
Verbosity : int = 0
Pseudo Block GMRES ->
Adaptive Block Size : bool = 1
Block Size : int = 1
Convergence Tolerance : double = 1e-08
Deflation Quorum : int = 1
Explicit Residual Scaling : string = Norm of Initial Residual
Implicit Residual Scaling : string = Norm of Preconditioned Initial Residual
Maximum Iterations : int = 1000
Maximum Restarts : int = 20
Num Blocks : int = 300
Orthogonalization : string = DGKS
Orthogonalization Constant : double = -1
Output Frequency : int = -1
Output Style : int = 0
Output Stream : Teuchos::RCP<std::ostream> = Teuchos::RCP<std::ostream>{ptr=0x53b820,node=0x544860,count=4}
Show Maximum Residual Norm Only : bool = 0
Timer Label : string = Belos
Verbosity : int = 0
VerboseObject ->
Output File : string = none
Verbosity Level : string = default
Preconditioner Types ->
Ifpack ->
Overlap : int = 0
Prec Type : string = ILU
Ifpack Settings ->
amesos: solver type : string = Amesos_Klu
fact: absolute threshold : double = 0
fact: drop tolerance : double = 0
fact: ict level-of-fill : double = 1
fact: ilut level-of-fill : double = 1
fact: level-of-fill : int = 0
fact: relative threshold : double = 1
fact: relax value : double = 0
fact: sparskit: alph : double = 0
fact: sparskit: droptol : double = 0
fact: sparskit: lfil : int = 0
fact: sparskit: mbloc : int = -1
fact: sparskit: permtol : double = 0.1
fact: sparskit: tol : double = 0
fact: sparskit: type : string = ILUT
partitioner: local parts : int = 1
partitioner: overlap : int = 0
partitioner: print level : int = 0
partitioner: type : string = greedy
partitioner: use symmetric graph : bool = 1
relaxation: damping factor : double = 1
relaxation: min diagonal value : double = 1
relaxation: sweeps : int = 1
relaxation: type : string = Jacobi
relaxation: zero starting solution : bool = 1
schwarz: combine mode : string = Zero
schwarz: compute condest : bool = 1
schwarz: filter singletons : bool = 0
schwarz: reordering type : string = none
VerboseObject ->
Output File : string = none
Verbosity Level : string = default
ML ->
Base Method Defaults : string = DD
ML Settings ->
aggregation: damping factor : double = 1.333
aggregation: edge prolongator drop threshold : double = 0
aggregation: local aggregates : int = 1
aggregation: next-level aggregates per process : int = 128
aggregation: nodes per aggregate : int = 512
aggregation: type : string = Uncoupled-MIS
coarse: max size : int = 128
coarse: pre or post : string = post
coarse: sweeps : int = 1
coarse: type : string = Amesos-KLU
default values : string = maxwell
eigen-analysis: iterations : int = 10
eigen-analysis: type : string = cg
increasing or decreasing : string = decreasing
max levels : int = 10
prec type : string = MGV
smoother: Aztec as solver : bool = 0
smoother: Aztec options : Teuchos::RCP<__gnu_debug_def::vector<int, std::allocator<int> > > = Teuchos::RCP<__gnu_debug_def::vector<int, std::allocator<int> > >{ptr=0x55de40,node=0x55df40,count=2}
smoother: Aztec params : Teuchos::RCP<__gnu_debug_def::vector<double, std::allocator<double> > > = Teuchos::RCP<__gnu_debug_def::vector<double, std::allocator<double> > >{ptr=0x55e140,node=0x55e920,count=2}
smoother: Hiptmair efficient symmetric : bool = 1
smoother: damping factor : double = 1
smoother: pre or post : string = both
smoother: sweeps : int = 1
smoother: type : string = Hiptmair
subsmoother: Chebyshev alpha : double = 20
subsmoother: edge sweeps : int = 4
subsmoother: node sweeps : int = 4
subsmoother: type : string = Chebyshev

    -->


    <!--***  Preconditioner Definitions ***-->
    <AttDef Type="Preconditioner" BaseType="" Abstract="1" Version="0" Unique="true" />
    <AttDef Type="Ifpack" Label="Ifpack" BaseType="Preconditioner" Abstract="0" Version="0" Unique="true" >
      <ItemDefinitions>
        <String Name="PreconditionerType" Label="Preconditioner type" Version="0" NumberOfRequiredValues="1">
          <DefaultValue>"ILU"</DefaultValue>
        </String>
        <Int Name="Overlap" Label="Partitioner overlap" Version="0" NumberOfRequiredValues="1">
          <DefaultValue>0</DefaultValue>
          <RangeInfo>
            <Min Inclusive="true">0</Min>
          </RangeInfo>
        </Int>
        <Double Name="DropTolerance" Label="Drop tolerance" Version="0" NumberOfRequiredValues="1">
          <DefaultValue>1e-9</DefaultValue>
          <RangeInfo>
            <Min Inclusive="false">0</Min>
          </RangeInfo>
        </Double>
        <Double Name="ILUTLevelOfFill" Label="ILUT Level of fill" Version="0" NumberOfRequiredValues="1">
          <DefaultValue>1</DefaultValue>
          <RangeInfo>
            <Min Inclusive="false">0</Min>
          </RangeInfo>
        </Double>
        <Int Name="LevelOfFill" Label="Level of fill" Version="0" NumberOfRequiredValues="1">
          <DefaultValue>1</DefaultValue>
          <RangeInfo>
            <Min Inclusive="true">1</Min>
          </RangeInfo>
        </Int>
      </ItemDefinitions>
    </AttDef>

  </Definitions>



  <!--**********  Attribute Instances ***********-->
  <Attributes>
  </Attributes>

  <!--********** Workflow Views ***********-->
  <RootView Title="SimBuilder">
    <DefaultColor>1., 1., 0.5, 1.</DefaultColor>
    <InvalidColor>1, 0.5, 0.5, 1</InvalidColor>

    <InstancedView Title="Problem Definition">
      <InstancedAttributes>
        <Att Type="ProblemDefinition">ProblemDefinition</Att>
        <Att Type="Discretization">Discretization</Att>
        <Att Type="AdaptationDefinition">AdaptationDefinition</Att>
      </InstancedAttributes>
    </InstancedView>

    <AttributeView Title="Materials" ModelEntityFilter="r" CreateEntities="true">
      <AttributeTypes>
        <Type>Material</Type>
      </AttributeTypes>
    </AttributeView>

    <AttributeView Title="Solvers">
      <AttributeTypes>
        <Type>NonlinearSolver</Type>
        <Type>LinearSolver</Type>
      </AttributeTypes>
    </AttributeView>

    <AttributeView Title="Nonlinear Search Direction">
      <AttributeTypes>
        <Type>NOXDirection</Type>
      </AttributeTypes>
    </AttributeView>

    <AttributeView Title="Belos Algorithm">
      <AttributeTypes>
        <Type>BelosAlgorithm</Type>
      </AttributeTypes>
    </AttributeView>

    <AttributeView Title="Boundary Conditions" ModelEntityFilter="f">
      <AttributeTypes>
        <Type>BoundaryCondition</Type>
      </AttributeTypes>
    </AttributeView>

    <SimpleExpressionView Title="Functions">
      <Definition>PolyLinearFunction</Definition>
    </SimpleExpressionView>

  </RootView>
</SMTK_AttributeManager>
