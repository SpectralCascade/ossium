# About Ossium
Ossium is a bare-bones, cross-platform 2D game engine being built in C++ with SDL 2 and Box2D.

# Features
* Low number of dependencies (SDL 2 with a few small extension libraries and Box2D for physics)
* Extendable serialisation support via completely code-based schemas
* JSON serialisable Entity-Component System
* Extendable input system with action bindings, states and input claiming
* CSV and JSON parsing
* Audio playback and mixing
* Simple unit testing framework
* Rendering with layer support

# Pre-requisites
All SDL libraries are listed on the [main SDL 2 webpage](https://www.libsdl.org/download-2.0.php) and the [SDL 2 projects page](https://www.libsdl.org/projects).
* A compiler with the C++17 standard; I use Mingw-w64 via [MSYS2](https://www.msys2.org) for the latest stable GCC version.
* SDL 2.0.8 (or greater)
* SDL_Image
* SDL_Mixer
* SDL_TTF

# How to Use

At some stage I intend to write up documentation for Ossium (once I've cleaned up the code a bit). Until then, you can check up on [my website](https://timlanesoftware.com) from time to time for blog posts and articles about Ossium (and other projects I'm working on). Or you can dive into the code to better understand how different parts of the engine work for yourself. Note that the engine is constantly being updated and improved currently, so if you intend to use it for your own games and projects beware of bugs and changes over time.

If building from scratch, you will need to build Box2D as a static lib and have that output in `Engine/Box2D/bin/Release` as `libBox2D.a`. Once built it must be linked correctly (if you're on Linux, the .cbp file for Ossium should be setup to link against Box2D already - make sure you select the correct build target (likely named `ossium (Linux)` in the Code::Blocks drop down)).

# Roadmap
I have a rough [todo list](https://github.com/SpectralCascade/Ossium/wiki/Todo) that details what I'm working on currently.

# Additional notes
I'm developing this project using Visual Studio Code and Make with the GNU GCC/G++ compiler. The engine should work fine on Linux (tested) and Mac (untested as of yet), and it should be possible to make mobile builds for Android or iOS (both untested as of yet).

Before building, please make sure you are compiling with C++17 enabled (use compile option `-std=c++1z` or `-std=c++17` and make sure you're using GCC 8 or later) as the project makes use of some of those features.

Check out Lazy Foo's SDL 2 tutorials at www.lazyfoo.net if you're unfamiliar with SDL; if you don't know how to setup SDL 2 to build Ossium, check out the first tutorial.

P.S. if you make anything cool with Ossium, I'd love to hear about it!
