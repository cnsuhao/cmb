<?xml version="1.0"?>
<SMTK_AttributeManager Version="1">
  <!--**********  Category and Analysis Infomation ***********-->
  <Categories Default="strategy"/>
  <Analyses>
    <Analysis Type="Dakota" Default="strategy"/>
  </Analyses>
  <!--**********  Attribute Definitions ***********-->
  <Definitions>
    <!--************* Strategy Attributes. This will probably need to be expanded. **************-->
    <AttDef Type="strategy" BaseType="" Abstract="0" Version="0" Unique="false" Associations="">
      <ItemDefinitions>
        <String Name="hybrid_type" Label="Hybrid Type" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
          <DiscreteInfo DefaultIndex="0">
            <Value Enum="sequential">sequential</Value>
          </DiscreteInfo>
        </String>
        <String Name="method_list" Label="Method List" NumberOfRequiredValues="1"/>
      </ItemDefinitions>
    </AttDef>

    <!--************* Method Attributes. ************-->
    <AttDef Type="method" BaseType="" Abstract="1" Version="0" Unique="false" Associations="">
      <ItemDefinitions>
        <String Name="id_method" Label="Method Name" Version="0" NumberOfRequiredValues="1"/>
        <Int Name="max_iterations" Label="Max Iterations" Optional="true" IsEnabledByDefault="false" Version="0" NumberOfRequiredValues="1"/>
        <Int Name="max_function_evaluations" Label="Max Function Evaluations" Optional="true" IsEnabledByDefault="false" Version="0" NumberOfRequiredValues="1"/>
      </ItemDefinitions>
    </AttDef>
    <AttDef Type="efficient_global" Label="Efficient Global" BaseType="method" Abstract="0" Version="0" Unique="false" Associations="">
      <ItemDefinitions>
      </ItemDefinitions>
    </AttDef>
    <AttDef Type="coliny_pattern_search" Label="Coliny Pattern Search" BaseType="method" Abstract="0" Version="0" Unique="false" Associations="">
      <ItemDefinitions>
        <Int Name="seed" Label="Seed" Version="0" NumberOfRequiredValues="1"/>
        <Double Name="initial_delta" Label="Initial Delta" Version="0" NumberOfRequiredValues="1"/>
        <Double Name="threshold_delta" Label="Threshold Delta" Version="0" NumberOfRequiredValues="1"/>
        <Double Name="solution_accuracy" Label="Solution Accuracy" Version="0" NumberOfRequiredValues="1"/>
        <String Name="exploratory_moves" Label="Exploratory Moves" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
          <DiscreteInfo DefaultIndex="0">
            <Value Enum="basic_pattern">basic_pattern</Value>
          </DiscreteInfo>
        </String>
      </ItemDefinitions>
    </AttDef>
    <AttDef Type="optpp_newton" Label= "OptPP Newton" BaseType="method" Abstract="0" Version="0" Unique="false" Associations="">
      <ItemDefinitions>
        <Double Name="gradient_tolerance" Label="Gradient Tolerance" Optional="true" IsEnabledByDefault="false" Version="0" NumberOfRequiredValues="1"/>
        <Double Name="convergence_tolerance" Label="Convergence Tolerance" Optional="true" IsEnabledByDefault="false" Version="0" NumberOfRequiredValues="1"/>
      </ItemDefinitions>
    </AttDef>
    <AttDef Type="sampling" Label="Nondeterministic Sampling Method" BaseType="method" Abstract="0" Version="0" Unique="false" Associations="">
      <ItemDefinitions>
        <String Name="sample_type" Label="Sampling Type" Optional="true" IsEnabledByDefault="false"  Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
          <DiscreteInfo DefaultIndex="0">
            <Value Enum="random">Random</Value>
            <Value Enum="lhs">Latin Hypercube</Value>
            <Value Enum="incremental_random">Incremental Random</Value>
            <Value Enum="incremental_lhs">Incremental Latin Hypercube</Value>
          </DiscreteInfo>
        </String>
        <Int Name="samples" Label="Number Of Samples" Optional="true" IsEnabledByDefault="false"  Version="0" AdvanceLevel="0" NumberOfRequiredValues="1"/>
        <Int Name="previous_samples" Label="previous samples" Optional="true" IsEnabledByDefault="false"  Version="0" AdvanceLevel="0" NumberOfRequiredValues="1"/>
        <String Name="rng" Label="Random Number Generator" Optional="true" IsEnabledByDefault="false"  Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
          <DiscreteInfo DefaultIndex="0">
            <Value Enum="mt19937">Mersenne twister</Value>
            <Value Enum="rnum2">Latin Hypercube</Value>
            <Value Enum="incremental_random">Incremental Random</Value>
            <Value Enum="incremental_lhs">Incremental Latin Hypercube</Value>
          </DiscreteInfo>
        </String>
        <Void Name="variance_based_decomp" Label="Variance Based Decomposition (VBD)" Optional="true" IsEnabledByDefault="false"  Version="0" AdvanceLevel="0" NumberOfRequiredValues="1"/>
        <Void Name="drop_tolerance" Label="VBD Tolerance fo omitting small indices" Optional="true" IsEnabledByDefault="false"  Version="0" AdvanceLevel="0" NumberOfRequiredValues="1"/>
      </ItemDefinitions>
    </AttDef>
    <AttDef Type="nl2sol" Label= "nl2sol" BaseType="method" Abstract="0" Version="0" Unique="false" Associations="">
      <ItemDefinitions>
        <Double Name="convergence_tolerance" Label="Convergence Tolerance" Optional="true" IsEnabledByDefault="false" Version="0" NumberOfRequiredValues="1"/>
      </ItemDefinitions>
    </AttDef>

    <!--*************** Variable Attributes. ****************-->
    <AttDef Type="variable" BaseType="" Abstract="1" Version="0" Unique="false" Associations="">
      <ItemDefinitions>
        <String Name="name" Label="Variable Name" Version="0" NumberOfRequiredValues="1"/>
      </ItemDefinitions>
    </AttDef>
    <AttDef Type="continuous_variable" Label="Continuous Variable" BaseType="variable" Abstract="0" Version="0" Unique="false" Associations="">
      <ItemDefinitions>
        <Double Name="min" Label="Min Value" Version="0" Optional="true" IsEnabledByDefault="false" NumberOfRequiredValues="1"/>
        <Double Name="max" Label="Max Value" Version="0" Optional="true" IsEnabledByDefault="false" NumberOfRequiredValues="1"/>
        <Double Name="init" Label="Initial Value" Version="0" Optional="true" IsEnabledByDefault="false" NumberOfRequiredValues="1"/>
      </ItemDefinitions>
    </AttDef>

    <!--************* Interface Attributes *************-->
    <AttDef Type="interface" Label="Basic Interface" BaseType="" Abstract="1" Version="0" Unique="false" Associations="">
      <ItemDefinitions>
        <String Name="analysis_drivers" Label="Analysis Drivers" Version="0" NumberOfRequiredValues="1"/>
      </ItemDefinitions>
    </AttDef>
    <AttDef Type="direct_interface" Label="Direct Interface" BaseType="interface" Abstract="0" Version="0" Unique="false" Associations="">
    </AttDef>
    <AttDef Type="fork_interface" Label="Fork" BaseType="interface" Abstract="0" Version="0" Unique="false" Associations="">
      <ItemDefinitions>
        <String Name="parameters_file" Label="Parameters File" Optional="true" IsEnabledByDefault="false" Version="0" NumberOfRequiredValues="1"/>
        <String Name="results_file" Label="Results File" Optional="true" IsEnabledByDefault="false" Version="0" NumberOfRequiredValues="1"/>
      </ItemDefinitions>
    </AttDef>
    <AttDef Type="python_interface" Label="Python" BaseType="interface" Abstract="0" Version="0" Unique="false" Associations=""/>

    <!--**************** Responces attributes.  Will probably need to be expanded *******************-->
    <AttDef Type="responses" BaseType="" Abstract="0" Version="0" Unique="false" Associations="">
      <ItemDefinitions>
        <Int Name="calibration_terms" Optional="true" Label="Number of Calibration Terms" Version="0" NumberOfRequiredValues="1"/>
        <String Name="calibration_data_file" Optional="true" Label="Calibration Data File" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1"/>
        <String Name="form" Label="Calibration File Form" Optional="true" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
          <DiscreteInfo DefaultIndex="0">
            <Value Enum="freeform">freeform</Value>
          </DiscreteInfo>
        </String>
        <Int Name="num_objective_functions" Optional="true" Label="Number of Objective Functions" Version="0" NumberOfRequiredValues="1"/>
        <String Name="gradiants" Label="Gradiants" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
          <DiscreteInfo DefaultIndex="0">
            <Value Enum="none">None</Value>
            <Value Enum="analytic">Analytic</Value>
            <Value Enum="numerical">Numerical</Value>
          </DiscreteInfo>
        </String>
        <String Name="hessians" Label="Hessians" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
          <DiscreteInfo DefaultIndex="0">
            <Value Enum="none">None</Value>
            <Value Enum="analytic">Analytic</Value>
            <Value Enum="numerical">Numerical</Value>
          </DiscreteInfo>
        </String>
      </ItemDefinitions>
    </AttDef>
  </Definitions>

  <!--**********  Attribute Instances ***********-->
  <Attributes>
  </Attributes>

  <!--********** Workflow Views ***********-->
  <RootView Title="SimBuilder">
    <InstancedView Title="Strategy">
      <InstancedAttributes>
        <Att Type="strategy">strategy</Att>
      </InstancedAttributes>
    </InstancedView>
    <AttributeView Title="Method" ModelEntityFilter="r">
      <AttributeTypes>
        <Type>method</Type>
      </AttributeTypes>
    </AttributeView>
    <AttributeView Title="Variables" ModelEntityFilter="r">
      <AttributeTypes>
        <Type>variable</Type>
      </AttributeTypes>
    </AttributeView>
    <AttributeView Title="Interface" ModelEntityFilter="r">
      <AttributeTypes>
        <Type>interface</Type>
      </AttributeTypes>
    </AttributeView>
    <InstancedView Title="Responses">
      <InstancedAttributes>
        <Att Type="responses">Responses</Att>
      </InstancedAttributes>
    </InstancedView>
  </RootView>
</SMTK_AttributeManager>
