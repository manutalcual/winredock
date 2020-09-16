Rearrange windows when you re-dock your laptop or change monitors.

# winredock

I'm tired of always rearranging windows when I leave my desk to go to
a meeting with my laptop. I always have to go one application to the
following restoring their previous positions and sizes.

This is quite annoying.

To sove this problem, I've developed a small tool to load window
positions and sizes, write a configuration file and be able to restore
the application windows to their previous sizes and positions.

It is a *tool* so don't expect to be a production class
application. (Comments and requess for improvement are welcome,
however.)

### Installation

1. Download bin/bin.zip
2. Unpack the contents into a directory on your computer (eg into a folder in your C:\ drive).
3. Create a shortcut to the *wm.exe* in `shell:startup` (C:\Users\<your user>\AppData\Roaming\Microsoft\Windows\Start Menu\Programs\Startup) so the program starts up when your computer does.

### Usage

There will be an icon in your traybar showing a gear with a green arrow. Redock will automatically save window positions and restore them when a display-change happens. Ignore the menu options, they're not needed (and deprecated).

### Compilation (MS Visual Studio)

There are errors actually in the VS compilation because of some
template machinery. Also because "using" statement.

I have the "comunity" VS version, so I don't really know if this an
issue with the code (although it compiles cleanly on Cygwin GCC) or a
version problem (being too old to allow those constructions.)
In any case, I don't have time to fix this unless there is a real
interest: if you want to compile from Windows, file an issue (or
better, as for a pull request!)

Go to the `msvs` folder, double click on the solution file (sln) and
compile whatever you want: Debug, Release.

Of course, for this version you don't need cygwin64 dlls, but you may
need MS c++ distributables, I don't know. (If so, file an issue on
Github.)

### Compilation (Cygwin64)

This is a Windows application, so it needs a Windows compiler. I prefer
not to pay for development tools, and I'm a Linux guy, so I use
Mingw64 to compile. That's out of this readme scope, so if you don't
know what I'm talking about, just fill an issue asking for a binary.

Then, the compilation.

Just:

    $ ./configure
    $ make

The standard automake procedure.

In the `scripts` directory there is a `reg` file to add an entry to
the registry to run the application at Windows startup. It's not
tested, _and it contains my user name_, so you have to change that:

    "amadock"="C:\\Users\\mcano\\wm.exe"

Where it says "mcano" you have to write your Windows user name.
