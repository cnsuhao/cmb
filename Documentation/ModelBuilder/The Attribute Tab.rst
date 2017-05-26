.. _attribute-tab:
.. index:: Attribute Tab

The Attribute Tab
=================

This tab is used to specify simulation parameters and assign them to the model.
It only appears when a simulation template has been loaded.

There are usually one or more tabs along the right side of the attribute tab, which
separate the simulation parameters into different sections.
These are defined and generated based on the loaded attribute file. For example:

.. findfigure:: AttributeTab.*
	:align: center
	:scale: 50%

"Show Level" controls how detailed the attribute tab should be. The template
writer decides the level of information. This feature can be used to hide
unnecessary information for certain users.

Another common feature is "Show by Category". This is another way to categorize
the simulation parameters. Unlike "Show Level", it categorizes the parameters
based on their physical meanings (it is also specified by the template writer).

Attribute Tabs
--------------
.. findfigure:: AttributeTabNewCopyDelete.*
	:align: right

There are two types of parameters: attribute and instanced. The attribute parameter
can have multiple instances which allows the user to create or delete based on
the simulation needs, while the instanced has only one instance.

This figure shows a typical attribute, where you can add or delete materials.
To add a new entry, click the "New" button. The new entry should be selected in
the table. Double-click it in the "Attribute" column to edit the name.
Click the "Color" column to change the representation color.

After selecting an entry, a set of configurable options should appear below the
table. These can all be edited and are specific to the selected attribute entry.
For attributes that can be assigned, a two-column table will be shown.
Only entities that can be assigned (volumes, faces, edges, or vertices) will be
shown. Moving an entity to the left side will assign the entity to the selected
attribute.

.. findfigure:: AssigningAttributes.*
	:align: center

Instanced Tabs
--------------
Instanced tabs are used for data that has only one instance in the entire
simulation for example gravity, time steps and so on.

There will usually be a tabular which lists the labels and values of the
instanced parameters, as shown in the figure below:

.. findfigure:: AttributeTabInstanced.*
	:align: center

.. seealso::

	`SMTK Template File Reference <http://smtk.readthedocs.org/en/latest/userguide/attribute/file-syntax.html>`_  for populating the Attribute Tab

	:ref:`Using Simulation Template <using-simulation-template>` for an example
