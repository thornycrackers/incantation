// xkbcat: Logs X11 keypresses, globally.

#include <X11/XKBlib.h>
#include <X11/extensions/XInput2.h>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

const char * DEFAULT_DISPLAY    = ":0";
const bool   DEFAULT_PRINT_UP   = false;

char * get_key() {

    const char * hostname    = DEFAULT_DISPLAY;
    bool         printKeyUps = DEFAULT_PRINT_UP;

    // Set up X
    Display * disp = XOpenDisplay(hostname);
    if (NULL == disp) {
        fprintf(stderr, "Cannot open X display: %s\n", hostname);
        exit(1);
    }

    // Test for XInput 2 extension
    int xi_opcode;
    int queryEvent, queryError;
    if (! XQueryExtension(disp, "XInputExtension", &xi_opcode,
                &queryEvent, &queryError)) {
        // XXX Test version >=2
        fprintf(stderr, "X Input extension not available\n"); return "";
    }

    // Register events
    Window root = DefaultRootWindow(disp);
    XIEventMask m;
    m.deviceid = XIAllMasterDevices;
    m.mask_len = XIMaskLen(XI_LASTEVENT);
    m.mask = calloc(m.mask_len, sizeof(char));
    XISetMask(m.mask, XI_RawKeyPress);
    XISetMask(m.mask, XI_RawKeyRelease);
    XISelectEvents(disp, root, &m, 1);
    XSync(disp, false);
    free(m.mask);

    while (1) { // Forever
        XEvent event;
        XGenericEventCookie *cookie = (XGenericEventCookie*)&event.xcookie;
        XNextEvent(disp, &event);

        if (XGetEventData(disp, cookie) &&
                cookie->type == GenericEvent &&
                cookie->extension == xi_opcode)
        {
            switch (cookie->evtype)
            {
                case XI_RawKeyRelease: if (!printKeyUps) continue;
                case XI_RawKeyPress: {
                    XIRawEvent *ev = cookie->data;

                    // Ask X what it calls that key
                    KeySym s = XkbKeycodeToKeysym(disp, ev->detail, 0, 0);
                    if (NoSymbol == s) continue;
                    char *str = XKeysymToString(s);
                    if (NULL == str) continue;


                    if (printKeyUps) printf("%s", cookie->evtype == XI_RawKeyPress ? "+" : "-");
                    char *ret = malloc(1000);
                    sprintf(ret, "%s", str);
                    XCloseDisplay(disp);
                    return ret;
                    break;
                                     }
            }
        }
        fflush(stdout);
    }
}
