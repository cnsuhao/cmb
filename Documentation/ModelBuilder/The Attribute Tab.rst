.. _attribute-tab:
.. index:: Attribute Tab

The Attribute Tab
=================

This tab only appears when attribute data has been loaded.

There are one or more tabs along the right side of the attribute tab. These are defined and generated based on the loaded attribute file.

These tabs come in two variants: Attribute and Instanced.

Attribute Tabs
--------------

.. findfigure:: AttributeTabNewCopyDelete.*
	:align: right

These tabs relate to data that can exist an arbitrary number of times (such as Materials). These tabs can be identified by looking for a table with "New", "Copy", and "Delete" tabs above.

To add a new entry, click the "New" button. The new entry should be selected in the table. Double-click it in the "Attribute" column to edit the name. Click the "Color" column to change the representation color.

After selecting an entry, a set of configurable options should appear below the table. These can all be edited and are specific to the selected attribute entry.

For attributes that can be assigned, a two-column table will be shown. Only entities that can be assigned (volumes, faces, edges, and/or vertices) will be shown. Moving an entity to the left side will assign the currently selected attribute the entity.

.. findfigure:: AssigningAttributes.*
	:align: center

Instanced Tabs
--------------

These tabs relate to data that exists as a single copy global to the entire simulation (Timesteps, Gravity, etc.).

These tabs do not have a table for new entries. Rather, they have just the labels and values.

The figure below shows a sample tab with a set of constants global to the entire simulation.

.. findfigure:: AttributeTabInstanced.*
	:align: center

.. seealso::

	`SMTK Template File Reference <http://smtk.readthedocs.org/en/latest/userguide/attribute/file-syntax.html>`_  for populating the Attribute Tab
