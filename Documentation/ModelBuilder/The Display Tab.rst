
.. index:: Display Tab

The Display Tab
===============

.. findfigure:: DisplayTab.*
	:align: right

The Display Tab contains various options related to displaying the model.

Representation—Changes the way the model is displayed in the viewport
	
	When viewing the mesh of an object, it may be worth switching this to “Wireframe” or “Surface with edges”
	
	For a point cloud representation, select “Points”
	“Outline” gives a bounding box representation 

Styling
-------

Opacity—Controls the transparency of the model

Point Size—The size of the vertex points

Line Width—The thickness of the edges

Lighting
--------

Interpolation—Whether the shading is flat or gouraud

Specular—Controls the specularity of the model

Specular Color/Power—Controls the color and intensity of the 
specularity

Ambient—The amount of lighting influence from ambient light

Diffuse—The amount of lighting influence from the diffuse color of the elements' color values

Edge Styling
------------

Backface Representation—Adjust the representation of the backfaces. This can be combined with the “Representation” option at the top of the Display Tab.
	
	“Cull Backface” is useful for when the model has semi-transparent material colors or the current representation is “Points” or “Wireframe”
	
	“Cull Frontface” removes the immediate face facing the camera, allowing the insides of a model to be shown


Backface Opacity—The alpha transparency of the backfaces

Transforming
------------

Translation—Move the model in XYZ space

Scale—Scale in model by its relative axes

Orientation—The rotation of the model

Origin—The center of the model

Miscellaneous
-------------

Pickable—Whether the model can be picked in the viewport

Texture—

Triangulate—Whether or not to triangulate the model

Nonlinear Subdivision Level—How many levels of subdivision to apply

Block Colors Distinct Values—

Cube Axes—Show a set of XYZ axes along the model with distance markers
