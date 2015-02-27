<?xml version="1.0"?>
<!--Created by XmlV1StringWriter-->
<SMTK_AttributeManager Version="1">
  <!--**********  Category and Analysis Infomation ***********-->
  <Categories>
    <Cat>Time</Cat>
    <Cat>Flow</Cat>
    <Cat>General</Cat>
    <Cat>Heat</Cat>
    <Cat>Spectral</Cat>
    <Cat>Veg</Cat>
  </Categories>
  <Analyses>
    <Analysis Type="Groundwater Flow">
      <Cat>Flow</Cat>
      <Cat>General</Cat>
      <Cat>Time</Cat>
    </Analysis>
    <Analysis Type="Groundwater Flow with Heat Transfer">
      <Cat>Flow</Cat>
      <Cat>General</Cat>
      <Cat>Heat</Cat>
      <Cat>Time</Cat>
    </Analysis>
    <Analysis Type="Veg Model">
      <Cat>General</Cat>
      <Cat>Veg</Cat>
    </Analysis>
    <Analysis Type="Ray Caster">
      <Cat>General</Cat>
      <Cat>Spectral</Cat>
    </Analysis>
  </Analyses>
  <!--**********  Attribute Definitions ***********-->
  <Definitions>
    <!--***  Expression Definitions ***-->
    <AttDef Type="SimExpression" Abstract="1" Association="None"/>
    <AttDef Type="SimInterpolation" BaseType="SimExpression" Abstract="1"/>
    <AttDef Type="PolyLinearFunction" Label="PolyLinear Function" BaseType="SimInterpolation" Version="0" Unique="true" Associations="">
      <ItemDefinitions>
        <Group Name="ValuePairs" Label="Value Pairs" NumberOfRequiredGroups="1">
          <ItemDefinitions>
            <Double Name="X" Version="0" AdvanceLevel="0" NumberOfRequiredValues="0" Extensible="true"/>
            <Double Name="Value" Version="0" AdvanceLevel="0" NumberOfRequiredValues="0" Extensible="true"/>
          </ItemDefinitions>
        </Group>
        <String Name="Sim1DLinearExp" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1" />
      </ItemDefinitions>
    </AttDef>

    <!--***  Material Definitions ***-->
    <AttDef Type="Material" Label="Material" BaseType="" Version="0" Unique="true" Associations="r">
      <ItemDefinitions>
        <Double Name="Porosity" Label="Porosity" Version="0" NumberOfRequiredValues="1">
          <Categories>
            <Cat>General</Cat>
          </Categories>
          <DefaultValue>0.4</DefaultValue>
          <RangeInfo>
            <Min Inclusive="true">0</Min>
            <Max Inclusive="true">1</Max>
          </RangeInfo>
        </Double>
        <Group Name="HydraulicConductivity" Label="Saturated hydraulic conductivity tensor&#10;  (XX, YY, ZZ, XY, XZ, YZ)" Version="0" NumberOfRequiredGroups="1">
          <ItemDefinitions>
            <Double Name="Value1" Label="XX" Version="0" NumberOfRequiredValues="1" Units="L/T">
              <Categories>
                <Cat>Flow</Cat>
              </Categories>
              <DefaultValue>1</DefaultValue>
              <RangeInfo>
                <Min Inclusive="false">0</Min>
              </RangeInfo>
            </Double>
            <Double Name="Value2" Label="YY" Version="0" NumberOfRequiredValues="1" Units="L/T">
              <Categories>
                <Cat>Flow</Cat>
              </Categories>
              <DefaultValue>1</DefaultValue>
              <RangeInfo>
                <Min Inclusive="false">0</Min>
              </RangeInfo>
            </Double>
            <Double Name="Value3" Label="ZZ" Version="0" NumberOfRequiredValues="1" Units="L/T">
              <Categories>
                <Cat>Flow</Cat>
              </Categories>
              <DefaultValue>1</DefaultValue>
              <RangeInfo>
                <Min Inclusive="false">0</Min>
              </RangeInfo>
            </Double>
            <Double Name="Value4" Label="XY" Version="0" NumberOfRequiredValues="1">
              <Categories>
                <Cat>Flow</Cat>
              </Categories>
              <DefaultValue>0</DefaultValue>
              <RangeInfo>
                <Min Inclusive="true">0</Min>
              </RangeInfo>
            </Double>
            <Double Name="Value5" Label="XZ" Version="0" NumberOfRequiredValues="1">
              <Categories>
                <Cat>Flow</Cat>
              </Categories>
              <DefaultValue>0</DefaultValue>
              <RangeInfo>
                <Min Inclusive="true">0</Min>
              </RangeInfo>
            </Double>
            <Double Name="Value6" Label="YZ" Version="0" NumberOfRequiredValues="1">
              <Categories>
                <Cat>Flow</Cat>
              </Categories>
              <DefaultValue>0</DefaultValue>
              <RangeInfo>
                <Min Inclusive="true">0</Min>
              </RangeInfo>
            </Double>
          </ItemDefinitions>
        </Group>
        <Double Name="PresSatCurveIndex" Label="Pressure-saturation function" Version="0" Optional="true" IsEnabledByDefault="false" AdvanceLevel="1" NumberOfRequiredValues="1">
          <Categories>
            <Cat>Flow</Cat>
          </Categories>Heat
          <ExpressionType>PolyLinearFunction</ExpressionType>
        </Double>
        <Double Name="PresRelCondCurveIndex" Label="Pressure-relative conductivity function" Version="0" Optional="true" IsEnabledByDefault="false" AdvanceLevel="1" NumberOfRequiredValues="1">
          <Categories>
            <Cat>Flow</Cat>
          </Categories>
          <ExpressionType>PolyLinearFunction</ExpressionType>
        </Double>
        <Double Name="SpecificStorage" Label="Specific storage" Version="0" AdvanceLevel="1" NumberOfRequiredValues="1">
          <Categories>
            <Cat>Flow</Cat>
          </Categories>
          <DefaultValue>1e-06</DefaultValue>
          <RangeInfo>
            <Min Inclusive="true">0</Min>
          </RangeInfo>
        </Double>
        <Double Name="ResidualSaturation" Label="Residual saturation (gravity drainage)" Version="0" NumberOfRequiredValues="1">
          <Categories>
            <Cat>Flow</Cat>
          </Categories>
          <DefaultValue>0.03</DefaultValue>
          <RangeInfo>
            <Min Inclusive="true">0</Min>
            <Max Inclusive="true">1</Max>
          </RangeInfo>
        </Double>
        <Double Name="VanGenuchtenAlpha" Label="van Genuchten function alpha coefficient" Version="0" NumberOfRequiredValues="1" Units="1/m">
          <Categories>
            <Cat>Flow</Cat>
          </Categories>
          <DefaultValue>0.1</DefaultValue>
          <RangeInfo>
            <Min Inclusive="false">0</Min>
          </RangeInfo>
        </Double>
        <Double Name="VanGenuchtenN" Label="van Genuchten function N exponent" Version="0" NumberOfRequiredValues="1">
          <Categories>
            <Cat>Flow</Cat>
          </Categories>
          <DefaultValue>1</DefaultValue>
          <RangeInfo>
            <Min Inclusive="false">0</Min>
          </RangeInfo>
        </Double>
        <Double Name="VanGenuchtenMaxCP" Label="van Genuchten max. capillary pressure head" Version="0" NumberOfRequiredValues="1" Units="m">
          <Categories>
            <Cat>Flow</Cat>
          </Categories>
          <DefaultValue>100</DefaultValue>
          <RangeInfo>
            <Min Inclusive="false">0</Min>
          </RangeInfo>
        </Double>
        <Int Name="VanGenuchtenNumXY" Label="Number of van Genuchten interpolation points" Version="0" NumberOfRequiredValues="1">
          <Categories>
            <Cat>Flow</Cat>
          </Categories>
          <DefaultValue>400</DefaultValue>
          <RangeInfo>
            <Min Inclusive="true">2</Min>
          </RangeInfo>
        </Int>
        <Double Name="SolidSpecificHeat" Label="Specific heat of solids" Version="0" AdvanceLevel="1" NumberOfRequiredValues="1" Units="W hr/g-C">
          <Categories>
            <Cat>Heat</Cat>
          </Categories>
          <DefaultValue>0.0002</DefaultValue>
          <RangeInfo>
            <Min Inclusive="false">0</Min>
          </RangeInfo>
        </Double>
        <Double Name="SolidSpecificGravity" Label="Specific gravity of solids" Version="0" AdvanceLevel="1" NumberOfRequiredValues="1">
          <Categories>
            <Cat>Heat</Cat>
          </Categories>
          <DefaultValue>2.65</DefaultValue>
          <RangeInfo>
            <Min Inclusive="false">0</Min>
          </RangeInfo>
        </Double>
        <Double Name="Albedo" Label="Albedo (reflectivity of SW energy)" Version="0" NumberOfRequiredValues="1">
          <Categories>
            <Cat>Heat</Cat>
          </Categories>
          <DefaultValue>0.3</DefaultValue>
          <RangeInfo>
            <Min Inclusive="true">0</Min>
            <Max Inclusive="true">1</Max>
          </RangeInfo>
        </Double>
        <Double Name="BulkEmissivity" Label="Broadband emissivity" Version="0" NumberOfRequiredValues="1">
          <Categories>
            <Cat>Heat</Cat>
          </Categories>
          <DefaultValue>0.8</DefaultValue>
          <RangeInfo>
            <Min Inclusive="true">0</Min>
            <Max Inclusive="true">1</Max>
          </RangeInfo>
        </Double>
        <Double Name="QuartzFraction" Label="Mass fraction of quartz" Version="0" AdvanceLevel="1" NumberOfRequiredValues="1">
          <Categories>
            <Cat>Heat</Cat>
          </Categories>
          <DefaultValue>0.3</DefaultValue>
          <RangeInfo>
            <Min Inclusive="true">0</Min>
            <Max Inclusive="true">1</Max>
          </RangeInfo>
        </Double>
        <Double Name="FractionSandGravel" Label="Mass fraction of coarse grains&#10;  (sand and gravel)" Version="0" AdvanceLevel="1" NumberOfRequiredValues="1">
          <Categories>
            <Cat>Heat</Cat>
          </Categories>
          <DefaultValue>0.3</DefaultValue>
          <RangeInfo>
            <Min Inclusive="true">0</Min>
            <Max Inclusive="true">1</Max>
          </RangeInfo>
        </Double>
        <Group Name="AnisotropyTensor" Label="Anisotropy tensor for thermal conductivity&#10;  (XX, YY, ZZ, XY, XZ, YZ)" Version="0" NumberOfRequiredGroups="1">
          <ItemDefinitions>
            <Double Name="Value1" Version="0" NumberOfRequiredValues="1">
              <Categories>
                <Cat>Heat</Cat>
              </Categories>
              <DefaultValue>1</DefaultValue>
              <RangeInfo>
                <Min Inclusive="false">0</Min>
              </RangeInfo>
            </Double>
            <Double Name="Value2" Version="0" NumberOfRequiredValues="1">
              <Categories>
                <Cat>Heat</Cat>
              </Categories>
              <DefaultValue>1</DefaultValue>
              <RangeInfo>
                <Min Inclusive="false">0</Min>
              </RangeInfo>
            </Double>
            <Double Name="Value3" Version="0" NumberOfRequiredValues="1">
              <Categories>
                <Cat>Heat</Cat>
              </Categories>
              <DefaultValue>1</DefaultValue>
              <RangeInfo>
                <Min Inclusive="false">0</Min>
              </RangeInfo>
            </Double>
            <Double Name="Value4" Version="0" NumberOfRequiredValues="1">
              <Categories>
                <Cat>Heat</Cat>
              </Categories>
              <DefaultValue>0</DefaultValue>
              <RangeInfo>
                <Min Inclusive="true">0</Min>
              </RangeInfo>
            </Double>
            <Double Name="Value5" Version="0" NumberOfRequiredValues="1">
              <Categories>
                <Cat>Heat</Cat>
              </Categories>
              <DefaultValue>0</DefaultValue>
              <RangeInfo>
                <Min Inclusive="true">0</Min>
              </RangeInfo>
            </Double>
            <Double Name="Value6" Version="0" NumberOfRequiredValues="1">
              <Categories>
                <Cat>Heat</Cat>
              </Categories>
              <DefaultValue>0</DefaultValue>
              <RangeInfo>
                <Min Inclusive="true">0</Min>
              </RangeInfo>
            </Double>
          </ItemDefinitions>
        </Group>
        <Double Name="DryThermalConductivity" Label="Dry mixture thermal conductivity" Version="0" AdvanceLevel="1" NumberOfRequiredValues="1" Units="W/mK">
          <Categories>
            <Cat>Heat</Cat>
          </Categories>
          <DefaultValue>0.3</DefaultValue>
          <RangeInfo>
            <Min Inclusive="true">0</Min>
          </RangeInfo>
        </Double>
        <Double Name="SatThermalConductivity" Label="Saturated mixture thermal conductivity" Version="0" AdvanceLevel="1" NumberOfRequiredValues="1" Units="W/mK">
          <Categories>
            <Cat>Heat</Cat>
          </Categories>
          <DefaultValue>0.3</DefaultValue>
          <RangeInfo>
            <Min Inclusive="true">0</Min>
          </RangeInfo>
        </Double>
        <Double Name="Tortuosity" Label="Tortuosity:" Version="0" AdvanceLevel="1" NumberOfRequiredValues="1">
          <Categories>
            <Cat>Heat</Cat>
          </Categories>
          <DefaultValue>0.7</DefaultValue>
          <RangeInfo>
            <Min Inclusive="true">0</Min>
            <Max Inclusive="true">1</Max>
          </RangeInfo>
        </Double>
        <Double Name="LongitudinalDispersivity" Label="Longitudinal dispersivity" Version="0" AdvanceLevel="1" NumberOfRequiredValues="1" Units="L">
          <Categories>
            <Cat>Heat</Cat>
          </Categories>
          <DefaultValue>1</DefaultValue>
          <RangeInfo>
            <Min Inclusive="false">0</Min>
          </RangeInfo>
        </Double>
        <Double Name="TransverseDispersivity" Label="Transverse dispersivity" Version="0" AdvanceLevel="1" NumberOfRequiredValues="1" Units="L">
          <Categories>
            <Cat>Heat</Cat>
          </Categories>
          <DefaultValue>0.1</DefaultValue>
          <RangeInfo>
            <Min Inclusive="false">0</Min>
          </RangeInfo>
        </Double>
        <Int Name="MaxRefineLevels" Label="Maximum levels of refinement" Version="0" AdvanceLevel="1" NumberOfRequiredValues="1">
          <Categories>
            <Cat>General</Cat>
          </Categories>
          <DefaultValue>0</DefaultValue>
          <RangeInfo>
            <Min Inclusive="true">0</Min>
          </RangeInfo>
        </Int>
        <Double Name="FlowRefineTol" Label="Refinement tolerance for flow" Version="0" AdvanceLevel="1" NumberOfRequiredValues="1">
          <Categories>
            <Cat>Flow</Cat>
          </Categories>
          <DefaultValue>1</DefaultValue>
          <RangeInfo>
            <Min Inclusive="true">0</Min>
          </RangeInfo>
        </Double>
        <Group Name="MaterialRGB" Label="Reflectivity (bands 1 - 6)" Version="0" NumberOfRequiredGroups="1">
          <ItemDefinitions>
            <Double Name="Value1" Version="0" NumberOfRequiredValues="1">
              <Categories>
                <Cat>Spectral</Cat>
              </Categories>
              <DefaultValue>0</DefaultValue>
              <RangeInfo>
                <Min Inclusive="true">0</Min>
              </RangeInfo>
            </Double>
            <Double Name="Value2" Version="0" NumberOfRequiredValues="1">
              <Categories>
                <Cat>Spectral</Cat>
              </Categories>
              <DefaultValue>0</DefaultValue>
              <RangeInfo>
                <Min Inclusive="true">0</Min>
              </RangeInfo>
            </Double>
            <Double Name="Value3" Version="0" NumberOfRequiredValues="1">
              <Categories>
                <Cat>Spectral</Cat>
              </Categories>
              <DefaultValue>0</DefaultValue>
              <RangeInfo>
                <Min Inclusive="true">0</Min>
              </RangeInfo>
            </Double>
            <Double Name="Value4" Version="0" NumberOfRequiredValues="1">
              <Categories>
                <Cat>Spectral</Cat>
              </Categories>
              <DefaultValue>0</DefaultValue>
              <RangeInfo>
                <Min Inclusive="true">0</Min>
              </RangeInfo>
            </Double>
            <Double Name="Value5" Version="0" NumberOfRequiredValues="1">
              <Categories>
                <Cat>Spectral</Cat>
              </Categories>
              <DefaultValue>0</DefaultValue>
              <RangeInfo>
                <Min Inclusive="true">0</Min>
              </RangeInfo>
            </Double>
            <Double Name="Value6" Version="0" NumberOfRequiredValues="1">
              <Categories>
                <Cat>Spectral</Cat>
              </Categories>
              <DefaultValue>0</DefaultValue>
              <RangeInfo>
                <Min Inclusive="true">0</Min>
              </RangeInfo>
            </Double>
          </ItemDefinitions>
        </Group>
        <Group Name="MaterialTran" Label="Transmissivity (bands 1 - 6)" Version="0" NumberOfRequiredGroups="1">
          <ItemDefinitions>
            <Double Name="Value1" Version="0" NumberOfRequiredValues="1">
              <Categories>
                <Cat>Spectral</Cat>
              </Categories>
              <DefaultValue>0</DefaultValue>
              <RangeInfo>
                <Min Inclusive="true">0</Min>
              </RangeInfo>
            </Double>
            <Double Name="Value2" Version="0" NumberOfRequiredValues="1">
              <Categories>
                <Cat>Spectral</Cat>
              </Categories>
              <DefaultValue>0</DefaultValue>
              <RangeInfo>
                <Min Inclusive="true">0</Min>
              </RangeInfo>
            </Double>
            <Double Name="Value3" Version="0" NumberOfRequiredValues="1">
              <Categories>
                <Cat>Spectral</Cat>
              </Categories>
              <DefaultValue>0</DefaultValue>
              <RangeInfo>
                <Min Inclusive="true">0</Min>
              </RangeInfo>
            </Double>
            <Double Name="Value4" Version="0" NumberOfRequiredValues="1">
              <Categories>
                <Cat>Spectral</Cat>
              </Categories>
              <DefaultValue>0</DefaultValue>
              <RangeInfo>
                <Min Inclusive="true">0</Min>
              </RangeInfo>
            </Double>
            <Double Name="Value5" Version="0" NumberOfRequiredValues="1">
              <Categories>
                <Cat>Spectral</Cat>
              </Categories>
              <DefaultValue>0</DefaultValue>
              <RangeInfo>
                <Min Inclusive="true">0</Min>
              </RangeInfo>
            </Double>
            <Double Name="Value6" Version="0" NumberOfRequiredValues="1">
              <Categories>
                <Cat>Spectral</Cat>
              </Categories>
              <DefaultValue>0</DefaultValue>
              <RangeInfo>
                <Min Inclusive="true">0</Min>
              </RangeInfo>
            </Double>
          </ItemDefinitions>
        </Group>
        <Group Name="MaterialEmit" Label="Emissivity (bands 1 - 6)" Version="0" NumberOfRequiredGroups="1">
          <ItemDefinitions>
            <Double Name="Value1" Version="0" NumberOfRequiredValues="1">
              <Categories>
                <Cat>Spectral</Cat>
              </Categories>
              <DefaultValue>0</DefaultValue>
              <RangeInfo>
                <Min Inclusive="true">0</Min>
              </RangeInfo>
            </Double>
            <Double Name="Value2" Version="0" NumberOfRequiredValues="1">
              <Categories>
                <Cat>Spectral</Cat>
              </Categories>
              <DefaultValue>0</DefaultValue>
              <RangeInfo>
                <Min Inclusive="true">0</Min>
              </RangeInfo>
            </Double>
            <Double Name="Value3" Version="0" NumberOfRequiredValues="1">
              <Categories>
                <Cat>Spectral</Cat>
              </Categories>
              <DefaultValue>0</DefaultValue>
              <RangeInfo>
                <Min Inclusive="true">0</Min>
              </RangeInfo>
            </Double>
            <Double Name="Value4" Version="0" NumberOfRequiredValues="1">
              <Categories>
                <Cat>Spectral</Cat>
              </Categories>
              <DefaultValue>0</DefaultValue>
              <RangeInfo>
                <Min Inclusive="true">0</Min>
              </RangeInfo>
            </Double>
            <Double Name="Value5" Version="0" NumberOfRequiredValues="1">
              <Categories>
                <Cat>Spectral</Cat>
              </Categories>
              <DefaultValue>0</DefaultValue>
              <RangeInfo>
                <Min Inclusive="true">0</Min>
              </RangeInfo>
            </Double>
            <Double Name="Value6" Version="0" NumberOfRequiredValues="1">
              <Categories>
                <Cat>Spectral</Cat>
              </Categories>
              <DefaultValue>0</DefaultValue>
              <RangeInfo>
                <Min Inclusive="true">0</Min>
              </RangeInfo>
            </Double>
          </ItemDefinitions>
        </Group>
        <Double Name="MaterialLeafSize" Label="Leaf size" Version="0" NumberOfRequiredValues="1">
          <Categories>
            <Cat>Veg</Cat>
          </Categories>
          <DefaultValue>0.01</DefaultValue>
          <RangeInfo>
            <Min Inclusive="true">0</Min>
          </RangeInfo>
        </Double>
      </ItemDefinitions>
    </AttDef>

    <!--***  Veg Definitions ***-->
    <AttDef Type="Veg" Label="Veg" BaseType="" Version="0" Unique="true" Associations="">
      <ItemDefinitions>
        <Void Name="RayVegTwins" Label="Allow radiation to pass through&#10;  (for vegetation)" Version="0" Optional="true" IsEnabledByDefault="false" />
        <String Name="VegNodeFile" Label="Solution file basename:" Version="0" NumberOfRequiredValues="1">
          <DefaultValue>veg_Temp</DefaultValue>
        </String>
        <String Name="VegStartTime" Label="Start time (DDDHHMM):" Version="0" NumberOfRequiredValues="1" />
        <String Name="VegEndTime" Label="End time (DDDHHMM):" Version="0" NumberOfRequiredValues="1" />
        <Double Name="METWindHeight" Label="MET station anemometer height:" Version="0" NumberOfRequiredValues="1">
          <DefaultValue>3</DefaultValue>
        </Double>
        <String Name="VegOutputMesh" Label="Output mesh file basename:" Version="0" NumberOfRequiredValues="1">
          <DefaultValue>veg</DefaultValue>
        </String>
        <String Name="OutputEnsightMesh" Label="Ensight mesh file basename:" Version="0" Optional="true" IsEnabledByDefault="false" AdvanceLevel="1" NumberOfRequiredValues="1" />
        <String Name="OutputEnsightNodeFile" Label="Ensight solution file basename:" Version="0" Optional="true" IsEnabledByDefault="false" AdvanceLevel="1" NumberOfRequiredValues="1" />
      </ItemDefinitions>
    </AttDef>

    <!--***  BoundaryCondition Definitions ***-->
    <AttDef Type="BoundaryCondition" BaseType="" Abstract="1" Version="0" Unique="true" Associations="f" />
    <AttDef Type="SpecifiedHead" Label="Specified Head" BaseType="BoundaryCondition" Version="0" Unique="true" Nodal="true" Associations="f">
      <ItemDefinitions>
        <Double Name="Value" Label="Specified head" Version="0" NumberOfRequiredValues="1">
          <Categories>
            <Cat>Flow</Cat>
          </Categories>
          <ExpressionType>PolyLinearFunction</ExpressionType>
        </Double>
      </ItemDefinitions>
    </AttDef>
    <AttDef Type="SpecifiedFlux" Label="Specified Flux" BaseType="BoundaryCondition" Version="0" Unique="true" Associations="f">
      <ItemDefinitions>
        <Double Name="Value" Label="Specified flux" Version="0" NumberOfRequiredValues="1">
          <Categories>
            <Cat>Flow</Cat>
          </Categories>
          <ExpressionType>PolyLinearFunction</ExpressionType>
        </Double>
      </ItemDefinitions>
    </AttDef>

    <AttDef Type="FlowInjectionWell" Label="Flow Injection Well" BaseType="BoundaryCondition" Version="0" Unique="true" Nodal="true" Associations="f">
      <ItemDefinitions>
        <Double Name="Value" Label="Injection well for flow" Version="0" NumberOfRequiredValues="1">
          <Categories>
            <Cat>Flow</Cat>
          </Categories>
          <ExpressionType>PolyLinearFunction</ExpressionType>
        </Double>
      </ItemDefinitions>
    </AttDef>

    <AttDef Type="METData" Label="MET Data" BaseType="BoundaryCondition" Version="0" Unique="true" Associations="f">
      <ItemDefinitions>
        <Void Name="Value" Label="Use MET data to drive rainfall" Version="0" AdvanceLevel="1">
          <Categories>
            <Cat>Flow</Cat>
          </Categories>
        </Void>
      </ItemDefinitions>
    </AttDef>

    <AttDef Type="GroundSurfaceHeatFlux" Label="Ground Surface Heat Flux" BaseType="BoundaryCondition" Version="0" Unique="true" Associations="f">
      <ItemDefinitions>
        <Double Name="Value" Label="Heat flux on the ground surface" Version="0" NumberOfRequiredValues="1">
          <Categories>
            <Cat>Heat</Cat>
          </Categories>
          <ExpressionType>PolyLinearFunction</ExpressionType>
        </Double>
      </ItemDefinitions>
    </AttDef>

    <AttDef Type="UseMETData" Label="Use MET Data" BaseType="BoundaryCondition" Version="0" Unique="true" Associations="f">
      <ItemDefinitions>
        <Void Name="Value" Label="Use MET data to drive surface heat exchange" Version="0" AdvanceLevel="1">
          <Categories>
            <Cat>Heat</Cat>
          </Categories>
        </Void>
      </ItemDefinitions>
    </AttDef>

    <AttDef Type="RayCaster" Label="Ray Caster" BaseType="BoundaryCondition" Version="0" Unique="true" Associations="f">
      <ItemDefinitions>
        <Group Name="RayCaster" Label="Use an external ray caster for heat flux" Version="0" NumberOfRequiredGroups="1">
          <ItemDefinitions>
            <Int Name="Value1" Version="0" NumberOfRequiredValues="1">
              <Categories>
                <Cat>Heat</Cat>
              </Categories>
              <RangeInfo>
                <Min Inclusive="false">0</Min>
              </RangeInfo>
            </Int>
            <Int Name="Value2" Version="0" NumberOfRequiredValues="1">
              <RangeInfo>
                <Min Inclusive="false">0</Min>
              </RangeInfo>
            </Int>
          </ItemDefinitions>
        </Group>
      </ItemDefinitions>
    </AttDef>

    <AttDef Type="BottomBoundaryTemp" Label="Bottom Boundary Temp" BaseType="BoundaryCondition" Version="0" Unique="true" Nodal="true" Associations="f">
      <ItemDefinitions>
        <Double Name="Value" Label="Bottom boundary temperature" Version="0" NumberOfRequiredValues="1">
          <Categories>
            <Cat>Heat</Cat>
          </Categories>
          <ExpressionType>PolyLinearFunction</ExpressionType>
        </Double>
      </ItemDefinitions>
    </AttDef>

    <!--***  Ray Definitions ***-->
    <AttDef Type="Ray" Label="Ray" BaseType="" Version="0" Unique="true" Associations="">
      <ItemDefinitions>
        <String Name="RayQCFileName" Label="Pre-QC file basename:" Version="0" NumberOfRequiredValues="1">
          <DefaultValue>preqc</DefaultValue>
        </String>
        <Void Name="RaySoilFlip" Label="Flip ordering of soil elements' nodes" Version="0" Optional="true" IsEnabledByDefault="false" />
        <Void Name="RayVegFlip" Label="Flip ordering of vegetation elements' nodes" Version="0" Optional="true" IsEnabledByDefault="false" />
      </ItemDefinitions>
    </AttDef>

    <!--***  Time Definitions ***-->
    <AttDef Type="Time" Label="Time" BaseType="" Version="0" Unique="true" Associations="">
      <ItemDefinitions>
        <Int Name="JulianDay" Label="Julian start day:" Version="0" Optional="true" IsEnabledByDefault="false" AdvanceLevel="1" NumberOfRequiredValues="1">
          <Categories>
            <Cat>General</Cat>
          </Categories>
          <DefaultValue>0</DefaultValue>
          <RangeInfo>
            <Min Inclusive="true">0</Min>
          </RangeInfo>
        </Int>
        <Group Name="StartTime" Label="Start time:" Version="0" NumberOfRequiredGroups="1">
          <ItemDefinitions>
            <Double Name="Value1" Label="Value" Version="0" NumberOfRequiredValues="1">
              <Categories>
                <Cat>General</Cat>
              </Categories>
              <DefaultValue>0</DefaultValue>
              <RangeInfo>
                <Min Inclusive="true">0</Min>
              </RangeInfo>
            </Double>
            <Int Name="Value2" Label="Units" Version="0" NumberOfRequiredValues="1">
              <DiscreteInfo DefaultIndex="0">
                <Value Enum="Seconds">0</Value>
                <Value Enum="Minutes">1</Value>
                <Value Enum="Hours">2</Value>
                <Value Enum="Days">3</Value>
                <Value Enum="Weeks">4</Value>
                <Value Enum="Months">5</Value>
                <Value Enum="Years">6</Value>
              </DiscreteInfo>
            </Int>
          </ItemDefinitions>
        </Group>
        <Group Name="EndTime" Label="End time:" Version="0" NumberOfRequiredGroups="1">
          <ItemDefinitions>
            <Double Name="Value1" Label="Value" Version="0" NumberOfRequiredValues="1">
              <Categories>
                <Cat>General</Cat>
              </Categories>
              <DefaultValue>162</DefaultValue>
              <RangeInfo>
                <Min Inclusive="true">0</Min>
              </RangeInfo>
            </Double>
            <Int Name="Value2" Label="Units" Version="0" NumberOfRequiredValues="1">
              <DiscreteInfo DefaultIndex="0">
                <Value Enum="Seconds">0</Value>
                <Value Enum="Minutes">1</Value>
                <Value Enum="Hours">2</Value>
                <Value Enum="Days">3</Value>
                <Value Enum="Weeks">4</Value>
                <Value Enum="Months">5</Value>
                <Value Enum="Years">6</Value>
              </DiscreteInfo>
            </Int>
          </ItemDefinitions>
        </Group>
        <Double Name="TimestepSize" Label="Time step size:&#10;  Select a function that defines the desired&#10; solver time stepping." Version="0" NumberOfRequiredValues="1">
          <Categories>
            <Cat>General</Cat>
          </Categories>
          <ExpressionType>PolyLinearFunction</ExpressionType>
        </Double>
        <Void Name="AdaptiveTimeStep" Label="Use adaptive time stepping" Version="0" Optional="true" IsEnabledByDefault="false" AdvanceLevel="1">
          <Categories>
            <Cat>General</Cat>
          </Categories>
        </Void>
        <Int Name="OutputInterval" Label="Output interval (for writing solution data)">
          <ChildrenDefinitions>
            <Double Name="FixedInterval" Label="Value">
              <DefaultValue>1</DefaultValue>
              <RangeInfo>
                <Min Inclusive="false">0</Min>
              </RangeInfo>
            </Double>
            <Int Name="FixedIntervalUnits" Label="Units" Version="0" NumberOfRequiredValues="1">
              <DiscreteInfo DefaultIndex="0">
                <Value Enum="Seconds">0</Value>
                <Value Enum="Minutes">1</Value>
                <Value Enum="Hours">2</Value>
                <Value Enum="Days">3</Value>
                <Value Enum="Weeks">4</Value>
                <Value Enum="Months">5</Value>
                <Value Enum="Years">6</Value>
              </DiscreteInfo>
            </Int>
            <Double Name="IntervalFunction" Label="Function" Version="0" NumberOfRequiredValues="1">
              <ExpressionType>PolyLinearFunction</ExpressionType>
            </Double>
          </ChildrenDefinitions>
          <Categories>
            <Cat>General</Cat>
          </Categories>
          <DiscreteInfo DefaultValue="0">
            <Structure>
              <Value Enum="Specified Time Interval">0</Value>
              <Items>
                <Item>FixedInterval</Item>
                <Item>FixedIntervalUnits</Item>
              </Items>
            </Structure>
            <Structure>
              <Value Enum="Specified Function">1</Value>
              <Items>
                <Item>IntervalFunction</Item>
              </Items>
            </Structure>
          </DiscreteInfo>
        </Int>

        <Void Name="PrintAdaptedMeshes" Label="Print adapted meshes" Version="0" Optional="true" IsEnabledByDefault="false" AdvanceLevel="1">
          <Categories>
            <Cat>General</Cat>
          </Categories>
        </Void>
      </ItemDefinitions>
    </AttDef>

    <!--***  Globals Definitions ***-->
    <AttDef Type="Globals" Label="Globals" BaseType="" Version="0" Unique="true">
      <ItemDefinitions>
        <String Name="MetFileName" Label="MET file:" Version="0" AdvanceLevel="1" NumberOfRequiredValues="1">
          <Categories>
            <Cat>General</Cat>
          </Categories>
        </String>
        <Int Name="RayToADHSocket" Label="ADH input communication file number:&#10;  (fluxes from ray caster to ADH)" Version="0" AdvanceLevel="1" NumberOfRequiredValues="1">
          <Categories>
            <Cat>General</Cat>
          </Categories>
          <DefaultValue>200</DefaultValue>
        </Int>
        <Int Name="RayToVegSocket" Label="Veg. input communication file number:&#10;  (fluxes from ray caster to veg. model)" Version="0" AdvanceLevel="1" NumberOfRequiredValues="1">
          <Categories>
            <Cat>General</Cat>
          </Categories>
          <DefaultValue>400</DefaultValue>
        </Int>
        <Int Name="ADHToRaySocket" Label="ADH output communication file number:&#10;  (temperatures from ADH to ray caster)" Version="0" AdvanceLevel="1" NumberOfRequiredValues="1">
          <Categories>
            <Cat>General</Cat>
          </Categories>
          <DefaultValue>300</DefaultValue>
        </Int>
        <Int Name="VegToRaySocket" Label="Veg. output communication file number:&#10;  (temperatures from veg. model to ray caster)" Version="0" AdvanceLevel="1" NumberOfRequiredValues="1">
          <Categories>
            <Cat>General</Cat>
          </Categories>
          <DefaultValue>500</DefaultValue>
        </Int>
        <Double Name="Gravity" Label="Gravity:" Version="0" AdvanceLevel="1" NumberOfRequiredValues="1" Units="m/hr^2">
          <Categories>
            <Cat>General</Cat>
          </Categories>
          <DefaultValue>1.27202e+08</DefaultValue>
          <RangeInfo>
            <Min Inclusive="false">0</Min>
          </RangeInfo>
        </Double>
        <Double Name="WaterSpecificHeat" Label="Specific heat capacity of water:" Version="0" AdvanceLevel="1" NumberOfRequiredValues="1" Units="W hr/g-K">
          <Categories>
            <Cat>General</Cat>
          </Categories>
          <DefaultValue>0.00116</DefaultValue>
          <RangeInfo>
            <Min Inclusive="false">0</Min>
          </RangeInfo>
        </Double>
        <Double Name="AirSpecificHeat" Label="Specific heat capacity of air:" Version="0" AdvanceLevel="1" NumberOfRequiredValues="1" Units="W hr/g-K">
          <Categories>
            <Cat>General</Cat>
          </Categories>
          <DefaultValue>0.000278</DefaultValue>
          <RangeInfo>
            <Min Inclusive="false">0</Min>
          </RangeInfo>
        </Double>
        <Double Name="WaterSpecificGravity" Label="Ratio of water density to the reference density:" Version="0" AdvanceLevel="1" NumberOfRequiredValues="1">
          <Categories>
            <Cat>General</Cat>
          </Categories>
          <DefaultValue>1</DefaultValue>
          <RangeInfo>
            <Min Inclusive="false">0</Min>
          </RangeInfo>
        </Double>
        <Double Name="GasSpecificGravity" Label="Ratio of gas density to reference density:" Version="0" AdvanceLevel="1" NumberOfRequiredValues="1">
          <Categories>
            <Cat>General</Cat>
          </Categories>
          <DefaultValue>0.001</DefaultValue>
          <RangeInfo>
            <Min Inclusive="false">0</Min>
          </RangeInfo>
        </Double>
        <Double Name="WaterThermalConductivity" Label="Thermal conductivity of water:" Version="0" AdvanceLevel="1" NumberOfRequiredValues="1" Units="W/m-K">
          <Categories>
            <Cat>General</Cat>
          </Categories>
          <DefaultValue>0.58</DefaultValue>
          <RangeInfo>
            <Min Inclusive="false">0</Min>
          </RangeInfo>
        </Double>
        <Double Name="AirThermalConductivity" Label="Thermal conductivity of air:" Version="0" AdvanceLevel="1" NumberOfRequiredValues="1" Units="W/m-K">
          <Categories>
            <Cat>General</Cat>
          </Categories>
          <DefaultValue>0.024</DefaultValue>
          <RangeInfo>
            <Min Inclusive="false">0</Min>
          </RangeInfo>
        </Double>
        <Double Name="ReferenceDensity" Label="Reference density:" Version="0" AdvanceLevel="1" NumberOfRequiredValues="1" Units="g/m^3">
          <Categories>
            <Cat>General</Cat>
          </Categories>
          <DefaultValue>1e+06</DefaultValue>
          <RangeInfo>
            <Min Inclusive="false">0</Min>
          </RangeInfo>
        </Double>
        <Double Name="ReferenceViscosity" Label="Reference viscosity:" Version="0" Optional="true" IsEnabledByDefault="true" AdvanceLevel="1" NumberOfRequiredValues="1" Units="kg/m-s">
          <Categories>
            <Cat>General</Cat>
          </Categories>
          <DefaultValue>1e-05</DefaultValue>
          <RangeInfo>
            <Min Inclusive="false">0</Min>
          </RangeInfo>
        </Double>
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
  </Attributes>

  <!--********** Workflow Views ***********-->
  <RootView Title="SimBuilder">
    <DefaultColor>1., 1., 0.5, 1.</DefaultColor>
    <InvalidColor>1, 0.5, 0.5, 1</InvalidColor>

    <SimpleExpressionView Title="Functions">
      <Definition>PolyLinearFunction</Definition>
    </SimpleExpressionView>

    <AttributeView Title="Materials" ModelEntityFilter="r" CreateEntities="true">
      <AttributeTypes>
        <Type>Material</Type>
      </AttributeTypes>
    </AttributeView>

    <InstancedView Title="Veg">
      <InstancedAttributes>
        <Att Type="Veg">Veg</Att>
      </InstancedAttributes>
    </InstancedView>

    <AttributeView Title="BoundaryConditions" ModelEntityFilter="f">
      <AttributeTypes>
        <Type>SpecifiedHead</Type>
        <Type>SpecifiedFlux</Type>
        <Type>FlowInjectionWell</Type>
        <Type>METData</Type>
        <Type>GroundSurfaceHeatFlux</Type>
        <Type>UseMETData</Type>
        <Type>RayCaster</Type>
        <Type>BottomBoundaryTemp</Type>
      </AttributeTypes>
    </AttributeView>

    <InstancedView Title="Ray">
      <InstancedAttributes>
        <Att Type="Ray">Ray</Att>
      </InstancedAttributes>
    </InstancedView>

    <InstancedView Title="Time">
      <InstancedAttributes>
        <Att Type="Time">Time</Att>
      </InstancedAttributes>
    </InstancedView>

    <InstancedView Title="Globals">
      <InstancedAttributes>
        <Att Type="Globals">Globals</Att>
      </InstancedAttributes>
    </InstancedView>

    <InstancedView Title="Solvers">
      <InstancedAttributes>
        <Att Type="Solvers">Solvers</Att>
      </InstancedAttributes>
    </InstancedView>

  </RootView>
</SMTK_AttributeManager>
