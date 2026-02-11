# rtl-wm

Siemens RTL tiled window manager from the X11R3 contrib tape. Patched for
gcc.

Details from Christoph Lohmann from 19 Dec 2011:

Greetings comrades,

I am proud to announce, that we (Paul Onyschuk an I) got the
Siemens RTL Tiled Window Manager to work on modern systems. It
was one of the early tiled window managers and has some ideas
of dwm, but is something different. Just try it out.

At [0] is the source of the patched rtl, to compile with
modern gcc versions. You will need liboldX[1] for the compila-
tion.

Instructions:

        % tar -xzf rtl-5.2-working.tar.gz
        % cd rtl
        % mxmkmf && make && make install
        # will install to /usr/bin
        % cat > ~/.rtl.tools
        XTerm
        <Tab>xterm
        <Ctrl+D>
        % rtl

Now use the right mouse button to use the menus. The .rtl.tools
file is needed, so you are able to at least start an xterm from
within rtl.

There seem to many yet untouched features in rtl, which are doc-
umented in the doc/ directory.

-- 
There's a video from Siemens about it which is on YouTube:
https://www.youtube.com/watch?v=IOxIUeroIVQ

and some notes with patches for the original X11R3-contrib version, from 1989:
https://mirrors.nycbug.org/pub/The_Unix_Archive/Unix_Usenet/comp.sources.x/1989-June/000581.html

## Building on macOS

Requires MacPorts with X11 libraries and liboldX.

### Prerequisites

```
xcode-select --install
sudo port install xorg-xorgproto xorg-libX11 xorg-libXt
```

You'll also need liboldX from https://www.x.org/archive/individual/lib/liboldX-1.0.1.tar.gz

Build it with `./configure --prefix=/opt/local && make` (no need to install).

### Build

```
make -f Makefile.macos
```

If using a local liboldX build (not installed to /opt/local), fix the library path:

```
install_name_tool -change /opt/local/lib/liboldX.6.dylib \
    /path/to/liboldX-1.0.1/src/.libs/liboldX.6.dylib rtl
```

### Running

Start the X server directly (without a window manager), then run rtl:

```
/opt/local/bin/Xquartz :0 &
export DISPLAY=:0
./rtl &
```

To launch X applications, run:

```
export DISPLAY=:0
xterm &
```
