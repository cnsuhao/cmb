<?xml version="1.0"?>
<!--Created by XmlV1StringWriter-->
<SMTK_AttributeManager Version="1">
  <!--**********  Category and Analysis Infomation ***********-->
  <Categories>
    <Cat>Time</Cat>
    <Cat>Flow</Cat>
    <Cat>General</Cat>
    <Cat>Heat</Cat>
    <Cat>Constituent</Cat>
  </Categories>
  <Analyses>
    <Analysis Type="CFD Flow">
      <Cat>Flow</Cat>
      <Cat>General</Cat>
      <Cat>Time</Cat>
    </Analysis>
    <Analysis Type="CFD Flow with Heat Transfer">
      <Cat>Flow</Cat>
      <Cat>General</Cat>
      <Cat>Heat</Cat>
      <Cat>Time</Cat>
    </Analysis>
    <Analysis Type="Constituent Transport">
      <Cat>Constituent</Cat>
      <Cat>General</Cat>
      <Cat>Time</Cat>
    </Analysis>
  </Analyses>
  <!--**********  Attribute Definitions ***********-->
  <Definitions>
    <!--***  Constituent Definitions ***-->
    <AttDef Type="Constituent" Label="Constituent" BaseType="" Abstract="1" Version="0" />
    <AttDef Type="GeneralConstituent" BaseType="Constituent" Label="General Constituent" Version="0">
      <ItemDefinitions>
        <Group Name="GenConstituentParams" Label="General transport parameters"
               Version="0" AdvanceLevel="0" NumberOfRequiredGroups="1">
          <ItemDefinitions>
            <Double Name="ReferenceConcentration" Label="Characteristic Concentration"
                    Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
              <RangeInfo>
                <Min Inclusive="false">0.0</Min>
              </RangeInfo>
              <DefaultValue>1.0</DefaultValue>
              <Categories>
                <Cat>Constituent</Cat>
              </Categories>
            </Double>
          </ItemDefinitions>
        </Group>
      </ItemDefinitions>
    </AttDef>
    <AttDef Type="VorticityConstituent" BaseType="Constituent" Label="Vorticity Constituent" Version="0">
      <ItemDefinitions>
        <Group Name="VortConstituentParams" Label="Vorticity constituent transport parameters"
               Version="0" AdvanceLevel="0" NumberOfRequiredGroups="1">
          <ItemDefinitions>
            <Double Name="NormalizationFactor" Label="Normalization Factor" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
              <RangeInfo>
                <Min Inclusive="false">0.0</Min>
              </RangeInfo>
              <DefaultValue>1.0</DefaultValue>
              <Categories>
                <Cat>Constituent</Cat>
              </Categories>
            </Double>
            <Double Name="AsTerm" Label="As Term" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
              <RangeInfo>
                <Min Inclusive="true">0.0</Min>
              </RangeInfo>
              <DefaultValue>0.0</DefaultValue>
            </Double>
            <Double Name="DsTerm" Label="Ds Term" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
              <RangeInfo>
                <Min Inclusive="true">0.0</Min>
              </RangeInfo>
              <DefaultValue>0.0</DefaultValue>
            </Double>
          </ItemDefinitions>
        </Group>
      </ItemDefinitions>
    </AttDef>
    <AttDef Type="SalinityConstituent" BaseType="Constituent" Label="Salinity Constituent" Version="0">
      <ItemDefinitions>
        <Group Name="SalConstituentParams" Label="Salinity transport parameters:"
               Version="0" AdvanceLevel="0" NumberOfRequiredGroups="1">
          <ItemDefinitions>
            <Double Name="ReferenceConcentration" Label="Reference Concentration"
                    Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
              <RangeInfo>
                <Min Inclusive="false">0.0</Min>
              </RangeInfo>
              <DefaultValue>1.0</DefaultValue>
              <Categories>
                <Cat>Constituent</Cat>
              </Categories>
            </Double>
          </ItemDefinitions>
        </Group>
      </ItemDefinitions>
    </AttDef>
    <AttDef Type="TemperatureConstituent" BaseType="Constituent" Label="Temperature Constituent" Version="0">
      <ItemDefinitions>
        <Group Name="TempConstituentParams" Label="Temperature transport parameters:"
               Version="0" AdvanceLevel="0" NumberOfRequiredGroups="1">
          <ItemDefinitions>
            <Double Name="ReferenceConcentration" Label="Reference Concentration"
                    Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
              <RangeInfo>
                <Min Inclusive="false">0.0</Min>
              </RangeInfo>
              <DefaultValue>1.0</DefaultValue>
              <Categories>
                <Cat>Constituent</Cat>
              </Categories>
            </Double>
          </ItemDefinitions>
        </Group>
      </ItemDefinitions>
    </AttDef>

    <!--***  Material Definitions ***-->
    <AttDef Type="SolidMaterial" Label="Solid Material" BaseType="" Version="0" Unique="true" Associations="f">
      <ItemDefinitions>

        <Group Name="TurbulentDiffusionRate" Label="Turbulent diffusion rate" Version="0" AdvanceLevel="0">
          <!-- Uses "standard pattern" for export type "idconval" -->
          <ItemDefinitions>
            <Double Name="DefaultValue" Label="Default turbulent diffusion rate" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
              <BriefDescription>The default value applied to any constituents not assigned individually.</BriefDescription>
              <Categories>
                <Cat>Constituent</Cat>
              </Categories>
              <DefaultValue>1.0E-06</DefaultValue>
              <RangeInfo>
                <Min Inclusive="false">0</Min>
              </RangeInfo>
            </Double>
            <Group Name="IndividualValue" Label="Individual turbulent diffusion rates" Version="0" AdvanceLevel="0"
                   Extensible="true" NumberOfRequiredGroups="0">
              <BriefDescription>The value for one individual constituent.</BriefDescription>
              <ItemDefinitions>
                <AttributeRef Name="Constituent" Label="Constituent" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
                  <AttDef>Constituent</AttDef>
                </AttributeRef>
                <Double Name="Value" Label="Turbulent diffusion rate" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
                  <Categories>
                    <Cat>Constituent</Cat>
                  </Categories>
                  <DefaultValue>1.0E-06</DefaultValue>
                  <RangeInfo>
                    <Min Inclusive="false">0</Min>
                  </RangeInfo>
                </Double>
              </ItemDefinitions>
            </Group>
          </ItemDefinitions>
        </Group>

        <Group Name="KinematicEddyViscosity" Label="Kinematic eddy viscosity:" Version="0" AdvanceLevel="0" NumberOfRequiredGroups="1">
          <ItemDefinitions>
            <Double Name="Value1" Label="Exx" Version="0" NumberOfRequiredValues="1">
              <RangeInfo>
                <Min Inclusive="false">0</Min>
              </RangeInfo>
              <Categories>
                <Cat>General</Cat>
              </Categories>
              <DefaultValue>0.5</DefaultValue>
            </Double>
            <Double Name="Value2" Label="Eyy" Version="0" NumberOfRequiredValues="1">
              <RangeInfo>
                <Min Inclusive="false">0</Min>
              </RangeInfo>
              <Categories>
                <Cat>General</Cat>
              </Categories>
              <DefaultValue>0.5</DefaultValue>
            </Double>
            <Double Name="Value3" Label="Exy" Version="0" NumberOfRequiredValues="1">
              <RangeInfo>
                <Min Inclusive="false">0</Min>
              </RangeInfo>
              <Categories>
                <Cat>General</Cat>
              </Categories>
              <DefaultValue>0.5</DefaultValue>
            </Double>
          </ItemDefinitions>
        </Group>

        <Double Name="CoriolisLatitude" Label="Coriolis Latitude:" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
          <RangeInfo>
            <Min Inclusive="true">-90</Min>
            <Max Inclusive="true">90</Max>
          </RangeInfo>
          <DefaultValue>0.0</DefaultValue>
          <Categories>
            <Cat>General</Cat>
          </Categories>
        </Double>
        <Int Name="MaxRefineLevels" Label="Maximum Levels of Refinement:" Version="0" AdvanceLevel="1" NumberOfRequiredValues="1">
          <RangeInfo>
            <Min Inclusive="true">0</Min>
          </RangeInfo>
          <DefaultValue>0</DefaultValue>
          <Categories>
            <Cat>General</Cat>
          </Categories>
        </Int>

        <Double Name="HydroRefineTol" Label="Hydro refinement tolerance:" Version="0" AdvanceLevel="1" NumberOfRequiredValues="1">
          <RangeInfo>
            <Min Inclusive="false">0</Min>
          </RangeInfo>
          <DefaultValue>1.0</DefaultValue>
          <Categories>
            <Cat>Flow</Cat>
          </Categories>
        </Double>

        <Group Name="TransportRefineTol" Label="Transport refinement tolerance:" Version="0" AdvanceLevel="1"
               Optional="true" IsEnabledByDefault="false" >
          <!-- Uses "standard pattern" for export type "idconval" -->
          <ItemDefinitions>
            <Double Name="DefaultValue" Label="Default transport refinement tolerance" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
              <BriefDescription>The default value applied to any constituents not assigned individually.</BriefDescription>
              <Categories>
                <Cat>Constituent</Cat>
              </Categories>
              <DefaultValue>1.0</DefaultValue>
              <RangeInfo>
                <Min Inclusive="false">0</Min>
              </RangeInfo>
            </Double>
            <Group Name="IndividualValue" Label="Individual transport refinement tolerances" Version="0" AdvanceLevel="0"
                   Extensible="true" NumberOfRequiredGroups="0">
              <BriefDescription>The value for one individual constituent.</BriefDescription>
              <ItemDefinitions>
                <AttributeRef Name="Constituent" Label="Constituent" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
                  <AttDef>Constituent</AttDef>
                </AttributeRef>
                <Double Name="Value" Label="Transport refinement tolerance" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
                  <Categories>
                    <Cat>Constituent</Cat>
                  </Categories>
                  <DefaultValue>1.0</DefaultValue>
                  <RangeInfo>
                    <Min Inclusive="false">0</Min>
                  </RangeInfo>
                </Double>
              </ItemDefinitions>
            </Group>
          </ItemDefinitions>
        </Group>

      </ItemDefinitions>
    </AttDef>

    <!--***  Friction Definitions ***-->
    <AttDef Type="Friction" Label="Friction" BaseType="" Abstract="true" Version="0" Unique="true" Associations="f"/>
    <AttDef Type="FrictionType1" BaseType="Friction" Label="Type 1 (MNG) Friction:"
    Version="0" Unique="true" Associations="f">
      <ItemDefinitions>
        <Double Name="Value" Label="MNG" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
          <Categories>
            <Cat>General</Cat>
          </Categories>
        </Double>
      </ItemDefinitions>
    </AttDef>
    <AttDef Type="FrictionType2" BaseType="Friction" Label="Type 2 (ERH) Friction:"
    Version="0" Unique="true" Associations="f">
      <ItemDefinitions>
        <Double Name="Value" Label="ERH" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
          <Categories>
            <Cat>General</Cat>
          </Categories>
        </Double>
      </ItemDefinitions>
    </AttDef>
    <AttDef Type="FrictionType3" BaseType="Friction" Label="Type 3 (SAV) Friction:"
    Version="0" Unique="true" Associations="f">
      <ItemDefinitions>
        <Double Name="Value" Label="SAV" Version="0" AdvanceLevel="0" NumberOfRequiredValues="2">
          <Categories>
            <Cat>General</Cat>
          </Categories>
        </Double>
      </ItemDefinitions>
    </AttDef>
    <AttDef Type="FrictionType4" BaseType="Friction" Label="Type 4 (URV) Friction:"
    Version="0" Unique="true" Associations="f">
      <ItemDefinitions>
        <Double Name="Value" Label="URV" Version="0" AdvanceLevel="0" NumberOfRequiredValues="3">
          <Categories>
            <Cat>General</Cat>
          </Categories>
        </Double>
      </ItemDefinitions>
    </AttDef>

    <!--***  BoundaryCondition Definitions ***-->
    <AttDef Type="BoundaryCondition" BaseType="" Abstract="1" Version="0" Unique="true" Associations="e"></AttDef>
    <AttDef Type="VelocityBound" Label="Velocity BC" BaseType="BoundaryCondition" Version="0" Unique="true" Nodal="true" Associations="e">
      <ItemDefinitions>
        <Group Name="DirichletVelocity" Label="Dirichlet velocity:" Version="0" AdvanceLevel="0" NumberOfRequiredGroups="1">
          <ItemDefinitions>
            <Double Name="Value1" Label="Value1" Version="0" NumberOfRequiredValues="1">
              <ExpressionType>PolyLinearFunction</ExpressionType>
              <Categories>
                <Cat>Flow</Cat>
              </Categories>
            </Double>

            <Double Name="Value2" Label="Value2" Version="0" NumberOfRequiredValues="1">
              <ExpressionType>PolyLinearFunction</ExpressionType>
              <Categories>
                <Cat>Flow</Cat>
              </Categories>
            </Double>
          </ItemDefinitions>
        </Group>
      </ItemDefinitions>
    </AttDef>

    <AttDef Type="VelocityAndDepthBound" Label="Velocity and Depth BC" BaseType="BoundaryCondition" Version="0" Unique="true" Nodal="true"
    Associations="e">
      <ItemDefinitions>
        <Group Name="DirichletVelocityDepth" Label="Dirichlet velocity and depth:" Version="0" AdvanceLevel="0" NumberOfRequiredGroups="1">
          <ItemDefinitions>
            <Double Name="Value1" Label="Value1" Version="0" NumberOfRequiredValues="1">
              <ExpressionType>PolyLinearFunction</ExpressionType>
              <Categories>
                <Cat>Flow</Cat>
              </Categories>
            </Double>
            <Double Name="Value2" Label="Value2" Version="0" NumberOfRequiredValues="1">
              <ExpressionType>PolyLinearFunction</ExpressionType>
              <Categories>
                <Cat>Flow</Cat>
              </Categories>
            </Double>
            <Double Name="Value3" Label="Value3" Version="0" NumberOfRequiredValues="1">
              <ExpressionType>PolyLinearFunction</ExpressionType>
              <Categories>
                <Cat>Flow</Cat>
              </Categories>
            </Double>
          </ItemDefinitions>
        </Group>
      </ItemDefinitions>
    </AttDef>

    <AttDef Type="LidElevation" Label="Lid Elevation BC" BaseType="BoundaryCondition"
            Version="0" Unique="true" Nodal="true" Associations="e">
      <ItemDefinitions>
        <Double Name="Value" Label="Stationary lid elevation:" Version="0" NumberOfRequiredValues="1">
          <ExpressionType>PolyLinearFunction</ExpressionType>
          <Categories>
            <Cat>Flow</Cat>
          </Categories>
        </Double>
      </ItemDefinitions>
    </AttDef>

    <AttDef Type="WaterDepthLid" Label="Water Depth Lid BC" BaseType="BoundaryCondition"
            Version="0" Unique="true" Nodal="true" Associations="e">
      <ItemDefinitions>
        <Double Name="Value" Label="Water depth under stationary lid:" Version="0" NumberOfRequiredValues="1">
          <ExpressionType>PolyLinearFunction</ExpressionType>
          <Categories>
            <Cat>Flow</Cat>
          </Categories>
        </Double>
      </ItemDefinitions>
    </AttDef>
    <AttDef Type="FloatingStationary" Label="Floating Stationary BC" BaseType="BoundaryCondition"
            Version="0" Unique="true" Nodal="true" Associations="e">
      <ItemDefinitions>
        <Double Name="Value" Label="Floating stationary object:" Version="0" NumberOfRequiredValues="1">
          <ExpressionType>PolyLinearFunction</ExpressionType>
          <Categories>
            <Cat>Flow</Cat>
          </Categories>
        </Double>
      </ItemDefinitions>
    </AttDef>
    <AttDef Type="TotalDischarge" Label="Total Discharge BC" BaseType="BoundaryCondition"
            Version="0" Unique="true" Nodal="true" Associations="e">
      <ItemDefinitions>
        <Double Name="Value" Label="Total discharge:" Version="0" NumberOfRequiredValues="1">
          <ExpressionType>PolyLinearFunction</ExpressionType>
          <Categories>
            <Cat>Flow</Cat>
          </Categories>
        </Double>
      </ItemDefinitions>
    </AttDef>
    <AttDef Type="UnitFlow" Label="Unit Flow BC" BaseType="BoundaryCondition" Version="0" Unique="true" Associations="e">
      <ItemDefinitions>
        <Double Name="Value" Label="Unit flow:" Version="0" NumberOfRequiredValues="1">
          <ExpressionType>PolyLinearFunction</ExpressionType>
          <Categories>
            <Cat>Flow</Cat>
          </Categories>
        </Double>
      </ItemDefinitions>
    </AttDef>
    <AttDef Type="WaterSurfElev" Label="Water Surf Elev BC" BaseType="BoundaryCondition" Version="0" Unique="true" Associations="e">
      <ItemDefinitions>
        <Double Name="Value" Label="Water surface elevation:" Version="0" NumberOfRequiredValues="1">
          <ExpressionType>PolyLinearFunction</ExpressionType>
          <Categories>
            <Cat>Flow</Cat>
          </Categories>
        </Double>
      </ItemDefinitions>
    </AttDef>

    <!-- Constituent transport BCs -->
    <AttDef Type="TransportBoundaryCondition" BaseType="BoundaryCondition" Abstract="1" Version="0"
            Unique="false" Associations="e">
      <ItemDefinitions>
        <AttributeRef Name="Constituent" Label="Constituent" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
          <AttDef>Constituent</AttDef>
        </AttributeRef>
      </ItemDefinitions>
    </AttDef>

    <AttDef Type="DirichletTransport" Label="Dirichlet transport:" BaseType="TransportBoundaryCondition"
       Nodal="true" Version="0" Unique="true" Associations="e">
      <ItemDefinitions>
        <Double Name="Value" Version="0" NumberOfRequiredValues="1">
          <ExpressionType>PolyLinearFunction</ExpressionType>
          <Categories>
            <Cat>Constituent</Cat>
          </Categories>
        </Double>
      </ItemDefinitions>
    </AttDef>

    <AttDef Type="NaturalTransport" Label="Natural transport:" BaseType="TransportBoundaryCondition"
       Version="0" Unique="true" Associations="e">
      <ItemDefinitions>
        <Double Name="Value" Version="0" NumberOfRequiredValues="1">
          <ExpressionType>PolyLinearFunction</ExpressionType>
          <Categories>
            <Cat>Constituent</Cat>
          </Categories>
        </Double>
      </ItemDefinitions>
    </AttDef>

    <!--***  Expression Definitions ***-->
    <AttDef Type="SimExpression" Abstract="1" Association="None" />
    <AttDef Type="SimInterpolation" BaseType="SimExpression" Abstract="1" />
    <AttDef Type="PolyLinearFunction" Label="PolyLinear Function" BaseType="SimInterpolation" Version="0" Unique="true" Associations="">
      <ItemDefinitions>
        <Group Name="ValuePairs" Label="Value Pairs" NumberOfRequiredGroups="1">
          <ItemDefinitions>
            <Double Name="X" Version="0" AdvanceLevel="0" NumberOfRequiredValues="0" Extensible="true"/>
            <Double Name="Value" Version="0" AdvanceLevel="0" NumberOfRequiredValues="0" Extensible="true" />
          </ItemDefinitions>
        </Group>
        <String Name="Sim1DLinearExp" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1"/>
      </ItemDefinitions>
    </AttDef>

    <!--***  Solvers Definitions ***-->
    <AttDef Type="Solvers" Label="Solvers" BaseType="" Version="0" Unique="true" Associations="">
      <ItemDefinitions>
        <Int Name="MemoryIncrementBlockSize" Label="Block size for memory increment:" Version="0" AdvanceLevel="1" NumberOfRequiredValues="1">
          <Categories>
            <Cat>General</Cat>
          </Categories>
          <DefaultValue>40</DefaultValue>
          <RangeInfo>
            <Min Inclusive="false">0</Min>
          </RangeInfo>
        </Int>
        <Int Name="PreconditioningBlocks" Label="Subdomain blocks per processor for preconditioning:" Version="0" AdvanceLevel="1" NumberOfRequiredValues="1">
          <Categories>
            <Cat>General</Cat>
          </Categories>
          <DefaultValue>1</DefaultValue>
          <RangeInfo>
            <Min Inclusive="false">0</Min>
          </RangeInfo>
        </Int>
        <Int Name="PreconditionerType" Label="Preconditioner Type:" Version="0" AdvanceLevel="1" NumberOfRequiredValues="1">
          <Categories>
            <Cat>General</Cat>
          </Categories>
          <DiscreteInfo DefaultIndex="0">
            <Value Enum="None">0</Value>
            <Value Enum="1 Level Additive Schwartz">1</Value>
            <Value Enum="2 Level Additive Schwartz">2</Value>
            <Value Enum="2 Level Hybrid">3</Value>
          </DiscreteInfo>
        </Int>
        <Double Name="TemporalSchemeCoefficient" Label="Coefficient for the second order temporal scheme:" Version="0" AdvanceLevel="1" NumberOfRequiredValues="1"
        Optional="true" IsEnabledByDefault="false">
          <RangeInfo>
            <Min Inclusive="true">0.0</Min>
            <Max Inclusive="true">1.0</Max>
          </RangeInfo>
          <Categories>
            <Cat>General</Cat>
          </Categories>
          <DefaultValue>0.0</DefaultValue>
        </Double>
        <Double Name="PetrovGalerkinCoefficient" Label="Coefficient for the Petrov-Galerkin equation:" Version="0" AdvanceLevel="1" NumberOfRequiredValues="1"
        Optional="true" IsEnabledByDefault="false">
          <RangeInfo>
            <Min Inclusive="true">0.0</Min>
            <Max Inclusive="true">0.5</Max>
          </RangeInfo>
          <Categories>
            <Cat>General</Cat>
          </Categories>
          <DefaultValue>0.0</DefaultValue>
        </Double>
        <Void Name="VesselMovement" Label="Enable vessel movement:" Version="0" AdvanceLevel="0" NumberOfRequiredValues="0"
        Optional="true" IsEnabledByDefault="false">
          <Categories>
            <Cat>General</Cat>
          </Categories>
        </Void>
        <Void Name="VesselEntrainment" Label="Enable vessel entrainment:" Version="0" AdvanceLevel="0" NumberOfRequiredValues="0"
        Optional="true" IsEnabledByDefault="false">
          <Categories>
            <Cat>General</Cat>
          </Categories>
        </Void>
        <Void Name="SW2Gradients" Label="Compute SW2 gradients:" Version="0" AdvanceLevel="1" NumberOfRequiredValues="0"
        Optional="true" IsEnabledByDefault="false">
          <Categories>
            <Cat>General</Cat>
          </Categories>
        </Void>
        <Double Name="NonLinearTolMaxNorm" Label="Non-Linear Tolerance (Max Norm):" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
          <RangeInfo>
            <Min Inclusive="false">0.0</Min>
          </RangeInfo>
          <Categories>
            <Cat>General</Cat>
          </Categories>
          <DefaultValue>1.0E-04</DefaultValue>
        </Double>
        <Double Name="NonLinearTolMaxChange" Label="Non-Linear Tolerance (Maximum Change in Iteration):" Version="0" AdvanceLevel="1" NumberOfRequiredValues="1"
        Optional="true" IsEnabledByDefault="false">
          <RangeInfo>
            <Min Inclusive="false">0.0</Min>
          </RangeInfo>
          <Categories>
            <Cat>General</Cat>
          </Categories>
          <DefaultValue>1.0E-02</DefaultValue>
        </Double>
        <Int Name="MaxNonLinearIters" Label="Maximum nonlinear iterations per time step:" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
          <RangeInfo>
            <Min Inclusive="false">0</Min>
          </RangeInfo>
          <Categories>
            <Cat>General</Cat>
          </Categories>
          <DefaultValue>6</DefaultValue>
        </Int>
        <Int Name="MaxLinearIters" Label="Maximum linear iterations per nonlinear iteration:" Version="0"
             AdvanceLevel="1" Optional="true" IsEnabledByDefault="false" NumberOfRequiredValues="1">
          <RangeInfo>
            <Min Inclusive="false">0</Min>
          </RangeInfo>
          <Categories>
            <Cat>General</Cat>
          </Categories>
          <DefaultValue>200</DefaultValue>
        </Int>
        <Void Name="RungeKuttaTol" Label="Runge-Kutta tolerance for reactive constituents:" Version="0" AdvanceLevel="1" NumberOfRequiredValues="0"
        Optional="true" IsEnabledByDefault="false">
          <Categories>
            <Cat>General</Cat>
          </Categories>
        </Void>
        <Double Name="HydrodynamicsTol" Label="Quasi-unsteady tolerance for hydrodynamics:" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
          <RangeInfo>
            <Min Inclusive="false">0.0</Min>
          </RangeInfo>
          <Categories>
            <Cat>General</Cat>
          </Categories>
          <DefaultValue>1.0E-04</DefaultValue>
        </Double>
      </ItemDefinitions>
    </AttDef>

    <!--*** Time Definitions ***-->
    <AttDef Type="Time" Label="Time" BaseType="" Version="0" Unique="true" Associations="">
      <ItemDefinitions>
        <Double Name="TimestepSize" Label="Timestep size:" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
          <ExpressionType>PolyLinearFunction</ExpressionType>
          <Categories>
            <Cat>General</Cat>
          </Categories>
        </Double>
        <Group Name="StartTime" Label="Start Time:" Version="0" AdvanceLevel="0" NumberOfRequiredGroups="1">
          <ItemDefinitions>
            <Double Name="Value" Label="Value" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
              <RangeInfo>
                <Min Inclusive="true">0.0</Min>
              </RangeInfo>
              <DefaultValue>0.0</DefaultValue>
              <Categories>
                <Cat>General</Cat>
              </Categories>
            </Double>
            <Int Name="Units" Label="Units" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
              <DiscreteInfo DefaultIndex="0">
                <Value Enum="Seconds">0</Value>
                <Value Enum="Minutes">1</Value>
                <Value Enum="Hours">2</Value>
                <Value Enum="Days">3</Value>
                <Value Enum="Weeks">4</Value>
              </DiscreteInfo>
              <DefaultIndex>0</DefaultIndex>
            </Int>
          </ItemDefinitions>
        </Group>
        <Group Name="EndTime" Label="End Time:" Version="0" AdvanceLevel="0" NumberOfRequiredGroups="1">
          <ItemDefinitions>
            <Double Name="Value" Label="Value" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
              <RangeInfo>
                <Min Inclusive="false">0.0</Min>
              </RangeInfo>
              <DefaultValue>0.0</DefaultValue>
              <Categories>
                <Cat>General</Cat>
              </Categories>
            </Double>
            <Int Name="Units" Label="Units" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
              <DiscreteInfo DefaultIndex="0">
                <Value Enum="Seconds">0</Value>
                <Value Enum="Minutes">1</Value>
                <Value Enum="Hours">2</Value>
                <Value Enum="Days">3</Value>
                <Value Enum="Weeks">4</Value>
              </DiscreteInfo>
              <DefaultIndex>0</DefaultIndex>
            </Int>
          </ItemDefinitions>
        </Group>
        <Double Name="SteadyStateSolveParams" Label="Steady state solve parameters:" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1"
        Optional="true" IsEnabledByDefault="false">
          <RangeInfo>
            <Min Inclusive="false">0.0</Min>
          </RangeInfo>
          <Categories>
            <Cat>General</Cat>
          </Categories>
        </Double>
        <Group Name="QuasiUnsteadyParams" Label="Parameters defining quasi-unsteady simulations:" Version="0" AdvanceLevel="0" NumberOfRequiredGroups="1"
        Optional="true" IsEnabledByDefault="false">
          <ItemDefinitions>
            <Double Name="SteadyStateHydrodynamicCondition" Label="Steady State Hydrodynamic Condition" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
              <BriefDescription>Select Function</BriefDescription>
              <Categories>
                <Cat>General</Cat>
              </Categories>
              <ExpressionType>PolyLinearFunction</ExpressionType>
            </Double>
            <Int Name="MaxIterations" Label="Max iterations for steady state solve" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
              <RangeInfo>
                <Min Inclusive="false">0</Min>
              </RangeInfo>
              <Categories>
                <Cat>General</Cat>
              </Categories>
            </Int>
            <Double Name="InitialTimeStep" Label="Initial time step for steady state calculation" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
              <RangeInfo>
                <Min Inclusive="false">0.0</Min>
              </RangeInfo>
              <Categories>
                <Cat>General</Cat>
              </Categories>
            </Double>
          </ItemDefinitions>
        </Group>
        <Group Name="AutoTimeStepFind" Label="Automatic timestep find:" Version="0" AdvanceLevel="0" NumberOfRequiredGroups="1"
        Optional="true" IsEnabledByDefault="false">
          <ItemDefinitions>
            <Double Name="InitialTimeStep" Label="Initial Time Step Size" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
              <Categories>
                <Cat>General</Cat>
              </Categories>
              <RangeInfo>
                <Min Inclusive="false">0.0</Min>
              </RangeInfo>
            </Double>
            <Double Name="TimeSeries" Label="Time Series">
              <Categories>
                <Cat>General</Cat>
              </Categories>
              <ExpressionType>PolyLinearFunction</ExpressionType>
            </Double>
          </ItemDefinitions>
        </Group>
        <Void Name="PrintAdaptedMeshes" Label="Print adapted meshes:" Version="0" AdvanceLevel="0" NumberOfRequiredValues="0"
        Optional="true" IsEnabledByDefault="false">
          <Categories>
            <Cat>General</Cat>
          </Categories>
        </Void>
        <Int Name="HotStartFile" Label="Hotstart File" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1"
        Optional="true" IsEnabledByDefault="false">
          <RangeInfo>
            <Min Inclusive="false">0</Min>
          </RangeInfo>
        </Int>
        <Int Name="OutputSeries" Label="Output Series" Optional="true" IsEnabledByDefault="false">
          <Categories>
            <Cat>Time</Cat>
          </Categories>
          <ChildrenDefinitions>
            <Double Name="OutputFunction" Label="Output Function">
              <ExpressionType>PolyLinearFunction</ExpressionType>
            </Double>
            <Int Name="OutputUnits" Label="Output Units" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
              <DiscreteInfo DefaultIndex="0">
                <Value Enum="Seconds">0</Value>
                <Value Enum="Minutes">1</Value>
                <Value Enum="Hours">2</Value>
                <Value Enum="Days">3</Value>
                <Value Enum="Weeks">4</Value>
              </DiscreteInfo>
            </Int>
            <Group Name="TimeSeriesData" Label="Series" Extensible="true" NumberOfRequiredGroups="1" >
              <ItemDefinitions>
                <Double Name="StartTime" Label="Start Time">
                  <RangeInfo>
                    <Min Inclusive="true">0.0</Min>
                  </RangeInfo>
                </Double>
                <Double Name="EndTime" Label="End Time">
                  <RangeInfo>
                    <Min Inclusive="false">0.0</Min>
                  </RangeInfo>
                </Double>
                <Double Name="TimeInterval" Label="Progression Interval">
                </Double>
                <Int Name="Units" Label="Input Units" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
                  <DiscreteInfo DefaultIndex="0">
                    <Value Enum="Seconds">0</Value>
                    <Value Enum="Minutes">1</Value>
                    <Value Enum="Hours">2</Value>
                    <Value Enum="Days">3</Value>
                    <Value Enum="Weeks">4</Value>
                  </DiscreteInfo>
                </Int>
              </ItemDefinitions>
            </Group>
          </ChildrenDefinitions>
          <DiscreteInfo DefaultIndex="0">
            <Structure>
              <Value Enum="Function">0</Value>
              <Items>
                <Item>OutputFunction</Item>
              </Items>
            </Structure>
            <Structure>
              <Value Enum="Auto-Build">1</Value>
              <Items>
                <Item>OutputUnits</Item>
                <Item>TimeSeriesData</Item>
                <Item>EndTime</Item>
              </Items>
            </Structure>
          </DiscreteInfo>
        </Int>

      </ItemDefinitions>
    </AttDef>

    <!--***  Globals Definitions ***-->
    <AttDef Type="Globals" Label="Globals" BaseType="" Version="0" Unique="true">
      <ItemDefinitions>
        <Double Name="Gravity" Label="Gravity:" Units="m/s^2" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
          <Categories>
            <Cat>General</Cat>
          </Categories>
          <RangeInfo>
            <Min Inclusive="false">0.0</Min>
          </RangeInfo>
          <DefaultValue>9.8</DefaultValue>
        </Double>
        <Double Name="KinMolViscosity" Label="Kinematic molecular viscosity:" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
          <Categories>
            <Cat>General</Cat>
          </Categories>
          <RangeInfo>
            <Min Inclusive="false">0.0</Min>
          </RangeInfo>
          <DefaultValue>1.0e-06</DefaultValue>
        </Double>
        <Double Name="ReferenceDensity" Label="Fluid density:" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
          <Categories>
            <Cat>General</Cat>
          </Categories>
          <RangeInfo>
            <Min Inclusive="false">0.0</Min>
          </RangeInfo>
          <DefaultValue>1000.0</DefaultValue>
        </Double>
        <Double Name="ManningsUnitConstant" Label="Manning's unit constant (1.0 for SI, 1.486 for English):" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
          <Categories>
            <Cat>General</Cat>
          </Categories>
          <RangeInfo>
            <Min Inclusive="false">0.0</Min>
          </RangeInfo>
          <DefaultValue>1.0</DefaultValue>
        </Double>
        <Double Name="WetDryLimits" Label="Wetting/drying limits:" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
          <Categories>
            <Cat>General</Cat>
          </Categories>
          <RangeInfo>
            <Min Inclusive="true">0.0</Min>
          </RangeInfo>
          <DefaultValue>0.0</DefaultValue>
        </Double>
      </ItemDefinitions>
    </AttDef>
  </Definitions>

  <!--**********  Attribute Instances ***********-->
  <Attributes>
  </Attributes>

</SMTK_AttributeManager>
