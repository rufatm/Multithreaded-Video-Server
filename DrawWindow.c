#include <stdio.h>
#include <stdlib.h>
#include <GL/glx.h>
#include <X11/keysym.h>
#include <netpbm/ppm.h>

#define ROW	120
#define COL	160

Display *dpy;
Window window;

extern void init(unsigned char *data);
static void make_window (int width, int height, char *name, int border);

int alreadyDeclared = 0;

void init(unsigned char *data)
{
	if(!alreadyDeclared)
	{
		make_window (COL, ROW, "Image Viewer", 1);
		glMatrixMode (GL_PROJECTION);
		glOrtho (0, COL, 0, ROW, -1, 1);
		glPixelStorei (GL_UNPACK_ALIGNMENT, 1);
		glMatrixMode (GL_MODELVIEW);
		glRasterPos2i (0, 0);
		alreadyDeclared = 1;
	}
	glDrawPixels (160, 120, GL_RGB, GL_UNSIGNED_BYTE, data);
	glFlush ();
}

static int attributeList[] = { GLX_RGBA, GLX_RED_SIZE, 1, None };

void noborder(Display *dpy, Window win)
{
	struct
	{
		long flags;
		long functions;
		long decorations;
		long input_mode;
	} *hints;
  
	int fmt;
	unsigned long nitems, byaf;
	Atom type;
	Atom mwmhints = XInternAtom (dpy, "_MOTIF_WM_HINTS", False);

	XGetWindowProperty(dpy, win, mwmhints, 0, 4, False, mwmhints,
			   &type, &fmt, &nitems, &byaf, 
			   (unsigned char**) &hints);
  
	if (!hints) hints = (void *)malloc(sizeof *hints);

	hints->decorations = 0;
	hints->flags |= 2;

	XChangeProperty (dpy, win, mwmhints, mwmhints, 32, PropModeReplace,
			 (unsigned char *)hints, 4);
	XFlush (dpy);
	free (hints);
}

static void make_window(int width, int height, char *name, int border)
{
	XVisualInfo *vi;
	Colormap cmap;
	XSetWindowAttributes swa;
	GLXContext cx;
	XSizeHints sizehints;

	dpy = XOpenDisplay(0);

	if (!(vi = glXChooseVisual (dpy, DefaultScreen(dpy), attributeList)))
	{
		printf("Can't find requested visual.\n");
		exit(1);
	}
	cx = glXCreateContext (dpy, vi, 0, GL_TRUE);

	swa.colormap = XCreateColormap (dpy, RootWindow (dpy, vi->screen),
					vi->visual, AllocNone);
	sizehints.flags = 0;

	swa.border_pixel = 0;
	swa.event_mask = ExposureMask | StructureNotifyMask | KeyPressMask;
	window = XCreateWindow (dpy, RootWindow (dpy, vi->screen), 
				0, 0, width, height,
				0, vi->depth, InputOutput, vi->visual,
				CWBorderPixel|CWColormap|CWEventMask, &swa);
	XMapWindow (dpy, window);
	XSetStandardProperties (dpy, window, name, name,
				None, (void *)0, 0, &sizehints);

	if (!border) noborder (dpy, window);

	glXMakeCurrent (dpy, window, cx);
}

