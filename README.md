# Elafri

### An Engine focused on Simplicity


![Statue rendered in Elafri](./Demo/Images/statue.png)


Elafri is a free and open source oversimplistic and multi-environment Engine.
This Engine is written in C and is ported to the Terminal and SDL.
It is meant to provide very minimalistic features for simple problems and light
programs. The vision is: A single task should need about a single function call.

Nowadays, to do something simple, you have to first do many complicated things.
One example is OpenGL where you need to spend days figuring out everything when
it barely has anything to do with YOUR project.

That's what this Engine aims to help with.
It aims to keep the line count less than 1000 and be easy to look through.
However, this comes with a few limitations and extras. For one, I've decided
that if you don't need a specific set of functions such as 3D graphics or audio,
it shouldn't be in the code that's being compiled. For this reason, I've made
every component of Elafri be a seperate file that must be included in Elafri.c
to be used in it (graphics get to stay included by default though). Lastly, due
to the minimalism, features are very stripped. 3D graphics for example, have a
lot to be desired. There are no textures and triangles are single color

#### People who can most benefit are those who:

* want to make simple lightweight programs
* are fine with minimalistic features such as no textures for 3D
* just want the job done (play an audio buffer, draw a line, etc...)

#### People who can't benefit are those who:

* need non minimalistic and proper functions and features/support
* want those features to be portable to many use cases
* do not want to have to tinker their tool for every use case

## Instructions

To use Elafri, you:

* first need to select a port (Terminal, SDL, etc...)
* go into that folder (ex. src/Terminal/)
* get the C files and optionally the makefile to see the compile args
* to use some addon like 3D Math or Sound, you take those files from src/


![3D Cube in Elafri](./Demo/Images/cube.gif)


## The Standard

Elafri is designed with simplicity and developer freedom in mind.
The Engine does not make the code you write run on top of it or inside it.
Instead, the developer is the one who invokes Elafri and choses what code runs
and when it runs. The entire project aims to keep a low line count for the main
files as well as all the extensions.

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
Feel free to contact me for any questions.

## Contact Information

I am not sure if I will see an email so try to use the ones bellow if you can

- Email: nikos.raudonus@gmail.com
- Telegram: @Nik051105
- Discord: Nik#6291
- Tox: 23F3ECA183E9824F271FD3A90ECBE54FCE2BC847228487F6E152F20B0D4E3D4B910873FBB377

## Donations

I do not expect donations but if you wish to support Elafri, you can contact me
for details like monero/ethereum wallet info or such.
