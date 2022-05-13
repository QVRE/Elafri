# Elafri

### An Engine focused on Simplicity and Tinkering

This Engine was written to help people bring simple ideas to life.
The vision is simplicity, customizability and tinkering. I try to keep the line
count as low as possible while also keeping it simple and easy to understand.
This is an educational and simplistic tool, sacrificing a lot of features for
the sake of providing a basic set of tools for each case. The 3D graphics have
a lot to be desired. I use euler angles and have relatively few functions for
doing something advanced, the triangles are single color and textureless and
there are just two drawing functions, one for 3D triangles and one for drawing
Elafri objects.

If you are fine with the minimalistic nature of this Engine then here are some
things that you can currently find and use:

* 2D & 3D graphics and things like framerate handling
* Both Keyboard and Mouse support for the terminal (and SDL of course)
* and more addons in the src/ directory such as basic RNG, a package handler, etc

#### People who can most benefit are those who:

* make non commercial and simple programs
* mostly care about making a hobby project
* like to mess around and tinker with things
* just want a simple "wrapper" to help them focus on their main idea

#### People who can't benefit are those who:

* are looking for a big proper engine with many features
* are not a big fan of tinkering their tool for every use case
* aren't using C of course

## Instructions

The way I use Elafri in a project I make is by:

* going into src/ and into the version I need
* getting the .c files of that directory and the makefile
* in some cases such as 3D graphics, I will get the contents of 3D/ instead
* in case I need some addon like 3D math, RNG, etc, I take those c files from src/

## Design

Elafri is designed with simplicity and developer freedom in mind.
The Engine does not make the code you write run on top of it or inside it.
Instead, the developer is the one who invokes Elafri and choses what code runs
and when it runs. The entire project aims to keep a low line count for the main
files as well as all the extensions

## Tinkering

I designed Elafri to very configurable and easy to explore.
I try my best to make sure that it is easy to tinker with and not a complex mess.
For that reason, I have chosen to not have .h files and use .c files only.
Many parts of the Engine can just be switched out and replaced and most of the
time, only Main() will get affected by that because of Elafri's modular design.
I took good care to keep a very simple design and even when you switch from the
Terminal to the SDL version, only the input mechanisms will change

## Documentation

So far, since I am the sole developer of everything regarding Elafri, there is
no available documentation. However, it will likely be added at some point.

## Contact Information

I am not sure if I will see an email so try to use the ones bellow if you can

- Email: nikos.raudonus@gmail.com
- Telegram: @Nik051105
- Discord: Nik#6291
- Tox: 23F3ECA183E9824F271FD3A90ECBE54FCE2BC847228487F6E152F20B0D4E3D4B910873FBB377

## Donations

I do not expect donations but if you wish to support Elafri, you can contact me
for details like monero/ethereum wallet info or such.
