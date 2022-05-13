# Structure of source directory

Each directory holds a version of Elafri and the directories inside those have
extra or modified features for those versions such as an example of 3D graphics.

## Versions

* Terminal: The original Elafri that runs on the terminal
* SDL: Elafri modified to provide the same features in SDL

### Addons

This directory holds addons that should (usually) work with all Elafri versions.
Here's a log of all the addons:

* evar: just code to make things easier
* rnd: a wrapper for getting random floating point numbers
* pkg: a system to a lot of data together more easily
* sfx: a not yet finished system for playing sounds
* grmath: a collection of functions for 3D math such as vectors and matrices
* render: Elafri's system for drawing 3D objects and triangles
