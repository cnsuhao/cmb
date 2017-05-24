Getting ModelBuilder
====================

Downloading ModelBuilder
------------------------

ModelBuilder can be downloaded from the
`Official Site <http://www.computationalmodelbuilder.org/download/>`_.

.. Note::
	ModelBuilder v4 is currently in development, but nightly builds are available.

Building ModelBuilder from Source
---------------------------------
Because ModelBuilder has a lot of dependencies such as opencv, paraview, qt4,
smtk and so on, we separate the code base into
`CMB-SuperBuild <https://gitlab.kitware.com/cmb/cmb-superbuild>`_, and
`CMB <https://gitlab.kitware.com/cmb/cmb>`_, where the former contains
everything involved and the latter only contains ModelBuilder itself. Either way
the user needs to checkout the CMB-SuperBuild repository first.
In CMB-SuperBuild, ModelBuilder can be built in two different modes:
developer mode and release mode. If configured in the developer mode, all the
dependencies of ModelBuilder will be built, and a the config file
"cmb-Developer-Config.cmake" will generated. The user should checkout the CMB
repository outside the CMB-SuperBuild and build ModelBuilder itself. Doing so
separates ModelBuilder and its dependencies and enables the user to rebuild
ModelBuilder easily. If configured in the release mode, ModelBuilder itself as
well as everything it needs will be built, there is no need to checkout CMB
repository.

.. seealso::

	`Our gitlab <https://gitlab.kitware.com/cmb/cmb-superbuild>`_ for more instructions.
