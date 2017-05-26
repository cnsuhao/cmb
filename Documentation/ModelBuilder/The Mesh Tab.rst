.. _mesh-tab:
.. index:: Mesh Tab

The Mesh Tab
============

This tab allows the user to mesh a model with an external mesher. By default the
mesh tab does not have much information. The dropdown menu of "Mesher" is empty
unless the user runs a mesher that is linked to ModelBuilder.

.. findfigure:: Mesh.*
	:align: right
	:scale: 75%

Since currently ModelBuilder is not shipped with any third-party mesher, the
user has to integrate his/her own mesher into ModelBuilder.

Here is an example of an external meshing package we have. We created an
interface for it so that it can talk to ModelBuilder. In order to use the
mesher in ModelBuilder, we must run it in the background. In our case, we run
it in the terminal window. After the mesher is called, the Mesh tab shows the
control options and parameters.

.. findfigure:: Mesher.*
	:align: center
	:scale: 75%

The menus under Mesh tab depends on the mesher and the interface program. In our
example, global sizing, local sizing, refinement rate etc. can be specified.
Click on "Mesh", a 2D mesh will be generated on the model. The figure below shows
the mesh generated on a random polygon model.

.. findfigure:: randomMesh.*
	:align: center
	:scale: 50%

.. seealso::
	`SMTK Mesh System Reference <http://smtk.readthedocs.io/en/latest/userguide/mesh/index.html>`_
	for meshing system
