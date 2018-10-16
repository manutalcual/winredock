# winredock

## Rearrange windows when you re-dock your laptop

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

### Not compiling

Use the zip in "bin" directory. There are Cygwin64 DLLs and the
executable app.

### Compilation (MS Visual Studio)

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

    $ configure
    $ make

The standar automake procedure.

In the `scripts` directory there is a `reg` file to add an entry to
the registry to run the application at Windows startup. It's not
tested, _and it contains my user name_, so you have to change that:

    "amadock"="C:\\Users\\mcano\\wm.exe"

Where it says "mcano" you have to write your Windows user name.

### Use instructions.

There will be an icon in your traybar showing a gear with a green
arrow. Just right click on the icon and a menu will appear:

    Get windows
    Save config.
    Read config
    --
    Exit

Get windows: will get you current windows positions and sizes.

Save config: will save those windows positions and sizes to a JSON
file.

Read config: will read that file.

Exit: will exit from the app.

At first run, you have to `Get windows` and `Save config.`, from now
on, you may `Read config` and, left clicking on the icon, the app will
restore windows.

You have to `Get windows` and `Save config` everytime you want to take
in account a new application.


Thsnk you for your time.

Manuel