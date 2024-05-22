# Elafri

### An Engine focused on Simplicity and Tinkering


![Statue rendered in Elafri](./Demo/Images/statue.png)


Elafri is a free and open source simplistic and multi-environment Engine.
This Engine is written in C and can output graphics to the Terminal and SDL.
It is meant to provide very minimalistic features you can build upon for light programs.
The vision is to make everything as simple as possible (one action = one function).

I have purposely kept the linecount very low to make it easy to go through and
I've made the Engine's components modular (you need to include something to use it).
This makes things much easier and also allows you to easily tinker with any part of
the Engine that you're using.

#### People who can most benefit are those who:

* want to make simple and/or lightweight programs
* are working in an environment without OpenGL or any GPU assisted features
* are looking for a simple way to test out a concept or prototype

## Instructions

To use Elafri, you:

* first need to select a port (Terminal, SDL, etc...)
* go into that folder (ex. src/Terminal/)
* get all the files (main.c is an example program)
* to use some addon like 3D or file reading, you take those files from src/
* for information about each file, check out the README at src/

## Design philosophy

Elafri is designed with simplicity and developer freedom in mind.
The Engine does not make the code you write run on top of it or inside it.
Instead, the developer is the one who invokes Elafri and choses what code runs
and when it runs. It's not flexible but it can easily be tinkered with due to
the project's low line count.

![3D Cube in Elafri](./Demo/Images/cube.gif)

## Documentation

Since this Engine has not gotten much attention, I haven't felt the need to
write an exhaustive documentation but I may do it in the future.
Feel free to contact me for any questions at any time.

## Contact Information

- Email: niko7V@proton.me
- Discord: niko64

## Donations

I do not expect donations but if you wish to support Elafri, you can contact me
for details.
