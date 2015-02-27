<?xml version="1.0"?>
<!--Created by XmlV1StringWriter-->
<SMTK_AttributeManager Version="1">
  <!--**********  Category and Analysis Infomation ***********-->
  <Categories>
    <Cat>Time</Cat>
    <Cat>Flow</Cat>
    <Cat>General</Cat>
    <Cat>Heat</Cat>
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
    <!--***  Material Definitions ***-->
    <AttDef Type="Material" Label="Material" BaseType="" Version="0" Unique="true" Associations="rf">
      <ItemDefinitions>
        <!--
            <Color Name="MaterialColor" Version="0" AdvanceLevel="0" Label="Display Color">
            <DefaultValue>1.0 1.0 1.0</DefaultValue>
            <Categories>
            <Cat>General</Cat>
            </Categories>
            </Color>
        -->
        <Double Name="Porosity" Label="Porosity" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
          <RangeInfo>
            <Min Inclusive="true">0</Min>
            <Max Inclusive="true">1</Max>
          </RangeInfo>
          <DefaultValue>0.4</DefaultValue>
          <Categories>
            <Cat>General</Cat>
          </Categories>
        </Double>

        <Group Name="HydraulicConductivity" Label="Saturated hydraulic conductivity tensor" Version="0" AdvanceLevel="1" NumberOfRequiredGroups="1">
          <ItemDefinitions>
            <Double Name="Value1" Label="XX" Version="0" NumberOfRequiredValues="1" Units="L/T">
              <RangeInfo>
                <Min Inclusive="false">0</Min>
              </RangeInfo>
              <Categories>
                <Cat>Flow</Cat>
              </Categories>
              <DefaultValue>1.0</DefaultValue>
            </Double>
            <Double Name="Value2" Label="YY" Version="0" NumberOfRequiredValues="1" Units="L/T">
              <RangeInfo>
                <Min Inclusive="false">0</Min>
              </RangeInfo>
              <Categories>
                <Cat>Flow</Cat>
              </Categories>
              <DefaultValue>1.0</DefaultValue>
            </Double>
            <Double Name="Value3" Label="ZZ" Version="0" NumberOfRequiredValues="1" Units="L/T">
              <RangeInfo>
                <Min Inclusive="false">0</Min>
              </RangeInfo>
              <Categories>
                <Cat>Flow</Cat>
              </Categories>
              <DefaultValue>1.0</DefaultValue>
            </Double>
            <Double Name="Value4" Label="XY" Version="0" NumberOfRequiredValues="1" Units="L/T">
              <RangeInfo>
                <Min Inclusive="true">0</Min>
              </RangeInfo>
              <Categories>
                <Cat>Flow</Cat>
              </Categories>
              <DefaultValue>0.0</DefaultValue>
            </Double>
            <Double Name="Value5" Label="XZ" Version="0" NumberOfRequiredValues="1" Units="L/T">
              <RangeInfo>
                <Min Inclusive="true">0</Min>
              </RangeInfo>
              <Categories>
                <Cat>Flow</Cat>
              </Categories>
              <DefaultValue>0.0</DefaultValue>
            </Double>
            <Double Name="Value6" Label="YZ" Version="0" NumberOfRequiredValues="1" Units="L/T">
              <RangeInfo>
                <Min Inclusive="true">0</Min>
              </RangeInfo>
              <Categories>
                <Cat>Flow</Cat>
              </Categories>
              <DefaultValue>0.0</DefaultValue>
            </Double>
          </ItemDefinitions>
        </Group>

        <Double Name="PresSatCurveIndex" Label="Pressure-saturation function" Version="0" AdvanceLevel="1" NumberOfRequiredValues="1"
                Optional="true" IsEnabledByDefault="false">
          <ExpressionType>PolyLinearFunction</ExpressionType>
          <Categories>
            <Cat>Flow</Cat>
          </Categories>
        </Double>

      </ItemDefinitions>
    </AttDef>

    <!--***  BoundaryCondition Definitions ***-->
    <AttDef Type="SurfaceBoundaryCondition" BaseType="" Abstract="1" Version="0" Unique="true" Associations="f">
      <!--
          <ItemDefinitions>
          <Color Name="BCColor" Version="0" AdvanceLevel="0" Label="Dispaly Color">
          <DefaultValue>1.0 0.0 0.0</DefaultValue>
          <Categories>
          <Cat>General</Cat>
          </Categories>
          </Color>
          </ItemDefinitions>
      -->
    </AttDef>

    <AttDef Type="SpecifiedHead" Label="Specified Head" BaseType="SurfaceBoundaryCondition" Version="0" Unique="true" Nodal="true" Associations="f">
      <ItemDefinitions>
        <Double Name="Value" Label="Specified head" Version="0" NumberOfRequiredValues="1">
          <ExpressionType>PolyLinearFunction</ExpressionType>
          <Categories>
            <Cat>Flow</Cat>
          </Categories>
        </Double>
      </ItemDefinitions>
    </AttDef>
    <AttDef Type="SpecifiedFlux" Label="Specified Flux" BaseType="SurfaceBoundaryCondition" Version="0" Unique="true" Associations="f">
      <ItemDefinitions>
        <Double Name="Value" Label="Specified flux" Version="0" NumberOfRequiredValues="1">
          <ExpressionType>PolyLinearFunction</ExpressionType>
          <Categories>
            <Cat>Flow</Cat>
          </Categories>
        </Double>
      </ItemDefinitions>
    </AttDef>
    <AttDef Type="FlowInjectionWell" Label="Flow Injection Well" BaseType="SurfaceBoundaryCondition" Version="0" Unique="true" Nodal="true" Associations="f">
      <ItemDefinitions>
        <Double Name="Value" Label="Injection well for flow" Version="0" NumberOfRequiredValues="1">
          <ExpressionType>PolyLinearFunction</ExpressionType>
          <Categories>
            <Cat>Flow</Cat>
          </Categories>
        </Double>
      </ItemDefinitions>
    </AttDef>

    <AttDef Type="EdgeBoundaryCondition" BaseType="" Abstract="1" Version="0" Unique="true" Associations="e">
    </AttDef>

    <AttDef Type="VelocityBound" Label="Velocity Bound" BaseType="EdgeBoundaryCondition" Version="0" Unique="true" Nodal="true" Associations="e">
      <ItemDefinitions>
        <Group Name="dirichletvelocity" Label="Dirichlet velocity:" NumberOfRequiredGroups="1" >
          <ItemDefinitions>
            <Double Name="Value1" Version="0" NumberOfRequiredValues="1">
              <ExpressionType>PolyLinearFunction</ExpressionType>
              <Categories>
                <Cat>Flow</Cat>
              </Categories>
            </Double>

            <Double Name="Value2" Version="0" NumberOfRequiredValues="1">
              <ExpressionType>PolyLinearFunction</ExpressionType>
              <Categories>
                <Cat>Flow</Cat>
              </Categories>
            </Double>
          </ItemDefinitions>
        </Group>
      </ItemDefinitions>
    </AttDef>

    <!--***  Expression Definitions ***-->
    <AttDef Type="SimExpression" Abstract="1" Association="None"/>
    <AttDef Type="SimInterpolation" BaseType="SimExpression" Abstract="1"/>
    <AttDef Type="PolyLinearFunction" Label="PolyLinear Function" BaseType="SimInterpolation" Version="0" Unique="true" Associations="">
      <ItemDefinitions>
        <Group Name="ValuePairs" Label="Value Pairs" NumberOfRequiredGroups="1">
          <ItemDefinitions>
            <Double Name="X" Version="0" AdvanceLevel="0" NumberOfRequiredValues="0"/>
            <Double Name="Value" Version="0" AdvanceLevel="0" NumberOfRequiredValues="0"/>
          </ItemDefinitions>
        </Group>
        <String Name="Sim1DLinearExp" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1" />
      </ItemDefinitions>
    </AttDef>

    <!--***  Solvers Definitions ***-->
    <AttDef Type="Solvers" Label="Solvers" BaseType="" Version="0" Unique="true" Associations="">
      <ItemDefinitions>
        <Int Name="MaxNonLinearIters" Label="Maximum number of non-linear iterations per timestep:" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
          <DefaultValue>10</DefaultValue>
          <RangeInfo>
            <Min Inclusive="false">0</Min>
          </RangeInfo>
        </Int>
        <Double Name="NonLinearTolMaxNorm" Label="Non-linear tolerance (absolute):" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
          <DefaultValue>1.0E-10</DefaultValue>
          <RangeInfo>
            <Min Inclusive="false">0</Min>
          </RangeInfo>
        </Double>
        <Double Name="NonLinearTolMaxChange" Label="Non-linear tolerance (increment):" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1"
                Optional="true" IsEnabledByDefault="true">
          <DefaultValue>1.0E-5</DefaultValue>
          <RangeInfo>
            <Min Inclusive="false">0</Min>
          </RangeInfo>
        </Double>
        <Int Name="MaxLinearIters" Label="Maximum number of linear iterations per non-linear iteration:" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
          <DefaultValue>500</DefaultValue>
          <RangeInfo>
            <Min Inclusive="false">0</Min>
          </RangeInfo>
        </Int>
        <Int Name="PreconditioningBlocks" Label="Number of Block-Jacobi blocks per processor: (used for some preconditioners)" Version="0" AdvanceLevel="1" NumberOfRequiredValues="1">
          <DefaultValue>8</DefaultValue>
          <RangeInfo>
            <Min Inclusive="false">0</Min>
          </RangeInfo>
        </Int>
        <Int Name="PreconditionerType" Label="Preconditioner:" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
          <DiscreteInfo DefaultIndex="0">
            <Value Enum="Point Jacobi">0</Value>
            <Value Enum="Block Jacobi">1</Value>
            <Value Enum="Additive Schwartz">2</Value>
            <Value Enum="Hybrid Schwartz">3</Value>
            <Value Enum="LAPACK Dense Direct Solve">4</Value>
            <Value Enum="BiCG-STAB">5</Value>
          </DiscreteInfo>
        </Int>
        <Int Name="MemoryIncrementBlockSize" Label="Block size for memory increment:" Version="0" AdvanceLevel="1" NumberOfRequiredValues="1">
          <DefaultValue>1000</DefaultValue>
          <RangeInfo>
            <Min Inclusive="false">0</Min>
          </RangeInfo>
        </Int>
      </ItemDefinitions>
    </AttDef>
  </Definitions>

  <!--**********  Attribute Instances ***********-->
  <Attributes>
    <Att Name="GEOSolvers" Type="Solvers" ID="1">
      <Items>
        <Int Name="MaxNonLinearIters">20</Int>
        <Double Name="NonLinearTolMaxNorm">10.0E-10</Double>
        <Double Name="NonLinearTolMaxChange">1.0E-5</Double>
        <Int Name="MaxLinearIters">100</Int>
        <Int Name="PreconditioningBlocks">8</Int>
        <Int Name="PreconditionerType">2</Int>
        <Int Name="MemoryIncrementBlockSize">1000</Int>
      </Items>
    </Att>
  </Attributes>

  <!--********** Workflow Sections ***********-->
  <RootSection Title="SimBuilder">
    <DefaultColor>1., 1., 0.5, 1.</DefaultColor>
    <InvalidColor>1, 0.5, 0.5, 1</InvalidColor>

    <InstancedSection Title="Solvers">
      <InstancedAttributes>
        <Att Type="Solvers">Solvers</Att>
      </InstancedAttributes>
    </InstancedSection>

    <SimpleExpressionSection Title="Functions">
      <Definition>PolyLinearFunction</Definition>
    </SimpleExpressionSection>

    <AttributeSection Title="Materials" ModelEntityFilter="r" CreateEntities="true">
      <AttributeTypes>
        <Type>Material</Type>
      </AttributeTypes>
    </AttributeSection>
    <AttributeSection Title="Surface-Based" ModelEntityFilter="f">
      <AttributeTypes>
        <Type>SpecifiedHead</Type>
        <Type>SpecifiedFlux</Type>
        <Type>FlowInjectionWell</Type>
      </AttributeTypes>
    </AttributeSection>
    <AttributeSection Title="Edge-Based" ModelEntityFilter="e">
      <AttributeTypes>
        <Type>VelocityBound</Type>
      </AttributeTypes>
    </AttributeSection>
    <ModelEntitySection Title="Domains" ModelEntityFilter="rf" />
    <ModelEntitySection Title="Boundary View" ModelEntityFilter="fe" />

  </RootSection>
</SMTK_AttributeManager>
