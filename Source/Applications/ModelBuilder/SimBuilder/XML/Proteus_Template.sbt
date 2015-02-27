<?xml version="1.0"?>
<!-- information from default_n.py and default_p.py git revision 93035bb1fe715784416ee5f4ee01267016d141a1 -->
<!-- attributes/items that are a per component input:
some tolerances
initial conditions
boundary conditions (not the stress flux though)
conservativeFlux
analytical solution (although not all components may have an analytical solution applied to them)
FEM space
Element Quadrature Method
Element Boundary Quadrature Method
LevelModelType

the actual number of components comes from Transport Coefficients (a singleton) but even with each
of those classes, the amount of components may vary.
-->
<SMTK_AttributeManager Version="1">
  <!--**********  Category and Analysis Infomation ***********-->
  <Categories Default="General">
    <Cat>General</Cat>
    <Cat>Incompressible Navier-Stokes</Cat>
  </Categories>
  <Analyses>
    <Analysis Type="Incompressible Navier-Stokes Analysis">
      <Cat>General</Cat>
      <Cat>Incompressible Navier-Stokes</Cat>
    </Analysis>
  </Analyses>
  <!--**********  Attribute Definitions ***********-->
  <Definitions>
    <!--***  Numerics  ***-->
    <!-- numerics are in poisson_3d_tetgen_c0p1_n.py or poisson_3d_tetgen_c0p2_n.py, default is in default_p.py -->
    <AttDef Type="time" BaseType="" Version="0" Unique="true">
      <ItemDefinitions>
        <Group Name="time" Label="Time" NumberOfRequiredGroups="1">
          <ItemDefinitions>
            <String Name="stepController" Label="Step Controller" Version="0" AdvanceLevel="1" NumberOfRequiredValues="1">
              <BriefDescription>StepControl.py(numerics::stepController)</BriefDescription>
              <DiscreteInfo DefaultIndex="5">
                <Value Enum="Newton controller">Newton_controller</Value>
                <Value Enum="PsiTCtte controller">PsiTCtte_controller</Value>
                <Value Enum="Osher controller">Osher_controller</Value>
                <Value Enum="Osher PsiTC controller">Osher_PsiTC_controller</Value>
                <Value Enum="Osher PsiTC controller2">Osher_PsiTC_controller2</Value>
                <Value Enum="Min dt controller">Min_dt_controller</Value>
                <Value Enum="Min dt RKcontroller">Min_dt_RKcontroller</Value>
                <Value Enum="FLCBDF controller">FLCBDF_controller</Value>
                <Value Enum="HeuristicNL dt controller">HeuristicNL_dt_controller</Value>
                <Value Enum="GustafssonFullNewton dt controller">GustafssonFullNewton_dt_controller</Value>
              </DiscreteInfo>
              <Categories>
                <Cat>General</Cat>
              </Categories>
            </String>
            <String Name="timeIntegration" Label="Time Integration" Version="0" AdvanceLevel="1" NumberOfRequiredValues="1">
              <BriefDescription>TimeIntegration.py(numerics::timeIntegration)</BriefDescription>
              <DiscreteInfo DefaultIndex="2">
                <Value Enum="Steady State">NoIntegration</Value>
                <Value Enum="Backward Euler">BackwardEuler</Value>
                <Value Enum="Backward Euler CFL">BackwardEuler_cfl</Value>
                <Value Enum="FLCBDF">BackwardEuler_cfl</Value>
                <Value Enum="PsiTCtte">PsiTCtte</Value>
                <Value Enum="PsiTCtte_new">PsiTCtte_new</Value>
                <Value Enum="Forward Euler">ForwardEuler</Value>
                <Value Enum="Forward Euler A">ForwardEuler_A</Value>
                <Value Enum="Forward Euler H">ForwardEuler_H</Value>
                <Value Enum="Outer Theta">OuterTheta</Value>
                <Value Enum="VBDF">VBDF</Value>
                <Value Enum="ExplicitRK Base">ExplicitRK_base</Value>
                <Value Enum="Linear SSPRK Integration">LinearSSPRKintegration</Value>
                <Value Enum="SSPRKPI Integration">SSPRKPIintegration</Value>
                <Value Enum="Linear SSPRKPI Integration">LinearSSPRKPIintegration</Value>
              </DiscreteInfo>
              <Categories>
                <Cat>General</Cat>
              </Categories>
            </String>
            <Double Name="runCFL" Label="Maximum CFL for the Time Step" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
              <BriefDescription>(numerics::runCFL)</BriefDescription>
              <DefaultValue>0.9</DefaultValue>
              <RangeInfo>
                <Min Inclusive="false">0.0</Min>
              </RangeInfo>
              <Categories>
                <Cat>General</Cat>
              </Categories>
            </Double>
            <Int Name="numstages" Label="Number of Stages for the Time Discretization" Version="0" AdvanceLevel="1" NumberOfRequiredValues="1">
              <BriefDescription>(numerics::nStagesTime)</BriefDescription>
              <DefaultValue>1</DefaultValue>
              <Categories>
                <Cat>General</Cat>
              </Categories>
            </Int>
            <Int Name="timeOrder" Label="Order of Time Integration" Version="0" AdvanceLevel="1" NumberOfRequiredValues="1">
              <BriefDescription>(numerics::timeOrder)</BriefDescription>
              <DefaultValue>1</DefaultValue>
              <Categories>
                <Cat>General</Cat>
              </Categories>
            </Int>
            <Double Name="timestep" Label="Time Step Length" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
              <BriefDescription>(numerics::DT)</BriefDescription>
              <DefaultValue>1.0</DefaultValue>
              <Categories>
                <Cat>General</Cat>
              </Categories>
            </Double>
            <Int Name="numtimesteps" Label="Number Of Output Time Steps" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
              <BriefDescription>(numerics::nDTout)</BriefDescription>
              <DefaultValue>1</DefaultValue>
              <Categories>
                <Cat>General</Cat>
              </Categories>
            </Int>
          </ItemDefinitions>
        </Group>
      </ItemDefinitions>
    </AttDef>

    <!--
        <AttDef Type="tolerances" BaseType="" Version="0" Unique="true">
        <ItemDefinitions>
        <Group Name="Tolerances" Label="Tolerances" NumberOfRequiredGroups="1">
        <ItemDefinitions>
        <String Name="rtol_u" Label="rtol_u" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
        <BriefDescription>(numerics::rtol_u)</BriefDescription>
        <DefaultValue>{0:1.0e-4}</DefaultValue>
        </String>
        <String Name="atol_u" Label="atol_u" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
        <BriefDescription>(numerics::atol_u)</BriefDescription>
        <DefaultValue>{0:1.0e-4}</DefaultValue>
        </String>
        <Double Name="nltol_u" Label="nltol_u" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
        <BriefDescription>(numerics::nltol_u)</BriefDescription>
        <DefaultValue>0.33</DefaultValue>
        </Double>
        <Double Name="ltol_u" Label="ltol_u" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
        <BriefDescription>(numerics::ltol_u)</BriefDescription>
        <DefaultValue>0.05</DefaultValue>
        </Double>
        <String Name="rtol_res" Label="rtol_res" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
        <BriefDescription>(numerics::rtol_res)</BriefDescription>
        <DefaultValue>{0:1.0e-4}</DefaultValue>
        </String>
        <String Name="atol_res" Label="atol_res" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
        <BriefDescription>(numerics::atol_res)</BriefDescription>
        <DefaultValue>{0:1.0e-4}</DefaultValue>
        </String>
        <Double Name="nl_atol_res" Label="nl_atol_res" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
        <BriefDescription>(numerics::nl_atol_res)</BriefDescription>
        <DefaultValue>1.0</DefaultValue>
        </Double>
        <Double Name="l_atol_res" Label="l_atol_res" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
        <BriefDescription>(numerics::l_atol_res)</BriefDescription>
        <DefaultValue>1.0</DefaultValue>
        </Double>
        </ItemDefinitions>
        </Group>
        </ItemDefinitions>
        </AttDef>
    -->




    <AttDef Type="tolerancesalternate" BaseType="" Version="0" Unique="true">
      <ItemDefinitions>
        <Double Name="nltol_u" Label="nltol_u" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
          <BriefDescription>(numerics::nltol_u)</BriefDescription>
          <DefaultValue>0.33</DefaultValue>
        </Double>
        <Double Name="ltol_u" Label="ltol_u" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
          <BriefDescription>(numerics::ltol_u)</BriefDescription>
          <DefaultValue>0.05</DefaultValue>
        </Double>
        <Double Name="nl_atol_res" Label="nl_atol_res" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
          <BriefDescription>(numerics::nl_atol_res)</BriefDescription>
          <DefaultValue>1.0</DefaultValue>
        </Double>
        <Double Name="l_atol_res" Label="l_atol_res" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
          <BriefDescription>(numerics::l_atol_res)</BriefDescription>
          <DefaultValue>1.0</DefaultValue>
        </Double>
      </ItemDefinitions>
    </AttDef>
    <AttDef Type="basetolerance" BaseType="" Abstract="1" Version="0" Unique="false"/>
    <AttDef Type="rtol_u" Label="rtol_u" BaseType="basetolerance" Version="0" Unique="false">
      <ItemDefinitions>
        <Int Name="component" Label="component" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
          <BriefDescription>(numerics::rtol_u)</BriefDescription>
        </Int>
        <Double Name="tolerance" Label="tolerance" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
          <BriefDescription>(numerics::rtol_u)</BriefDescription>
          <DefaultValue>1.0e-4</DefaultValue>
        </Double>
      </ItemDefinitions>
    </AttDef>
    <AttDef Type="atol_u" Label="atol_u" BaseType="basetolerance" Version="0" Unique="false">
      <ItemDefinitions>
        <Int Name="component" Label="component" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
          <BriefDescription>(numerics::atol_u)</BriefDescription>
        </Int>
        <Double Name="tolerance" Label="tolerance" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
          <BriefDescription>(numerics::atol_u)</BriefDescription>
          <DefaultValue>1.0e-4</DefaultValue>
        </Double>
      </ItemDefinitions>
    </AttDef>









    <AttDef Type="femspaces" BaseType="" Version="0" Unique="true">
      <ItemDefinitions>
        <String Name="femSpaces" Label="FEM Space" Version="0" AdvanceLevel="1" NumberOfRequiredValues="1">
          <BriefDescription>FemTools.py(numerics::femSpaces)</BriefDescription>
          <DiscreteInfo DefaultIndex="0">
            <Value Enum="C0_AffineLinearOnSimplexWithNodalBasis">C0_AffineLinearOnSimplexWithNodalBasis</Value>
            <Value Enum="C0_AffineLinearOnCubeWithNodalBasis">C0_AffineLinearOnCubeWithNodalBasis</Value>
            <Value Enum="C0_AffineLagrangeOnCubeWithNodalBasis">C0_AffineLagrangeOnCubeWithNodalBasis</Value>
            <Value Enum="DG_AffinePolynomialsOnSimplexWithMonomialBasis">DG_AffinePolynomialsOnSimplexWithMonomialBasis</Value>
            <Value Enum="DG_AffineLinearOnSimplexWithNodalBasis">DG_AffineLinearOnSimplexWithNodalBasis</Value>
            <Value Enum="C0_AffineQuadraticOnSimplexWithNodalBasis">C0_AffineQuadraticOnSimplexWithNodalBasis</Value>
            <Value Enum="DG_AffineQuadraticOnSimplexWithNodalBasis">DG_AffineQuadraticOnSimplexWithNodalBasis</Value>
            <Value Enum="NC_AffineLinearOnSimplexWithNodalBasis">NC_AffineLinearOnSimplexWithNodalBasis</Value>
            <Value Enum="DG_Constants">DG_Constants</Value>
            <Value Enum="C0_AffineP1BubbleOnSimplexWithNodalBasis">C0_AffineP1BubbleOnSimplexWithNodalBasis</Value>
            <Value Enum="C0_AffineP1P0BubbleOnSimplexWithNodalBasis">C0_AffineP1P0BubbleOnSimplexWithNodalBasis</Value>
          </DiscreteInfo>
          <Categories>
            <Cat>General</Cat>
          </Categories>
        </String>
      </ItemDefinitions>
    </AttDef>

    <AttDef Type="quadrature" BaseType="" Version="0" Unique="true">
      <ItemDefinitions>
        <String Name="QuadratureMethod" Label="Element Quadrature Method" Version="0" AdvanceLevel="1" NumberOfRequiredValues="1">
          <BriefDescription>Quadrature.py(numerics::elementQuadrature)</BriefDescription>
          <DiscreteInfo DefaultIndex="10">
            <Value Enum="Gauss Point">GaussPoint</Value><!-- also LobattoPoint -->
            <Value Enum="Gauss Edge">GaussEdge</Value>
            <Value Enum="Lobatto Edge">LobattoEdge</Value>
            <Value Enum="Lobatto Edge Alt">LobattoEdgeAlt</Value>
            <Value Enum="Composite Trapezoidal Edge">CompositeTrapezoidalEdge</Value>
            <Value Enum="Face Barycenter Edge">FaceBarycenterEdge</Value>
            <Value Enum="Gauss Triangle">GaussTriangle</Value>
            <Value Enum="Lobatto Triangle">LobattoTriangle</Value>
            <Value Enum="Composite Trapezoidal Triangle">CompositeTrapezoidalTriangle</Value>
            <Value Enum="Face Barycenter Triangle">FaceBarycenterTriangle</Value>
            <Value Enum="Gauss Tetrahedron">GaussTetrahedron</Value>
            <Value Enum="Lobatto Tetrahedron">LobattoTetrahedron</Value>
            <Value Enum="Face Barycenter Tetrahedron">FaceBarycenterTetrahedron</Value>
            <Value Enum="Simplex Gauss Quadrature">SimplexGaussQuadrature</Value>
            <Value Enum="Cube Gauss Quadrature">CubeGaussQuadrature</Value>
            <Value Enum="Simplex Lobatto Quadrature">SimplexLobattoQuadrature</Value>
          </DiscreteInfo>
          <Categories>
            <Cat>General</Cat>
          </Categories>
        </String>
        <String Name="BoundaryQuadratureMethod" Label="Element Boundary Quadrature Method" Version="0" AdvanceLevel="1" NumberOfRequiredValues="1">
          <BriefDescription>Quadrature.py(numerics::elementBoundaryQuadrature method)</BriefDescription>
          <DiscreteInfo DefaultIndex="6">
            <Value Enum="Gauss Point">GaussPoint</Value><!-- also LobattoPoint -->
            <Value Enum="Gauss Edge">GaussEdge</Value>
            <Value Enum="Lobatto Edge">LobattoEdge</Value>
            <Value Enum="Lobatto Edge Alt">LobattoEdgeAlt</Value>
            <Value Enum="Composite Trapezoidal Edge">CompositeTrapezoidalEdge</Value>
            <Value Enum="Face Barycenter Edge">FaceBarycenterEdge</Value>
            <Value Enum="Gauss Triangle">GaussTriangle</Value>
            <Value Enum="Lobatto Triangle">LobattoTriangle</Value>
            <Value Enum="Composite Trapezoidal Triangle">CompositeTrapezoidalTriangle</Value>
            <Value Enum="Face Barycenter Triangle">FaceBarycenterTriangle</Value>
            <Value Enum="Gauss Tetrahedron">GaussTetrahedron</Value>
            <Value Enum="Lobatto Tetrahedron">LobattoTetrahedron</Value>
            <Value Enum="Face Barycenter Tetrahedron">FaceBarycenterTetrahedron</Value>
            <Value Enum="Simplex Gauss Quadrature">SimplexGaussQuadrature</Value>
            <Value Enum="Cube Gauss Quadrature">CubeGaussQuadrature</Value>
            <Value Enum="Simplex Lobatto Quadrature">SimplexLobattoQuadrature</Value>
          </DiscreteInfo>
          <Categories>
            <Cat>General</Cat>
          </Categories>
        </String>

      </ItemDefinitions>
    </AttDef>

    <AttDef Type="multilevelmeshlevels" BaseType="" Version="0" Unique="true">
      <ItemDefinitions>
        <Int Name="nLevels" Label="Number of Levels for Multilevel Mesh" Version="0" AdvanceLevel="1" NumberOfRequiredValues="1">
          <BriefDescription>(numerics::nLevels)</BriefDescription>
          <DefaultValue>1</DefaultValue>
          <Categories>
            <Cat>General</Cat>
          </Categories>
        </Int>
      </ItemDefinitions>
    </AttDef>

    <AttDef Type="gridalternate" BaseType="" Version="0" Unique="true">
      <ItemDefinitions>
        <String Name="gridinput" Label="Input Grid" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
          <ChildrenDefinitions>
            <File Name="meshfile" Label="meshfile" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
              <BriefDescription>Domain.py(physics::meshfile)</BriefDescription>
              <Categories>
                <Cat>General</Cat>
              </Categories>
            </File>
            <String Name="geometry" Label="Geometry specification" AdvanceLevel="0" NumberOfRequiredValues="1">
              <ChildrenDefinitions>
                <File Name="polyfile" Label="polyfile" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
                  <BriefDescription>Domain.py(physics::polyfile)</BriefDescription>
                  <Categories>
                    <Cat>General</Cat>
                  </Categories>
                </File>
                <Double Name="L" Label="Domain Length" Version="0" AdvanceLevel="0" NumberOfRequiredValues="3">
                  <BriefDescription>(physics::L)</BriefDescription>
                  <DefaultValue>1</DefaultValue>
                  <ComponentLabels>
                    <Label>X</Label>
                    <Label>Y</Label>
                    <Label>Z</Label>
                  </ComponentLabels>
                  <Categories>
                    <Cat>General</Cat>
                  </Categories>
                </Double>
                <Int Name="{nnx,nny,nnz}" Label="Number of Nodes" Version="0" AdvanceLevel="0" NumberOfRequiredValues="3">
                  <BriefDescription>(numerics::{nnx,nny,nnz})</BriefDescription>
                  <DefaultValue>3</DefaultValue>
                  <ComponentLabels>
                    <Label>X</Label>
                    <Label>Y</Label>
                    <Label>Z</Label>
                  </ComponentLabels>
                  <Categories>
                    <Cat>General</Cat>
                  </Categories>
                </Int>
                <String Name="domain" Label="Domain Object" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
                  <BriefDescription>Domain.py(physics::domain)</BriefDescription>
                  <!-- acbauer - may want to add in the constructor options specified in Domain.py but
                       for more complex domains we may want to create the geometry in ModelBuilder -->
                  <DiscreteInfo>
                    <Value Enum="RectangularDomain">RectangularDomain</Value>
                    <Value Enum="PlanarStraightLineGraphDomain">PlanarStraightLineGraphDomain</Value>
                    <Value Enum="TriangulatedSurfaceDomain">TriangulatedSurfaceDomain</Value>
                    <Value Enum="Mesh2DMDomain">Mesh2DMDomain</Value>
                    <Value Enum="Mesh3DMDomain">Mesh3DMDomain</Value>
                    <Value Enum="MeshHexDomain">MeshHexDomain</Value>
                    <Value Enum="MeshTetgenDomain">MeshTetgenDomain</Value>
                    <Value Enum="PiecewiseLinearComplexDomain">PiecewiseLinearComplexDomain</Value>
                  </DiscreteInfo>
                  <Categories>
                    <Cat>General</Cat>
                  </Categories>
                </String>

                <Void Name="movingDomain" Label="Moving Domain" Version="0" AdvanceLevel="0" Optional="true" IsEnabledByDefault="false" NumberOfRequiredValues="0">
                  <BriefDescription>(physics::movingDomain)</BriefDescription>
                  <Categories>
                    <Cat>General</Cat>
                  </Categories>
                </Void>
              </ChildrenDefinitions>
              <DiscreteInfo DefaultIndex="0">
                <Structure>
                  <Value Enum="Poly File">polyfile</Value>
                  <Items>
                    <Item>polyfile</Item>
                  </Items>
                </Structure>
                <Structure>
                  <Value Enum="Box Geometry">box</Value>
                  <Items>
                    <Item>L</Item>
                    <Item>{nnx,nny,nnz}</Item>
                  </Items>
                </Structure>
                <Structure>
                  <Value Enum="Complex Domain">domain</Value>
                  <Items>
                    <Item>domain</Item>
                    <Item>movingDomain</Item>
                  </Items>
                </Structure>
              </DiscreteInfo>
            </String>
            <String Name="triangleOptions" Label="Triangle Options" Version="0" AdvanceLevel="0" Optional="true" IsEnabledByDefault="false" NumberOfRequiredValues="1">
              <BriefDescription>(numerics::triangleOptions)</BriefDescription>
              <DefaultValue>q30DenA</DefaultValue>
              <Categories>
                <Cat>General</Cat>
              </Categories>
            </String>
            <Void Name="genMesh" Label="Trigger Mesh Generation" Version="0" AdvanceLevel="1" Optional="true" IsEnabledByDefault="true" NumberOfRequiredValues="0">
              <BriefDescription>(physics::genMesh)</BriefDescription>
              <Categories>
                <Cat>General</Cat>
              </Categories>
            </Void>
          </ChildrenDefinitions>
          <DiscreteInfo DefaultIndex="0">
            <Structure>
              <Value Enum="Mesh file">param</Value>
              <Items>
                <Item>meshfile</Item>
              </Items>
            </Structure>
            <Structure>
              <Value Enum="Auto-generated">verbose</Value>
              <Items>
                <Item>geometry</Item>
                <Item>triangleOptions</Item>
                <Item>genMesh</Item>
              </Items>
            </Structure>
          </DiscreteInfo>
        </String>
      </ItemDefinitions>
    </AttDef>

    <AttDef Type="numerics_tools" BaseType="" Version="0" Unique="true">
      <ItemDefinitions>
        <String Name="subgridError" Label="Subgrid Error" Version="0" AdvanceLevel="1" Optional="true" IsEnabledByDefault="false" NumberOfRequiredValues="1">
          <BriefDescription>SubgridError.py(numerics::subgridError)</BriefDescription>
          <DiscreteInfo DefaultValue="1">
            <Value Enum="Advection_ASGS">Advection_ASGS</Value>
            <Value Enum="AdvectionDiffusionReaction_ASGS">AdvectionDiffusionReaction_ASGS</Value>
            <Value Enum="FFDarcyFC_ASGS">FFDarcyFC_ASGS</Value>
            <Value Enum="DarcyFC_ASGS">DarcyFC_ASGS</Value>
            <Value Enum="HamiltonJacobi_ASGS">HamiltonJacobi_ASGS</Value>
            <Value Enum="HamiltonJacobi_ASGS_opt">HamiltonJacobi_ASGS_opt</Value>
            <Value Enum="StokesStabilization_1">StokesStabilization_1</Value>
            <Value Enum="StokesASGS_velocity">StokesASGS_velocity</Value>
            <Value Enum="NavierStokesASGS_velocity_pressure">NavierStokesASGS_velocity_pressure</Value>
            <Value Enum="NavierStokesASGS_velocity_pressure_opt">NavierStokesASGS_velocity_pressure_opt</Value>
            <Value Enum="NavierStokesASGS_velocity_pressure_optV2">NavierStokesASGS_velocity_pressure_optV2</Value>
            <Value Enum="StokesASGS_velocity_pressure">StokesASGS_velocity_pressure</Value>
            <Value Enum="TwophaseStokes_LS_FC_ASGS">TwophaseStokes_LS_FC_ASGS</Value>
            <Value Enum="ShallowWater_CFL">ShallowWater_CFL</Value>
            <Value Enum="AdvectionDiffusionReactionHaukeSangalliInterpolant_ASGS">AdvectionDiffusionReactionHaukeSangalliInterpolant_ASGS</Value>
          </DiscreteInfo>
          <Categories>
            <Cat>General</Cat>
          </Categories>
        </String>
        <Void Name="massLumping" Label="Lump Mass Matrix" Version="0" AdvanceLevel="0" Optional="true" IsEnabledByDefault="false" NumberOfRequiredValues="0">
          <BriefDescription>(numerics::massLumping)</BriefDescription>
          <Categories>
            <Cat>General</Cat>
          </Categories>
        </Void>
        <Void Name="reactionLumping" Label="Lump Reaction Term" Version="0" AdvanceLevel="0" Optional="true" IsEnabledByDefault="false" NumberOfRequiredValues="0">
          <BriefDescription>(numerics::reactionLumping)</BriefDescription>
          <Categories>
            <Cat>General</Cat>
          </Categories>
        </Void>
        <String Name="shockCapturing" Label="Shock Capturing" Version="0" AdvanceLevel="0" Optional="true" IsEnabledByDefault="false" NumberOfRequiredValues="1">
          <BriefDescription>ShockCapturing.py(numerics::shockCapturing)</BriefDescription>
          <DiscreteInfo>
            <Value Enum="ShockCapturing_base">ShockCapturing_base</Value>
            <Value Enum="ResGrad_SC">ResGrad_SC</Value>
            <Value Enum="ResGradFFDarcy_SC">ResGradFFDarcy_SC</Value>
            <Value Enum="ResGradQuad_SC">ResGradQuad_SC</Value>
            <Value Enum="Eikonal_SC">Eikonal_SC</Value>
            <Value Enum="ScalarAdvection_SC">ScalarAdvection_SC</Value>
            <Value Enum="HamiltonJacobi_SC">HamiltonJacobi_SC</Value>
            <Value Enum="HamiltonJacobiJaffre_SC">HamiltonJacobiJaffre_SC</Value>
            <Value Enum="JaffreGradU_SC">JaffreGradU_SC</Value>
            <Value Enum="ResGradJuanes_SC">ResGradJuanes_SC</Value>
          </DiscreteInfo>
          <Categories>
            <Cat>General</Cat>
          </Categories>
        </String>
        <String Name="numericalFluxType" Label="Numerical Flux Type" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
          <BriefDescription>NumericalFlux.py(numerics::numericalFluxType)</BriefDescription>
          <DiscreteInfo>
            <Value Enum="NF_base">NF_base</Value>
            <Value Enum="DoNothing">DoNothing</Value>
            <Value Enum="NoFlux">NoFlux</Value>
            <Value Enum="StrongDirichlet">StrongDirichlet</Value>
            <Value Enum="Advection_DiagonalUpwind">Advection_DiagonalUpwind</Value>
            <Value Enum="Advection_Diagonal_average">Advection_Diagonal_average</Value>
            <Value Enum="Advection_DiagonalUpwind_Diffusion_IIPG">Advection_DiagonalUpwind_Diffusion_IIPG</Value>
            <Value Enum="Advection_DiagonalUpwind_Diffusion_IIPG_exterior">Advection_DiagonalUpwind_Diffusion_IIPG_exterior</Value>
            <Value Enum="Advection_DiagonalUpwind_IIPG_exterior">Advection_DiagonalUpwind_IIPG_exterior</Value>
            <Value Enum="Curvature_exterior">Curvature_exterior</Value>
            <Value Enum="Stokes_Advection_DiagonalUpwind_Diffusion_IIPG_exterior">Stokes_Advection_DiagonalUpwind_Diffusion_IIPG_exterior</Value>
            <Value Enum="StokesP_Advection_DiagonalUpwind_Diffusion_IIPG_exterior">StokesP_Advection_DiagonalUpwind_Diffusion_IIPG_exterior</Value>
            <Value Enum="NavierStokes_Advection_DiagonalUpwind_Diffusion_IIPG_exterior">NavierStokes_Advection_DiagonalUpwind_Diffusion_IIPG_exterior</Value>
            <Value Enum="Diffusion_IIPG_exterior">Diffusion_IIPG_exterior</Value>
            <Value Enum="DarcySplitPressure_IIPG_exterior">DarcySplitPressure_IIPG_exterior</Value>
            <Value Enum="Diffusion_LDG">Diffusion_LDG</Value>
            <Value Enum="HamiltonJacobi_DiagonalLesaintRaviart">HamiltonJacobi_DiagonalLesaintRaviart</Value>
            <Value Enum="HamiltonJacobi_DiagonalLesaintRaviart_Diffusion_IIPG">HamiltonJacobi_DiagonalLesaintRaviart_Diffusion_IIPG</Value>
            <Value Enum="DarcyFCFF_IIPG_exterior">DarcyFCFF_IIPG_exterior</Value>
            <Value Enum="DarcyFC_IIPG_exterior">DarcyFC_IIPG_exterior</Value>
            <Value Enum="DarcyFCPP_IIPG_exterior">DarcyFCPP_IIPG_exterior</Value>
            <Value Enum="ShallowWater_1D">ShallowWater_1D</Value>
            <Value Enum="ShallowWaterHLL_1D">ShallowWaterHLL_1D</Value>
            <Value Enum="ShallowWater_2D">ShallowWater_2D</Value>
            <Value Enum="HamiltonJacobi_DiagonalChengShu">HamiltonJacobi_DiagonalChengShu</Value>
            <Value Enum="Stress_IIPG_exterior">Stress_IIPG_exterior</Value>
            <Value Enum="Richards_IIPG_exterior">Richards_IIPG_exterior</Value>
          </DiscreteInfo>
          <Categories>
            <Cat>General</Cat>
          </Categories>
        </String>
        <String Name="conservativeFlux" Label="conservativeFlux" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
          <BriefDescription>(numerics::conservativeFlux)</BriefDescription>
          <DiscreteInfo>
            <Value Enum="p1-nc">p1-nc</Value>
            <Value Enum="pwc">pwc</Value>
            <Value Enum="pwl">pwl</Value>
            <Value Enum="pwl-bdm">pwl-bdm</Value>
            <Value Enum="pwl-opt">pwl-opt</Value>
            <Value Enum="pwl-bdm-opt">pwl-bdm-opt</Value>
            <Value Enum="sun-rt0">sun-rt0</Value>
            <Value Enum="sun-gs-rt0">sun-gs-rt0</Value>
            <Value Enum="point-eval">point-eval</Value>
            <Value Enum="dg-point-eval">dg-point-eval</Value>
            <Value Enum="point-eval-gwvd">point-eval-gwvd</Value>
            <Value Enum="dg">dg</Value>
            <Value Enum="dg-bdm">dg-bdm</Value>
            <Value Enum="pwl-ib-fix-0">pwl-ib-fix-0</Value>
          </DiscreteInfo>
        </String>
        <Void Name="checkMass" Label="checkMass" Version="0" AdvanceLevel="0" Optional="true" IsEnabledByDefault="false" NumberOfRequiredValues="0">
          <BriefDescription>(numerics::checkMass)</BriefDescription>
        </Void>
        <Void Name="needEBQ_GLOBAL" Label="needEBQ_GLOBAL" Version="0" AdvanceLevel="0" Optional="true" IsEnabledByDefault="false" NumberOfRequiredValues="0">
          <BriefDescription>(numerics::needEBQ_GLOBAL)</BriefDescription>
        </Void>
        <Void Name="needEBQ" Label="needEBQ" Version="0" AdvanceLevel="0" Optional="true" IsEnabledByDefault="false" NumberOfRequiredValues="0">
          <BriefDescription>(numerics::needEBQ)</BriefDescription>
        </Void>
        <Void Name="restrictFineSolutionToAllMeshes" Label="restrictFineSolutionToAllMeshes" Version="0" AdvanceLevel="0" Optional="true" IsEnabledByDefault="false" NumberOfRequiredValues="0">
          <BriefDescription>(numerics::restrictFineSolutionToAllMeshes)</BriefDescription>
        </Void>
        <Void Name="parallelPeriodic" Label="parallelPeriodic" Version="0" AdvanceLevel="1" Optional="true" IsEnabledByDefault="false" NumberOfRequiredValues="0">
          <BriefDescription>(numerics::parallelPeriodic)</BriefDescription>
        </Void>

      </ItemDefinitions>
    </AttDef>

    <AttDef Type="solver" BaseType="" Version="0" Unique="true">
      <ItemDefinitions>
        <Group Name="solver" Label="Equation Solver" NumberOfRequiredGroups="1">
          <ItemDefinitions>
            <String Name="multilevelNonlinearSolver" Label="Multilevel Nonlinear Solver" Version="0" AdvanceLevel="1" NumberOfRequiredValues="1">
              <BriefDescription>NonlinearSolvers.py(numerics::multilevelNonlinearSolver)</BriefDescription>
              <DiscreteInfo DefaultIndex="1">
                <Value Enum="MultilevelNonlinearSolver">MultilevelNonlinearSolver</Value>
                <Value Enum="NLNI">NLNI</Value>
              </DiscreteInfo>
              <Categories>
                <Cat>General</Cat>
              </Categories>
            </String>
            <String Name="levelNonlinearSolver" Label="Level Nonlinear Solver" Version="0" AdvanceLevel="1" NumberOfRequiredValues="1">
              <BriefDescription>NonlinearSolvers.py(numerics::levelNonlinearSolver)</BriefDescription>
              <DiscreteInfo DefaultIndex="1">
                <Value Enum="NonlinearSolver">NonlinearSolver</Value>
                <Value Enum="Newton">Newton</Value>
                <Value Enum="NewtonNS">NewtonNS</Value>
                <Value Enum="NLJacobi">NLJacobi</Value>
                <Value Enum="NLGaussSeidel">NLGaussSeidel</Value>
                <Value Enum="NLStarILU">NLStarILU</Value>
                <Value Enum="FasTwoLevel">FasTwoLevel</Value>
              </DiscreteInfo>
              <Categories>
                <Cat>General</Cat>
              </Categories>
            </String>
            <String Name="nonlinearSmoother" Label="Nonlinear Smoother" Version="0" AdvanceLevel="1" NumberOfRequiredValues="1">
              <BriefDescription>NonlinearSolvers.py(numerics::nonlinearSmoother)</BriefDescription>
              <DiscreteInfo DefaultIndex="4">
                <Value Enum="NonlinearSolver">NonlinearSolver</Value>
                <Value Enum="Newton">Newton</Value>
                <Value Enum="NewtonNS">NewtonNS</Value>
                <Value Enum="NLJacobi">NLJacobi</Value>
                <Value Enum="NLGaussSeidel">NLGaussSeidel</Value>
                <Value Enum="NLStarILU">NLStarILU</Value>
                <Value Enum="FasTwoLevel">FasTwoLevel</Value>
              </DiscreteInfo>
              <Categories>
                <Cat>General</Cat>
              </Categories>
            </String>
            <Void Name="fullNewtonFlag" Label="Use Full Newton" Version="0" AdvanceLevel="1" Optional="true" IsEnabledByDefault="true" NumberOfRequiredValues="0">
              <BriefDescription>(numerics::fullNewtonFlag)</BriefDescription>
              <Categories>
                <Cat>General</Cat>
              </Categories>
            </Void>
            <String Name="nonlinearSolverNorm" Label="Nonlinear Solver Norm" Version="0" AdvanceLevel="1" Optional="true" IsEnabledByDefault="false" NumberOfRequiredValues="1">
              <BriefDescription>(numerics::nonlinearSolverNorm)</BriefDescription>
              <DiscreteInfo DefaultIndex="1">
                <Value Enum="l1">l1Norm</Value>
                <Value Enum="l2">l2Norm</Value>
                <Value Enum="l infinity">l2InfNorm</Value>
              </DiscreteInfo>
              <Categories>
                <Cat>General</Cat>
              </Categories>
            </String>
          </ItemDefinitions>
        </Group>
      </ItemDefinitions>
    </AttDef>

    <!-- undocumented default_n.py inputs -->
    <AttDef Type="numerics_other" BaseType="" Version="0" Unique="true">
      <ItemDefinitions>
        <Group Name="othernumerics" Label="Undocumented numerics items" NumberOfRequiredGroups="1">
          <ItemDefinitions>
            <Double Name="tolFace" Label="tolFace" Version="0" AdvanceLevel="1" NumberOfRequiredValues="1">
              <BriefDescription>(numerics::tolFac)</BriefDescription>
              <DefaultValue>0.01</DefaultValue>
            </Double>
            <Double Name="atol" Label="atol" Version="0" AdvanceLevel="1" NumberOfRequiredValues="1">
              <BriefDescription>(numerics::atol)</BriefDescription>
              <DefaultValue>1.0e-8</DefaultValue>
            </Double>
            <Int Name="maxNonlinearIts" Label="maxNonlinearIts" Version="0" AdvanceLevel="1" NumberOfRequiredValues="1">
              <BriefDescription>(numerics::maxNonlinearIts)</BriefDescription>
              <DefaultValue>10</DefaultValue>
            </Int>
            <Int Name="maxLineSearches" Label="maxLineSearches" Version="0" AdvanceLevel="1" NumberOfRequiredValues="1">
              <BriefDescription>(numerics::maxLineSearches)</BriefDescription>
              <DefaultValue>10</DefaultValue>
            </Int>

            <Int Name="psitc.nStepsForce" Label="psitc['nStepsForce']" Version="0" AdvanceLevel="1" NumberOfRequiredValues="1">
              <BriefDescription>(numerics::psitc['nStepsForce'])</BriefDescription>
              <DefaultValue>3</DefaultValue>
            </Int>
            <Int Name="psitc.nStepsMax" Label="psitc['nStepsMax']" Version="0" AdvanceLevel="1" NumberOfRequiredValues="1">
              <BriefDescription>(numerics::psitc['nStepsMax'])</BriefDescription>
              <DefaultValue>100</DefaultValue>
            </Int>
            <Double Name="psitc.reduceRatio" Label="psitc['reduceRatio']" Version="0" AdvanceLevel="1" NumberOfRequiredValues="1">
              <BriefDescription>(numerics::psitc['reduceRatio'])</BriefDescription>
            </Double>
            <Double Name="psitc.startRatio" Label="psitc['startRatio']" Version="0" AdvanceLevel="1" NumberOfRequiredValues="1">
              <BriefDescription>(numerics::psitc['startRatio'])</BriefDescription>
            </Double>


            <String Name="matrix" Label="matrix" Version="0" AdvanceLevel="1" NumberOfRequiredValues="1">
              <BriefDescription>(numerics::matrix)</BriefDescription>
              <DiscreteInfo DefaultIndex="0">
                <Value Enum="Sparse">SparseMatrix</Value>
                <Value Enum="Dense">DenseMatrix</Value>
              </DiscreteInfo>
            </String>
            <String Name="multilevelLinearSolver" Label="multilevelLinearSolver" Version="0" AdvanceLevel="1" NumberOfRequiredValues="1">
              <BriefDescription>(numerics::multilevelLinearSolver)</BriefDescription>
              <DiscreteInfo DefaultIndex="10"> <!-- NI is default -->
                <Value Enum="LU">LU</Value>
                <Value Enum="PETSc">PETSc</Value>
                <Value Enum="KSP_petsc4py">KSP_petsc4py</Value>
                <Value Enum="Jacobi">Jacobi</Value>
                <Value Enum="GaussSeidel">GaussSeidel</Value>
                <Value Enum="StarILU">StarILU</Value>
                <Value Enum="StarBILU">StarBILU</Value>
                <Value Enum="TwoLevel">TwoLevel</Value>
                <Value Enum="MultilevelLinearSolver">MultilevelLinearSolver</Value>
                <Value Enum="MGM">MGM</Value>
                <Value Enum="NI">NI</Value>
              </DiscreteInfo>
            </String>
            <String Name="levelLinearSolver" Label="levelLinearSolver" Version="0" AdvanceLevel="1" NumberOfRequiredValues="1">
              <BriefDescription>(numerics::levelLinearSolver)</BriefDescription>
              <DiscreteInfo DefaultIndex="0">
                <Value Enum="LU">LU</Value>
                <Value Enum="PETSc">PETSc</Value>
                <Value Enum="KSP_petsc4py">KSP_petsc4py</Value>
                <Value Enum="Jacobi">Jacobi</Value>
                <Value Enum="GaussSeidel">GaussSeidel</Value>
                <Value Enum="StarILU">StarILU</Value>
                <Value Enum="StarBILU">StarBILU</Value>
                <Value Enum="TwoLevel">TwoLevel</Value>
                <Value Enum="MultilevelLinearSolver">MultilevelLinearSolver</Value>
                <Value Enum="MGM">MGM</Value>
                <Value Enum="NI">NI</Value>
              </DiscreteInfo>
            </String>
            <Void Name="computeEigenvalues" Label="Compute Eigenvalues" Version="0" AdvanceLevel="1" Optional="true" IsEnabledByDefault="false" NumberOfRequiredValues="0">
              <BriefDescription>(numerics::computeEigenvalues)</BriefDescription>
            </Void>
            <String Name="computeEigenvectors" Label="computeEigenvectors" Version="0" AdvanceLevel="1"  Optional="true" IsEnabledByDefault="false" NumberOfRequiredValues="1">
              <BriefDescription>(numerics::computeEigenvectors)</BriefDescription>
              <DiscreteInfo>
                <Value Enum="left">left</Value>
                <Value Enum="right">right</Value>
              </DiscreteInfo>
            </String>
            <String Name="linearSmoother" Label="linearSmoother" Version="0" AdvanceLevel="1" NumberOfRequiredValues="1">
              <BriefDescription>(numerics::linearSmoother)</BriefDescription>
              <DiscreteInfo DefaultIndex="6">
                <Value Enum="LinearSolver">LinearSolver</Value>
                <Value Enum="LU">LU</Value>
                <Value Enum="PETSc">PETSc</Value>
                <Value Enum="KSP_petsc4py">KSP_petsc4py</Value>
                <Value Enum="Jacobi">Jacobi</Value>
                <Value Enum="GaussSeidel">GaussSeidel</Value>
                <Value Enum="StarILU">StarILU</Value>
                <Value Enum="StarBILU">StarBILU</Value>
                <Value Enum="TwoLevel">TwoLevel</Value>
                <Value Enum="MultilevelLinearSolver">MultilevelLinearSolver</Value>
                <Value Enum="MGM">MGM</Value>
                <Value Enum="NI">NI</Value>
              </DiscreteInfo>
            </String>
            <Double Name="linTolFac" Label="linTolFac" Version="0" AdvanceLevel="1" NumberOfRequiredValues="1">
              <BriefDescription>(numerics::linTolFac)</BriefDescription>
              <DefaultValue>0.001</DefaultValue>
            </Double>
            <Int Name="multigridCycles" Label="multigridCycles" Version="0" AdvanceLevel="1" NumberOfRequiredValues="1">
              <BriefDescription>(numerics::multigridCycles)</BriefDescription>
              <DefaultValue>2</DefaultValue>
            </Int>
            <Int Name="preSmooths" Label="preSmooths" Version="0" AdvanceLevel="1" NumberOfRequiredValues="1">
              <BriefDescription>(numerics::preSmooths)</BriefDescription>
              <DefaultValue>2</DefaultValue>
            </Int>
            <Int Name="postSmooths" Label="postSmooths" Version="0" AdvanceLevel="1" NumberOfRequiredValues="1">
              <BriefDescription>(numerics::postSmooths)</BriefDescription>
              <DefaultValue>2</DefaultValue>
            </Int>
            <Void Name="computeLinearSolverRates" Label="computeLinearSolverRates" Version="0" AdvanceLevel="1" Optional="true" IsEnabledByDefault="false" NumberOfRequiredValues="0">
              <BriefDescription>(numerics::computeLinearSolverRates)</BriefDescription>
            </Void>
            <Void Name="printLinearSolverRates" Label="printLinearSolverRates" Version="0" AdvanceLevel="1" Optional="true" IsEnabledByDefault="false" NumberOfRequiredValues="0">
              <BriefDescription>(numerics::printLinearSolverRates)</BriefDescription>
            </Void>
            <Void Name="computeLevelLinearSolverRates" Label="computeLevelLinearSolverRates" Version="0" AdvanceLevel="1" Optional="true" IsEnabledByDefault="false" NumberOfRequiredValues="0">
              <BriefDescription>(numerics::computeLevelLinearSolverRates)</BriefDescription>
            </Void>
            <Void Name="printLevelLinearSolverInfo" Label="printLevelLinearSolverInfo" Version="0" AdvanceLevel="1" Optional="true" IsEnabledByDefault="false" NumberOfRequiredValues="0">
              <BriefDescription>(numerics::printLevelLinearSolverInfo)</BriefDescription>
            </Void>
            <Void Name="computeLinearSmootherRates" Label="computeLinearSmootherRates" Version="0" AdvanceLevel="1" Optional="true" IsEnabledByDefault="false" NumberOfRequiredValues="0">
              <BriefDescription>(numerics::computeLinearSmootherRates)</BriefDescription>
            </Void>
            <Void Name="printLinearSmootherInfo" Label="printLinearSmootherInfo" Version="0" AdvanceLevel="1" Optional="true" IsEnabledByDefault="false" NumberOfRequiredValues="0">
              <BriefDescription>(numerics::printLinearSmootherInfo)</BriefDescription>
            </Void>
            <Int Name="linearSolverMaxIts" Label="linearSolverMaxIts" Version="0" AdvanceLevel="1" NumberOfRequiredValues="1">
              <BriefDescription>(numerics::linearSolverMaxIts)</BriefDescription>
              <DefaultValue>1000</DefaultValue>
            </Int>
            <Int Name="linearWCycles" Label="linearWCycles" Version="0" AdvanceLevel="1" NumberOfRequiredValues="1">
              <BriefDescription>(numerics::linearWCycles)</BriefDescription>
              <DefaultValue>3</DefaultValue>
            </Int>
            <Int Name="linearPreSmooths" Label="linearPreSmooths" Version="0" AdvanceLevel="1" NumberOfRequiredValues="1">
              <BriefDescription>(numerics::linearPreSmooths)</BriefDescription>
              <DefaultValue>3</DefaultValue>
            </Int>
            <Int Name="linearPostSmooths" Label="linearPostSmooths" Version="0" AdvanceLevel="1" NumberOfRequiredValues="1">
              <BriefDescription>(numerics::linearPostSmooths)</BriefDescription>
              <DefaultValue>3</DefaultValue>
            </Int>
            <Void Name="computeNonlinearSolverRates" Label="computeNonlinearSolverRates" Version="0" AdvanceLevel="1" Optional="true" IsEnabledByDefault="true" NumberOfRequiredValues="0">
              <BriefDescription>(numerics::computeNonlinearSolverRates)</BriefDescription>
            </Void>
            <Void Name="printNonlinearSolverInfo" Label="printNonlinearSolverInfo" Version="0" AdvanceLevel="1" Optional="true" IsEnabledByDefault="false" NumberOfRequiredValues="0">
              <BriefDescription>(numerics::printNonlinearSolverInfo)</BriefDescription>
            </Void>
            <Void Name="computeNonlinearLevelSolverRates" Label="computeNonlinearLevelSolverRates" Version="0" AdvanceLevel="1" Optional="true" IsEnabledByDefault="false" NumberOfRequiredValues="0">
              <BriefDescription>(numerics::computeNonlinearLevelSolverRates)</BriefDescription>
            </Void>
            <Void Name="printNonlinearLevelSolverInfo" Label="printNonlinearLevelSolverInfo" Version="0" AdvanceLevel="1" Optional="true" IsEnabledByDefault="false" NumberOfRequiredValues="0">
              <BriefDescription>(numerics::printNonlinearLevelSolverInfo)</BriefDescription>
            </Void>
            <Void Name="computeNonlinearSmootherRates" Label="computeNonlinearSmootherRates" Version="0" AdvanceLevel="1" Optional="true" IsEnabledByDefault="false" NumberOfRequiredValues="0">
              <BriefDescription>(numerics::computeNonlinearSmootherRates)</BriefDescription>
            </Void>
            <Void Name="printNonlinearSmootherInfo" Label="printNonlinearSmootherInfo" Version="0" AdvanceLevel="1" Optional="true" IsEnabledByDefault="false" NumberOfRequiredValues="0">
              <BriefDescription>(numerics::printNonlinearSmootherInfo)</BriefDescription>
            </Void>
            <Int Name="nonlinearPreSmooths" Label="nonlinearPreSmooths" Version="0" AdvanceLevel="1" NumberOfRequiredValues="1">
              <BriefDescription>(numerics::nonlinearPreSmooths)</BriefDescription>
              <DefaultValue>3</DefaultValue>
            </Int>
            <Int Name="nonlinearPostSmooths" Label="nonlinearPostSmooths" Version="0" AdvanceLevel="1" NumberOfRequiredValues="1">
              <BriefDescription>(numerics::nonlinearPostSmooths)</BriefDescription>
              <DefaultValue>3</DefaultValue>
            </Int>
            <Int Name="nonlinearWCycles" Label="nonlinearWCycles" Version="0" AdvanceLevel="1" NumberOfRequiredValues="1">
              <BriefDescription>(numerics::nonlinearWCycles)</BriefDescription>
              <DefaultValue>3</DefaultValue>
            </Int>
            <Void Name="useEisenstatWalker" Label="useEisenstatWalker" Version="0" AdvanceLevel="1" Optional="true" IsEnabledByDefault="false" NumberOfRequiredValues="0">
              <BriefDescription>(numerics::useEisenstatWalker)</BriefDescription>
            </Void>
            <Int Name="maxErrorFailures" Label="maxErrorFailures" Version="0" AdvanceLevel="1" NumberOfRequiredValues="1">
              <BriefDescription>(numerics::maxErrorFailures)</BriefDescription>
              <DefaultValue>10</DefaultValue>
            </Int>
            <Int Name="maxSolverFailures" Label="maxSolverFailures" Version="0" AdvanceLevel="1" NumberOfRequiredValues="1">
              <BriefDescription>(numerics::maxSolverFailures)</BriefDescription>
              <DefaultValue>10</DefaultValue>
            </Int>
            <String Name="nonlinearSolverConvergenceTest" Label="nonlinearSolverConvergenceTest" Version="0" AdvanceLevel="1" NumberOfRequiredValues="1">
              <BriefDescription>(numerics::nonlinearSolverConvergenceTest)</BriefDescription>
              <DefaultValue>'r'</DefaultValue>
            </String>
            <String Name="levelNonlinearSolverConvergenceTest" Label="levelNonlinearSolverConvergenceTest" Version="0" AdvanceLevel="1" NumberOfRequiredValues="1">
              <BriefDescription>(numerics::levelNonlinearSolverConvergenceTest)</BriefDescription>
              <DefaultValue>'r'</DefaultValue>
            </String>
            <String Name="linearSolverConvergenceTest" Label="linearSolverConvergenceTest" Version="0" AdvanceLevel="1" NumberOfRequiredValues="1">
              <BriefDescription>(numerics::linearSolverConvergenceTest)</BriefDescription>
              <DefaultValue>'r'</DefaultValue>
            </String>
          </ItemDefinitions>
        </Group>
      </ItemDefinitions>
    </AttDef>

    <AttDef Type="Partitioning" Label="Partitioning" Abstract="0" Version="0">
      <ItemDefinitions>
        <String Name="parallelPartitioningType" Label="parallelPartitioningType" Version="0" AdvanceLevel="1" NumberOfRequiredValues="1">
          <BriefDescription>(numerics::parallelPartitioningType)</BriefDescription>
          <DiscreteInfo DefaultIndex="1">
            <Value Enum="Node">MeshParallelPartitioningTypes.node</Value>
            <Value Enum="Element">MeshParallelPartitioningTypes.element</Value>
          </DiscreteInfo>
          <Categories>
            <Cat>General</Cat>
          </Categories>
        </String>
        <Int Name="nLayersOfOverlapForParallel" Label="nLayersOfOverlapForParallel" Version="0" AdvanceLevel="1" NumberOfRequiredValues="1">
          <BriefDescription>(numerics::nLayersOfOverlapForParallel)</BriefDescription>
          <DefaultValue>1</DefaultValue>
        </Int>
      </ItemDefinitions>
    </AttDef>


    <AttDef Type="auxiliaryVariable" Label="AuxiliaryVariable" BaseType="" Abstract="1" Version="0"/>
    <AttDef Type="GatherDOF" Label="Gather DOF" BaseType="auxiliaryVariable" Abstract="0" Version="0">
      <ItemDefinitions>
        <String Name="fileName" Label="File Name" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
          <BriefDescription>(numerics::auxiliaryVariables)</BriefDescription>
        </String>
      </ItemDefinitions>
    </AttDef>
    <AttDef Type="BoundaryForce" Label="Boundary Force" BaseType="auxiliaryVariable" Abstract="0" Version="0">
      <ItemDefinitions>
        <Double Name="D" Label="D" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
          <BriefDescription>(numerics::auxiliaryVariables)</BriefDescription>
          <DefaultValue>1.0</DefaultValue>
        </Double>
        <Double Name="Ubar" Label="Ubar" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
          <BriefDescription>(numerics::auxiliaryVariables)</BriefDescription>
          <DefaultValue>1.0</DefaultValue>
        </Double>
        <Double Name="rho" Label="rho" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
          <BriefDescription>(numerics::auxiliaryVariables)</BriefDescription>
          <DefaultValue>1.0</DefaultValue>
        </Double>
      </ItemDefinitions>
    </AttDef>
    <AttDef Type="PressureProfile" Label="Pressure Profile" BaseType="auxiliaryVariable" Abstract="0" Version="0">
      <ItemDefinitions>
        <Int Name="flag" Label="flag" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
          <BriefDescription>(numerics::auxiliaryVariables)</BriefDescription>
          <DefaultValue>0</DefaultValue>
        </Int>
        <Double Name="center" Label="center" Version="0" AdvanceLevel="0" NumberOfRequiredValues="2">
          <BriefDescription>(numerics::auxiliaryVariables)</BriefDescription>
          <DefaultValue>0.0</DefaultValue>
        </Double>
        <Double Name="radius" Label="radius" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
          <BriefDescription>(numerics::auxiliaryVariables)</BriefDescription>
          <DefaultValue>1.0</DefaultValue>
        </Double>
      </ItemDefinitions>
    </AttDef>
    <AttDef Type="RecirculationLength" Label="Recirculation Length" BaseType="auxiliaryVariable" Abstract="0" Version="0">
      <ItemDefinitions>
        <String Name="rcStartX" Label="rcStartX" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
          <BriefDescription>(numerics::auxiliaryVariables)</BriefDescription>
          <DefaultValue>None</DefaultValue>
        </String>
      </ItemDefinitions>
    </AttDef>
    <AttDef Type="VelocityAverage" Label="Velocity Average" BaseType="auxiliaryVariable" Abstract="0" Version="0"/>
    <AttDef Type="BoundaryPressure" Label="Boundary Pressure" BaseType="auxiliaryVariable" Abstract="0" Version="0"/>
    <AttDef Type="ConservativeHistoryMC" Label="Conservative History MC" BaseType="GatherDOF" Abstract="0" Version="0"/>
    <AttDef Type="ConservativeHistoryLS" Label="Conservative History LS" BaseType="GatherDOF" Abstract="0" Version="0"/>
    <AttDef Type="VelocityNormOverRegion" Label="Velocity Norm Over Region" BaseType="auxiliaryVariable" Abstract="0" Version="0">
      <ItemDefinitions>
        <String Name="regionIdList" Label="Region Id List" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
          <BriefDescription>(numerics::auxiliaryVariables)</BriefDescription>
          <DefaultValue>[1]</DefaultValue> <!-- acbauer this is probably taking in a model entity tag -->
        </String>
      </ItemDefinitions>
    </AttDef>
    <AttDef Type="MassOverRegion" Label="Mass Over Region" BaseType="VelocityNormOverRegion" Abstract="0" Version="0">
      <ItemDefinitions>
        <Double Name="ci" Label="ci" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
          <BriefDescription>(numerics::auxiliaryVariables)</BriefDescription>
          <DefaultValue>0</DefaultValue>
        </Double>
      </ItemDefinitions>
    </AttDef>
    <AttDef Type="PT123VelocityGenerator" Label="PT123 VelocityGenerator" BaseType="auxiliaryVariable" Abstract="0" Version="0">
      <ItemDefinitions>
        <String Name="filebase" Label="File Base" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
          <BriefDescription>(numerics::auxiliaryVariables)</BriefDescription>
        </String>
        <String Name="tnList" Label="tnList" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
          <BriefDescription>(numerics::auxiliaryVariables)</BriefDescription> <!-- acbauer - i have no idea what this is -->
        </String>
        <Double Name="ci" Label="ci" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
          <BriefDescription>(numerics::auxiliaryVariables)</BriefDescription>
          <DefaultValue>0</DefaultValue>
        </Double>
      </ItemDefinitions>
    </AttDef>


    <!--*** Physics ***-->
    <!-- Physics are in poisson_3d_tetgen_p.py with defaults in defaults_p.py -->
    <AttDef Type="physicsname" BaseType="" Version="0" Unique="true">
      <ItemDefinitions>
        <Int Name="numberOfComponents" Label="Number Of Components" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
          <BriefDescription>doesn't match up to anything in numerics or physics</BriefDescription>
          <DefaultValue>1</DefaultValue>
          <RangeInfo>
            <Min Inclusive="true">1</Min>
          </RangeInfo>
          <Categories>
            <Cat>General</Cat>
          </Categories>
        </Int>
        <String Name="name" Label="Model Name" Version="0" AdvanceLevel="0" Optional="true" IsEnabledByDefault="true" NumberOfRequiredValues="1">
          <BriefDescription>(physics::name)</BriefDescription>
          <Categories>
            <Cat>General</Cat>
          </Categories>
        </String>
      </ItemDefinitions>
    </AttDef>
    <!-- (physics::nd) will come from the model -->
    <AttDef Type="analyticalSolution" BaseType="" Version="0" Unique="true">
      <ItemDefinitions>
        <String Name="T" Label="Analytical Solution" Version="0" AdvanceLevel="0" Optional="true" IsEnabledByDefault="false" NumberOfRequiredValues="1">
          <BriefDescription>AnalyticalSolutions.py(physics::analyticalSolution)</BriefDescription>
          <DiscreteInfo>
            <Value Enum="LinearAD_DiracIC">LinearAD_DiracIC</Value>
            <Value Enum="Buckley_Leverett_RiemannSoln">Buckley_Leverett_RiemannSoln</Value>
            <Value Enum="VortexDecay_u">VortexDecay_u</Value>
            <Value Enum="VortexDecay_v">VortexDecay_v</Value>
            <Value Enum="VortexDecay_p">VortexDecay_p</Value>
          </DiscreteInfo>
          <Categories>
            <Cat>General</Cat>
          </Categories>
        </String>
      </ItemDefinitions>
    </AttDef>
    <AttDef Type="coefficients" BaseType="" Abstract="0" Version="0" Unique="false">
      <ItemDefinitions>
        <String Name="coefficients" Label="Transport Coefficients" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
          <BriefDescription>AnalyticalSolutions.py(physics::coefficients)</BriefDescription>
          <DiscreteInfo>
            <Value Enum="TC_base">TC_base</Value>
            <Value Enum="LinearVADR_ConstantCoefficients">LinearVADR_ConstantCoefficients</Value>
            <Value Enum="LinearVADR_ConstantCoefficients_skew">LinearVADR_ConstantCoefficients_skew</Value>
            <Value Enum="LinearVADR_ConstantCoefficients_upper">LinearVADR_ConstantCoefficients_upper</Value>
            <Value Enum="LinearVADR_ConstantCoefficients_lower">LinearVADR_ConstantCoefficients_lower</Value>
            <Value Enum="LinearVADR_ConstantCoefficients_full">LinearVADR_ConstantCoefficients_full</Value>
            <Value Enum="NonlinearVADR_pqrst">NonlinearVADR_pqrst</Value>
            <Value Enum="NonlinearVADR_pqrst_full">NonlinearVADR_pqrst_full</Value>
            <Value Enum="UnitSquareRotation">UnitSquareRotation</Value>
            <Value Enum="UnitCubeRotation">UnitCubeRotation</Value>
            <Value Enum="NavierStokes">NavierStokes</Value>
            <Value Enum="ShallowWater">ShallowWater</Value>
            <Value Enum="Stokes">Stokes</Value>
            <Value Enum="StokesP">StokesP</Value>
            <Value Enum="TwophaseNavierStokes_LS_SO">TwophaseNavierStokes_LS_SO</Value>
            <Value Enum="TwophaseNavierStokes_ST_LS_SO">TwophaseNavierStokes_ST_LS_SO</Value>
            <Value Enum="ThreephaseNavierStokes_ST_LS_SO">ThreephaseNavierStokes_ST_LS_SO</Value>
            <Value Enum="TwophaseStokes_LS_SO">TwophaseStokes_LS_SO</Value>
            <Value Enum="TwophaseNavierStokes_VOF_SO">TwophaseNavierStokes_VOF_SO</Value>
            <Value Enum="TwophaseStokes_VOF_SO">TwophaseStokes_VOF_SO</Value>
            <Value Enum="NCLevelSetCoefficients">NCLevelSetCoefficients</Value>
            <Value Enum="CLevelSetCoefficients">CLevelSetCoefficients</Value>
            <Value Enum="VOFCoefficients">VOFCoefficients</Value>
            <Value Enum="LevelSetNormalCoefficients">LevelSetNormalCoefficients</Value>
            <Value Enum="LevelSetCurvatureCoefficients">LevelSetCurvatureCoefficients</Value>
            <Value Enum="LevelSetConservation">LevelSetConservation</Value>
            <Value Enum="ConservativeHeadRichardsL2projMualemVanGenuchten">ConservativeHeadRichardsL2projMualemVanGenuchten</Value>
            <Value Enum="ConservativeHeadRichardsL2projMualemVanGenuchtenBlockHet">ConservativeHeadRichardsL2projMualemVanGenuchtenBlockHet</Value>
            <Value Enum="ConservativeHeadRichardsMualemVanGenuchten">ConservativeHeadRichardsMualemVanGenuchten</Value>
            <Value Enum="ConservativeSatRichardsMualemVanGenuchten">ConservativeSatRichardsMualemVanGenuchten</Value>
            <Value Enum="ConservativeTotalHeadRichardsMualemVanGenuchten">ConservativeTotalHeadRichardsMualemVanGenuchten</Value>
            <Value Enum="ConservativeHeadRichardsBrooksCoreyBurdine">ConservativeHeadRichardsBrooksCoreyBurdine</Value>
            <Value Enum="ConservativeHeadRichardsMualemVanGenuchtenHet">ConservativeHeadRichardsMualemVanGenuchtenHet</Value>
            <Value Enum="ConservativeHeadRichardsBrooksCoreyBurdineHet">ConservativeHeadRichardsBrooksCoreyBurdineHet</Value>
            <Value Enum="ConservativeHeadRichardsMualemVanGenuchtenBlockHet">ConservativeHeadRichardsMualemVanGenuchtenBlockHet</Value>
            <Value Enum="ConservativeHeadRichardsMualemVanGenuchtenBlockHetV2">ConservativeHeadRichardsMualemVanGenuchtenBlockHetV2</Value>
            <Value Enum="SeepageBrezis">SeepageBrezis</Value>
            <Value Enum="ConservativeHeadRichardsJLeverett">ConservativeHeadRichardsJLeverett</Value>
            <Value Enum="ConservativeHeadRichardsJLeverettAni">ConservativeHeadRichardsJLeverettAni</Value>
            <Value Enum="ConstantVelocityLevelSet">ConstantVelocityLevelSet</Value>
            <Value Enum="UnitSquareVortexLevelSet">UnitSquareVortexLevelSet</Value>
            <Value Enum="RotatingVelocityLevelSet">RotatingVelocityLevelSet</Value>
            <Value Enum="EikonalEquationCoefficients">EikonalEquationCoefficients</Value>
            <Value Enum="RedistanceLevelSet">RedistanceLevelSet</Value>
            <Value Enum="ConservativeHead2PMualemVanGenuchten">ConservativeHead2PMualemVanGenuchten</Value>
            <Value Enum="PoissonEquationCoefficients">PoissonEquationCoefficients</Value>
            <Value Enum="LinearElasticity">LinearElasticity</Value>
            <Value Enum="MovingMesh">MovingMesh</Value>
            <Value Enum="kEpsilon">kEpsilon</Value>
            <Value Enum="kEpsilon_k">kEpsilon_k</Value>
            <Value Enum="kEpsilon_epsilon">kEpsilon_epsilon</Value>
            <Value Enum="ViscousBurgersEqn">ViscousBurgersEqn</Value>
            <Value Enum="BuckleyLeverettLiuExample">BuckleyLeverettLiuExample</Value>
            <Value Enum="VolumeAveragedNavierStokesFullDevStress">VolumeAveragedNavierStokesFullDevStress</Value>
            <Value Enum="GroundwaterTransportCoefficients">GroundwaterTransportCoefficients</Value>
            <Value Enum="GroundwaterBiodegradation01Coefficients">GroundwaterBiodegradation01Coefficients</Value>
            <Value Enum="GroundwaterBryantDawsonIonExCoefficients">GroundwaterBryantDawsonIonExCoefficients</Value>
            <Value Enum="GroundwaterTransportCoefficientsELLAM">GroundwaterTransportCoefficientsELLAM</Value>
            <Value Enum="ConservativeHeadRichardsMualemVanGenuchtenBlockHetV2withUpwind">ConservativeHeadRichardsMualemVanGenuchtenBlockHetV2withUpwind</Value>
            <Value Enum="DiffusiveWave_1D">DiffusiveWave_1D</Value>
            <Value Enum="DiffusiveWave_2D">DiffusiveWave_2D</Value>
            <Value Enum="SinglePhaseDarcyCoefficients">SinglePhaseDarcyCoefficients</Value>
          </DiscreteInfo>
          <Categories>
            <Cat>General</Cat>
          </Categories>
        </String>
      </ItemDefinitions>
    </AttDef>

    <!--***  Boundary Condition Definitions ***-->
    <AttDef Type="BoundaryCondition" BaseType="" Abstract="1" Version="0" Unique="false" Associations="f" >
      <ItemDefinitions>
        <String Name="function" Label="Function" Version="0" AdvanceLevel="0" MultipleLines="true" NumberOfRequiredValues="1">
          <BriefDescription>(physics::boundaryCondition function????)</BriefDescription>
          <Categories>
            <Cat>General</Cat>
          </Categories>
        </String>
      </ItemDefinitions>
    </AttDef>
    <!-- BC structure to enforce uniqueness/prevent overspecification of BCs on a boundary -->
    <AttDef Type="advectiveFluxBoundaryConditions" Label="Advective Flux Boundary Conditions" BaseType="BoundaryCondition" Abstract="0" Version="0" Unique="true" Associations="f" /><!-- (physics::advectiveFluxBoundaryConditions) -->
    <AttDef Type="diffusiveFluxBoundaryConditions" Label="Diffusive Flux Boundary Conditions" BaseType="BoundaryCondition" Abstract="0" Version="0" Unique="true" Associations="f" /><!-- (physics::diffusiveFluxBoundaryConditions) -->
    <AttDef Type="dirichletConditions" Label="Dirichlet Conditions" BaseType="BoundaryCondition" Abstract="0" Version="0" Unique="false" Associations="f" /><!-- (physics::dirichletConditions) -->
    <AttDef Type="fluxBoundaryConditions" Label="Flux Boundary Conditions" BaseType="BoundaryCondition" Abstract="0" Version="0" Unique="false" Associations="f" /><!-- (physics::fluxBoundaryConditions) -->
    <AttDef Type="periodicDirichletConditions" Label="Periodic Dirichlet Conditions" BaseType="BoundaryCondition" Abstract="0" Version="0" Unique="false" Associations="f" /><!-- (physics::periodicDirichletConditions) -->
    <AttDef Type="stressFluxBoundaryConditions" Label="Stress Tensor Flux Boundary Conditions" BaseType="BoundaryCondition" Abstract="0" Version="0" Unique="false" Associations="f" /><!-- (physics::stressFluxBoundaryConditions) -->
    <AttDef Type="weakDirichletConditions" Label="Weak Dirichlet Conditions" BaseType="BoundaryCondition" Abstract="0" Version="0" Unique="false" Associations="f"/><!-- (physics::weakDirichletConditions) -->

    <!--***  Initial Condition Definitions ***-->
    <AttDef Type="initialCondition" BaseType="" Version="0" Unique="false" Associations="r" > <!-- (physics::initialConditions) -->
      <ItemDefinitions>
        <String Name="function" Label="Function" Version="0" AdvanceLevel="0" MultipleLines="true" NumberOfRequiredValues="1">
          <BriefDescription>(physics::initialConditions function????)</BriefDescription>
          <Categories>
            <Cat>General</Cat>
          </Categories>
        </String>
      </ItemDefinitions>
    </AttDef>


    <AttDef Type="bcsTimeDependent" BaseType="" Version="0" Unique="true">
      <ItemDefinitions>
        <Void Name="bcsTimeDependent" Label="Time Dependent Boundary Conditions" Version="0" AdvanceLevel="0" Optional="true" IsEnabledByDefault="true" NumberOfRequiredValues="0">
          <BriefDescription>(physics::bcsTimeDependent)</BriefDescription>
          <Categories>
            <Cat>General</Cat>
          </Categories>
        </Void>
      </ItemDefinitions>
    </AttDef>
    <AttDef Type="dummyInitialCondition" BaseType="" Version="0" Unique="true">
      <ItemDefinitions>
        <Void Name="dummyInitialCondition" Label="Dummy Initial Conditions" Version="0" AdvanceLevel="0" Optional="true" IsEnabledByDefault="false" NumberOfRequiredValues="0">
          <BriefDescription>(physics::dummyInitialConditions)</BriefDescription>
          <Categories>
            <Cat>General</Cat>
          </Categories>
        </Void>
      </ItemDefinitions>
    </AttDef>
    <!-- (physics::finalizeStepDummy) comes from finalizeStep -->
    <!-- finalizeStep is a user-specified python function. It provides a handle for someone to fix up solutions after the time step is completed. I don't think we should expose it in the gui. CE Kees 2/26/2014 -->
    <AttDef Type="finalizeStep" BaseType="" Version="0" Unique="true">
      <ItemDefinitions>
        <String Name="finalizeStep" Label="Finalize Step" Version="0" AdvanceLevel="0" Optional="true" IsEnabledByDefault="true" MultipleLines="true" NumberOfRequiredValues="1">
          <BriefDescription>(physics::finalizeStep)</BriefDescription>
          <Categories>
            <Cat>General</Cat>
          </Categories>
        </String>
      </ItemDefinitions>
    </AttDef>
    <AttDef Type="physicsTime" BaseType="" Version="0" Unique="true">
      <ItemDefinitions>
        <Double Name="T" Label="End of Time Interval" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
          <BriefDescription>(physics::T)</BriefDescription>
          <DefaultValue>1</DefaultValue>
          <Categories>
            <Cat>General</Cat>
          </Categories>
        </Double>
      </ItemDefinitions>
    </AttDef>
    <AttDef Type="sparseDiffusionTensors" BaseType="" Version="0" Unique="true">
      <ItemDefinitions>
        <Void Name="sd" Label="Use Sparse Representation of Diffusion Tensors" Version="0" AdvanceLevel="0" Optional="true" IsEnabledByDefault="true" NumberOfRequiredValues="0">
          <BriefDescription>(physics::sd)</BriefDescription>
          <Categories>
            <Cat>General</Cat>
          </Categories>
        </Void>
      </ItemDefinitions>
    </AttDef>
    <AttDef Type="LevelModelType" BaseType="" Version="0" Unique="true">
      <ItemDefinitions>
        <String Name="LevelModelType" Label="Level Model Type" Version="0" AdvanceLevel="0" Optional="true" IsEnabledByDefault="true" NumberOfRequiredValues="1">
          <BriefDescription>(physics::LevelModelType)</BriefDescription>
          <DiscreteInfo DefaultIndex="0">
            <Value Enum="OneLevelTransport">OneLevelTransport</Value> <!-- the constructor for this takes in about 15 values that still need to be filled in - acbauer ?????? -->
            <Value Enum="OneLevelLADR">OneLevelLADR</Value> <!-- the constructor for this takes in about 15 values that still need to be filled in - acbauer ?????? -->
            <Value Enum="RE_NCP1_OneLevelTransport">RE_NCP1_OneLevelTransport</Value> <!-- the constructor for this takes in about 15 values that still need to be filled in - acbauer ?????? -->
          </DiscreteInfo>
          <Categories>
            <Cat>General</Cat>
          </Categories>
        </String>
      </ItemDefinitions>
    </AttDef>

    <!--

**************** this should be the end of the physics attributes *********************
**************** this should be the end of the physics attributes *********************
**************** this should be the end of the physics attributes *********************

    -->




  </Definitions>



  <!--**********  Attribute Instances ***********-->
  <Attributes>
  </Attributes>

  <!--********** Workflow Views ***********-->
  <RootView Title="SimBuilder">
    <DefaultColor>1., 1., 0.5, 1.</DefaultColor>
    <InvalidColor>1, 0.5, 0.5, 1</InvalidColor>

    <GroupView Title="Numerics" >
      <InstancedView Title="Solver">
        <InstancedAttributes>
          <Att Type="time">time</Att>
          <Att Type="solver">solver</Att>
          <Att Type="numerics_other">numerics_other</Att>
        </InstancedAttributes>
      </InstancedView>
      <InstancedView Title="FEM">
        <InstancedAttributes>
          <Att Type="femspaces">femspaces</Att>
          <Att Type="quadrature">quadrature</Att>
          <Att Type="numerics_tools">numerics_tools</Att>
        </InstancedAttributes>
      </InstancedView>
      <GroupView Title="Tolerances" Style="Tiled">
        <InstancedView Title="Global Tolerances">
          <InstancedAttributes>
            <Att Type="tolerancesalternate">Parameters</Att>
          </InstancedAttributes>
        </InstancedView>
        <AttributeView Title="Component Tolerances">
          <AttributeTypes>
            <Type>basetolerance</Type>
          </AttributeTypes>
        </AttributeView>
      </GroupView>
      <InstancedView Title="Mesh">
        <InstancedAttributes>
          <Att Type="Partitioning">partitioning</Att>
          <Att Type="multilevelmeshlevels">multilevelmeshlevels</Att>
        </InstancedAttributes>
      </InstancedView>
      <GroupView Title="Miscellaneous" Style="Tiled">
        <AttributeView Title="Auxiliary Variables">
          <AttributeTypes>
            <Type>auxiliaryVariable</Type>
          </AttributeTypes>
        </AttributeView>
      </GroupView>

    </GroupView>

    <GroupView Title="Physics" >
      <InstancedView Title="PDE">
        <InstancedAttributes>
          <Att Type="physicsname">ModelName</Att>
          <Att Type="analyticalSolution">analyticalSolution</Att>
          <Att Type="coefficients">Coefficients</Att>
          <Att Type="dummyInitialCondition">dummyInitialCondition</Att>
          <Att Type="physicsTime">Physics Time???</Att>
          <Att Type="sparseDiffusionTensors">Sparse Diffusion Tensors</Att>
          <Att Type="LevelModelType">LevelModelType</Att>
        </InstancedAttributes>
      </InstancedView>
      <GroupView Title="Boundary Conditions" Style="Tiled">
        <InstancedView Title="">
          <InstancedAttributes>
            <Att Type="bcsTimeDependent">bcsTimeDependent</Att>
          </InstancedAttributes>
        </InstancedView>
        <AttributeView Title="" ModelEntityFilter="f">
          <AttributeTypes>
            <Type>BoundaryCondition</Type>
          </AttributeTypes>
        </AttributeView>
      </GroupView>

      <AttributeView Title="Initial Conditions" ModelEntityFilter="r">
        <AttributeTypes>
          <Type>initialCondition</Type>
        </AttributeTypes>
      </AttributeView>
      <InstancedView Title="Domain">
        <InstancedAttributes>
          <Att Type="gridalternate">gridalternate</Att>
        </InstancedAttributes>
      </InstancedView>
    </GroupView>


  </RootView>
</SMTK_AttributeManager>
