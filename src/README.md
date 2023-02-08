# Overview

Each directory holds a port of Elafri for that platform. You also need [evar.h](./evar.h).
Elafri does not have a standardized input system so you will have to handle user input for
each port differently depending on that port's Elafri.h file.
You will also likely want to use an extra component like 3D rendering or image handling.
For that, see the Components section below.

## Released Versions

* [Terminal](./Terminal)
* [SDL](./SDL)

### Components

In this directory, you can see many files and below is a description of what each one does.
Add any of these files in your project to use them. Most of them require evar.h

#### General

* evar: provides definitions for types and vectors (used by Elafri)
* rnd: a wrapper for getting pseudo-random floating point numbers
* pkg: like a filesystem but for arrays. Allows simpler storage and retrieval
* image: for reading some image file formats

#### Audio

* sfx: a not yet finished system for playing sounds

#### 3D Rendering

* grmath: functions for 3D math like vector/matrix rotations, view transform, etc
* 3D: rendering for both textured and untextured 3D objects (dont use together with 3Dc)
* 3Dc: rendering for only untextured 3D objects (faces are single-color)
* object: for reading some 3D object file formats
