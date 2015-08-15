What is Model Builder?
======================

ModelBuilder is an application for creating simulation input decks.
This manual breaks the overall process into 3 steps:

1. Specifying simulation parameters,
2. Relating parameters to geometry, and
3. Exporting an input deck.

The following paragraphs introduce some terminology
and provide a short summary of each task, which the
manual as a whole then describes in detail.

Each simulation package (e.g., Albany_, OpenFOAM_, `Deal II`_) accepts
different sets of parameters.
ModelBuilder calls such a set of parameters an *attribute*.
Attributes consist of child *items* (i.e., individual parameters) and,
depending on the values of those parameters, other conditional parameters
may be exposed — forming a hierarchy of items that together control the
simulation.

ModelBuilder can be customized for any simulation package by writing

1. an XML description of the parameters a simulation accepts (called a *template*) and

2. a Python script (called an *exporter*) that, given particular parameter values
   entered into ModelBuilder GUI, generates an input deck for the simulation.

Most often, you will use a pre-existing template and exporter.
Some come with ModelBuilder and some may be provided by your organization.
Thus, one of the first steps in using ModelBuilder will be to load in
a simulation *template*.

Once you have specified solver parameters, material properties, boundary conditions,
and initial conditions, you will need to associate some of these values with
regions of the simulation domain.
For example, in modeling groundwater flow, once the different sediment layers
are defined, they must be related, or *associated*, to a geometric portion of
the earth being simulated.

Thus one of the next tasks in ModelBuilder is to load or create a geometric model
and associate portions of it with simulation *attributes* or *items*.
ModelBuilder has some facilities for marking and manipulating geometric models
after they have been loaded, as well as a framework for generating mesh discretizations from
geometric models; however, other tools (to be added to the CMB suite
in the future) are intended to cover the process of creating geometric
models from sensor data.

Finally, once the simulation parameters have been specified and related
to the problem geometry, ModelBuilder can be used to *export* an input deck
for submission to a simulation.

.. _Albany: http://software.sandia.gov/albany
.. _Deal II: http://www.dealii.org/
.. _OpenFOAM: http://openfoam.org/
