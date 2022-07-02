# Structure of source directory

Each directory holds a port of Elafri and its graphics handling.
This should, along with the Elafri Standard, allow you to just replace these
files in your project to port it to a different interface.

## Released Versions

* [Terminal](./Terminal)
* [SDL](./SDL)

### Addons

This directory holds addons that should (usually) work with all Elafri versions.
Here's a list of all the addons:

#### General

* evar: provides definitions of Elafri.c (included when ELAFRI is not defined)
* rnd: a wrapper for getting random floating point numbers
* pkg: like a filesystem but for arrays. Allows simpler storage and retrieval

#### Audio

* sfx: a not yet finished system for playing sounds

#### 3D Rendering

* grmath: functions for 3D math like vector/matrix rotation, view transform, etc
* 3D: full 3D rendering for both textured and untextured objects
* 3Dc: for 3D rendering of only untextured objects with single color faces
