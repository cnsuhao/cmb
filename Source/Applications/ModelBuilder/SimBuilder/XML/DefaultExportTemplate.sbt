<?xml version="1.0"?>
<!-- Default export-dialog template for CMB -->
<SMTK_AttributeManager Version="1">
  <Definitions>
    <AttDef Type="ExportSpec" BaseType="" Version="0" Unique="true">
      <ItemDefinitions>
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
  <RootView Title="Export">
    <DefaultColor>1., 1., 0.5, 1.</DefaultColor>
    <InvalidColor>1, 0.5, 0.5, 1</InvalidColor>
    <AdvancedFontEffects Bold="0" Italic="0" />

    <InstancedView Title="ExportSpec">
      <InstancedAttributes>
        <Att Type="ExportSpec">Options</Att>
      </InstancedAttributes>
    </InstancedView>
  </RootView>
</SMTK_AttributeManager>
