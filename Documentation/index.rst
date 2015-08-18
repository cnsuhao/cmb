Computational Model Builder (CMB) Suite
====================================================

.. image:: ModelBuilder/images/ModelBuilderIcon.png
	:scale: 50%
	:align: center

Computational Model Builder (CMB) is a suite of applications
that provide a customizable workflow for the entire lifetime
of a computational simulation.
Generally speaking most workflows involve

+ specifying the governing equations of state of the
  system to be simulated, their free parameters, and
  other quantities required by the simulation;
+ importing, constructing, or fitting a geometric model of
  the physical domain of the simulation;
+ discretizing the geometric model into a mesh suitable for
  approximating functions which enable people to make a
  decision based on the outcome of the simulation;
+ associating material properties, boundary and initial
  conditions, and other attributes to regions of the mesh.
+ queueing the simulation (or an ensemble of simulations)
  for execution;
+ monitoring simulation progress and inspecting results
  after the simulation has run;
+ integrating simulation results, experimental measurements,
  and other knowledge in order to make a decision about how
  to proceed.

The first application in this suite that we are introducing
is ModelBuilder, which targets the first four steps above for
several major use cases.
ParaView_ is often used for the final steps.

.. _ParaView: http://paraview.org/

Contents:
---------
.. toctree::
   :maxdepth: 2

   ModelBuilder/index.rst


Indices and tables
------------------

* :ref:`genindex`
