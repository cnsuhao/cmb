.. _mesh-tab:
.. index:: Mesh Tab

The Mesh Tab
============

This tab allows the user to mesh a model with an external mesher. The drop-down menu of "Mesher" will be empty unless the
user runs a mesher that is linked to ModelBuilder.

.. findfigure:: Mesh.*
	:align: center
	:scale: 75%

Currently ModelBuilder is not shipped with any third-party mesher, the user has to integrate his/her own mesher into ModelBuilder.

.. seealso::

	`SMTK Mesh System Reference <http://smtk.readthedocs.io/en/latest/userguide/mesh/index.html>`_  for meshing system

Take filigree as an example, we first create an interface that talks to ModelBuilder called FiligreeMesh.
In order to use the mesher in ModelBuilder, we must run it in the background. In our case, we call it in the terminal window.

After FiligreeMesh is called, the Mesh tab will have more content

.. findfigure:: FiligreeMesh.*
	:align: center
	:scale: 75%

The menus under Mesh tab depends on the mesher. In our example, global sizing, local sizing, refinement rate etc. can be specified.
Click on "Mesh", a 2D mesh will be generated on the model.
