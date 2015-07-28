Discrete Session
================

Edit Bathymetry
---------------
This allows for adding depth to 2D models. 

Radius for Averaging Elevation: The search radius for every point in search for other points to modify

Set Highest/Lowest Elevation: Set a hard cap for the generated elevation

The "Remove Bathymetry" option reverts this elevation operation

Entity Group
------------
Create Group: Creates a group based on the currently selected entities.

..Note:: 2D models create groups of edges, 3D models create groups of faces

Modify Group: Add/remove entities from an existing group

Remove Group: Remove an existing group

Grow
----
(3D models only)

Select triangles of a model.

.. findfigure:: pqGrowCell32.*
	:align: right

Select triangle

.. findfigure:: pqGrowCellPlus32.*

Add to current selection

.. findfigure:: pqGrowCellMinus32.*

Remove from current selection

.. findfigure:: pqCancel32.*

Clear selection

.. findfigure:: ExtractFacet24.*

.. todo:: todo

Feature Angle: The maximum angle difference for selection

.. Note:: If a box select is used, the feature angle is ignored and all triangles in
	the box selection are selected.

Import
------
Shape files being imported can specify the boundary type

Merge Face
----------
Combine faces of a model

Mesh
----
.. todo:: todo

Modify Edge
-----------
Using a box selection, create a new vertex

Read
----
Open files with the same functionality as File-Open

Set Property
------------
Set a property of the selected entity (e.g. visibility, color)

Split Face
----------
(3D only)

Split all triangles that are greater than -feature angle- apart into separate faces.

Write
-----
Write out the selected entities. Same operation as File-Save Simulation but works on any entity (e.g. a face).
