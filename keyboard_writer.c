#include "keyboard_writer.h"
#include <stdio.h>
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/extensions/XTest.h>


int x11_key(char *zh){
  Display *dpy;
  dpy = XOpenDisplay(NULL);

  KeySym sym = XStringToKeysym(zh);
  KeyCode code = XKeysymToKeycode(dpy,sym);
  XTestFakeKeyEvent(dpy, code, True, 1);
  XTestFakeKeyEvent(dpy, code, False, 1);

  XFlush( dpy );
  XCloseDisplay( dpy );

  return 0;
}

int x11_key_shift(char *zh){
  Display *dpy;
  dpy = XOpenDisplay(NULL);

  KeySym sym = XStringToKeysym(zh);
  KeyCode code = XKeysymToKeycode(dpy,sym);
  XTestFakeKeyEvent(dpy, 50, True, 1);
  XTestFakeKeyEvent(dpy, code, True, 1);
  XTestFakeKeyEvent(dpy, code, False, 1);
  XTestFakeKeyEvent(dpy, 50, False, 1);

  XFlush( dpy );
  XCloseDisplay( dpy );

  return 0;
}

