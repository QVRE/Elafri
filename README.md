# Elafri

### A lightweight terminal Engine for Linux

This Engine was written to help me (and others) bring simple ideas that don't
require any fancy graphics or advanced audio into practise and consequently
not have to deal with any troubles that may come from those, may that be having
to deal with a very big and complex library.
Elafri will provide Graphics and Text output, Keyboard and Mouse support, Audio
playback and a package manager for easily wrapping up and saving a lot of
different data into a single file

#### People who can most benefit are those who:

* make non commercial and simple Linux programs
* mostly care about making a hobby project
* look to add more interactiveness to their Terminal App
* like to mess around and tinker with things
* value simplicity over richness and high performance

#### People who can't benefit are those who:

* don't use Linux
* don't use C
* require complex graphics and sound
* want to have portability
* don't want to rely on Terminal emulators

## Design

Elafri is designed with simplicity and developer freedom in mind.
The Engine does not make the code you write run on top of it or inside it.
Instead, the developer is the one who invokes Elafri and choses what code runs
and when it runs. The entire project will stay less than 1000 lines of code in
order to be easy to read-through and lightweight

## Tinkering

I designed Elafri to very configurable and easy to explore.
All the source code files are in the same directory as your project and you have
maximum control over changing what happens without it being complicated to do so.
Many parts of the Engine can just be switched out and replaced and most of the
time, only Main() will get affected by that because of Elafri's modular design

#### Don't want the Package manager?
Just go to Elafri.c and comment out the #include and the Init call that
corresponds to it

#### Don't like the Frame rate System?
The Frame rate system is housed inside `Main()` and is only ~16 lines long.
As such, it is very easy to change, replace or just remove altogether without
even leaving main.c

## FAQ

#### How do I fetch User Input?
Input() is the function that sorts and simplifies stdin into:

- `m` which is a 2D Vector that holds mouse coordinates
- `mC` which is the Mouse Code. Some values are:
    - 32 - Left Mouse down (33 if mouse wheel or 34 if right mouse button)
    - 35 - Mouse up
    - 67 - Mouse moved
    - 64 - moved while LM down (65 mouse wheel 66 right mouse)
    - 96 - scroll up (97 down)
- `kbd` which is an 8 bit array of keyboard inputs
    - unicode get's passed as-is
    - 255 is passed for extended keys. See Elafri.c macros
