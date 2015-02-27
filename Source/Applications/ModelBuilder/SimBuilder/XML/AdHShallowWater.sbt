<?xml version="1.0"?>
<SMTK_AttributeManager Version="1">
  <Includes>
    <!-- Uses same definitions as surface water -->
    <File>AdHSurfaceWaterDefinitions.sbt</File>
  </Includes>

  <Categories>
    <Cat>Meshing</Cat>
  </Categories>

  <Definitions>
    <AttDef Type="Extrusion" Label="Extrusion" Unique="true" Associations="f">
      <ItemDefinitions>
        <Int Name="NumberOfLayers">
          <Categories>Meshing</Categories>
          <DefaultValue>1</DefaultValue>
          <RangeInfo>
            <Min Exclusive="false">1</Min>
          </RangeInfo>
        </Int>
      </ItemDefinitions>
    </AttDef>
  </Definitions>

  <!--********** Workflow Views ***********-->
  <!-- Same as surface water PLUS meshing view -->
  <RootView Title="SimBuilder">
    <DefaultColor>1., 1., 0.5, 1.</DefaultColor>
    <InvalidColor>1, 0.5, 0.5, 1</InvalidColor>

    <InstancedView Title="Solvers">
      <InstancedAttributes>
        <Att Type="Solvers">Solvers</Att>
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

    <SimpleExpressionView Title="Functions">
      <Definition>PolyLinearFunction</Definition>
    </SimpleExpressionView>

    <AttributeView Title="Materials" ModelEntityFilter="f" CreateEntities="true">
      <AttributeTypes>
        <Type>SolidMaterial</Type>
      </AttributeTypes>
    </AttributeView>

    <AttributeView Title="Constituents" CreateEntities="true">
      <AttributeTypes>
        <Type>Constituent</Type>
      </AttributeTypes>
    </AttributeView>

    <AttributeView Title="Friction" ModelEntityFilter="f" CreateEntities="true">
      <AttributeTypes>
        <Type>Friction</Type>
      </AttributeTypes>
    </AttributeView>

    <AttributeView Title="BoundaryConditions" ModelEntityFilter="e">
      <AttributeTypes>
        <Type>VelocityBound</Type>
        <Type>LidElevation</Type>
        <Type>VelocityAndDepthBound</Type>
        <Type>WaterDepthLid</Type>
        <Type>WaterSurfElev</Type>
        <Type>UnitFlow</Type>
        <Type>TotalDischarge</Type>
        <Type>FloatingStationary</Type>
        <Type>DirichletTransport</Type>
        <Type>NaturalTransport</Type>
      </AttributeTypes>
    </AttributeView>

    <AttributeView Title="Extrusions" ModelEntityFilter="f">
      <AttributeTypes>
        <Type>Extrusion</Type>
      </AttributeTypes>
    </AttributeView>

  </RootView>


</SMTK_AttributeManager>
