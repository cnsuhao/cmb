<?xml version="1.0"?>
<!-- Default export-dialog template for CMB -->
<SMTK_AttributeSystem Version="2">
  <Definitions>
    <AttDef Type="ExportSpec" BaseType="" Version="1" Unique="true">
      <AssociationsDef Name="model" Label="Model" Version="0"
        NumberOfRequiredValues="1">
        <MembershipMask>model</MembershipMask>
      </AssociationsDef>
      <ItemDefinitions>
        <MeshEntity Name="mesh" Label="Mesh" Version="0" NumberOfRequiredValues="1" />
        <String Name="AnalysisTypes" Label="Analysis Types" AdvanceLevel="99" Version="0"
                Extensible="true" NumberOfRequiredValues="0" />
        <File Name="PythonScript" Label="Python script" Version="0"  NumberOfRequiredValues="1"
              ShouldExist="true" FileFilters="Python files (*.py);;All files (*.*)" />
        <File Name="OutputFile" Label="Output file" Version="0" NumberOfRequiredValues="1"
              FileFilters="All files (*.*)" />
      </ItemDefinitions>
    </AttDef>
  </Definitions>
  <Attributes />

  <Views>
    <View Type="Instanced" Title="ExportSpec" TopLevel="true"
      FilterByAdvanceLevel="false" FilterByCategory="false">
      <InstancedAttributes>
        <Att Name="Export Specification" Type="ExportSpec">Options</Att>
      </InstancedAttributes>
    </View>
  </Views>
</SMTK_AttributeSystem>
