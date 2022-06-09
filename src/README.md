# Structure of source directory

Each directory holds a port of Elafri and its graphics handling.
This should, along with the Elafri Standard, allow you to just replace these
files in your project to port it to a different interface.

## Versions

* [Terminal](./Terminal)
* [SDL](./SDL)

### Addons

This directory holds addons that should (usually) work with all Elafri versions.
Here's a log of all the addons:

* evar: provides definitions of Elafri.c (included when ELAFRI is not defined)
* rnd: a wrapper for getting random floating point numbers
* pkg: like a filesystem but for arrays. Allows simpler storage and retrieval
* sfx: a not yet finished system for playing sounds
* grmath: a collection of functions for 3D math such as vectors and matrices
* render: Draws 3D Triangles and Objects
