/*
 *  WMBlueMem - a memory monitor
 *
 *  Copyright (C) 2003 Draghicioiu Mihai Andrei <misuceldestept@go.ro>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/select.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <X11/xpm.h>
#include <X11/extensions/shape.h>

#include "mem.h"
#include "mwm.h"
#include "menu.h"

#include "pixmap.xpm"
#include "icon.xpm"

#define WINDOW_WIDTH 64
#define WINDOW_HEIGHT 64
#define WINDOW_NAME "WMBlueMem"
#define ICON_NAME "wmbluemem"
#define CLASS_NAME "wmbluemem"
#define CLASS_CLASS "WMBlueMem"

#define OPT_DISPLAY NULL
#define OPT_MILISECS 100
#define OPT_BGCOLOR "black"
#define OPT_OFFCOLOR "rgb:00/95/E0"
#define OPT_ONCOLOR "rgb:87/D7/FF"
#define OPT_WINDOW 0
#define OPT_SHAPE 1
#define OPT_BUFFERS 0
#define OPT_CACHE 0

int            argc;
char         **argv;
Display       *display;
int            screen;
Colormap       colormap;
Visual        *visual;
int            depth;
GC             backgc, offgc, ongc;
Window         rootwindow, window, iconwindow, mapwindow;
XEvent         event;
int            exitloop;
Atom           wm_delete_window;
Atom           _motif_wm_hints;
MWMHints       mwmhints;
Pixmap         buffer, pixmap, pixmask, iconpixmap;
long int       bgcolor, offcolor, oncolor;
long long int  oldmem = -1, oldswp = -1;
int            redraw = 1;
struct timeval tv = { 0, 0 };
int            oldx, oldy;
menu_t *       m;

char *opt_display    = OPT_DISPLAY;
int   opt_milisecs   = OPT_MILISECS;
char *opt_bgcolor    = OPT_BGCOLOR;
char *opt_offcolor   = OPT_OFFCOLOR;
char *opt_oncolor    = OPT_ONCOLOR;
int   opt_window     = OPT_WINDOW;
int   opt_shape      = OPT_SHAPE;
int   opt_buffers    = OPT_BUFFERS;
int   opt_cache      = OPT_CACHE;

unsigned long get_color(char *colorname)
{
 XColor color, color2;
 color.pixel = 0;
 XAllocNamedColor(display, colormap, colorname, &color2, &color);
 return color2.pixel;
}

void bad_option(int arg)
{
 fprintf(stderr, "%s needs an argument.\n"
                 "Try `%s --help' for more information.\n",
                 argv[arg-1], argv[0]);
 exit(1);
}

void print_usage()
{
 printf("Usage: %s [option]\n"
	"Options:\n"
	"    -h,  --help           Print out this help and exit\n"
	"    -v,  --version        Print out version number and exit\n"
	"    -d,  --display  <dpy> The X11 display to connect to\n"
	"    -m,  --milisecs <ms>  The number of milisecs between updates\n"
	"    -b,  --bgcolor  <col> The background color\n"
	"    -f,  --offcolor <col> Color for Off leds\n"
	"    -o,  --oncolor  <col> Color for On leds\n"
	"    -w,  --window         Run in a window\n"
	"    -nw, --no-window      Run as a dockapp\n"
	"    -s,  --shape          Use XShape extension\n"
	"    -ns, --no-shape       Don't use XShape extension\n"
	"    -u,  --buffers        Consider the buffers memory\n"
	"    -nu, --no-buffers     Ignore the buffers memory\n"
	"    -c,  --cache          Consider the cache\n"
	"    -nc, --no-cache       Ignore the cache\n",
	argv[0]);
 exit(0);
}

void print_version()
{
 printf(WINDOW_NAME " version " VERSION "\n");
 exit(0);
}

void parse_args()
{
 int n;

 for(n = 1; n < argc; n++)
 {
  if(!strcmp(argv[n], "-h") ||
     !strcmp(argv[n], "--help"))
  {
   print_usage();
  }
  else if(!strcmp(argv[n], "-v") ||
          !strcmp(argv[n], "--version"))
  {
   print_version();
  }
  else if(!strcmp(argv[n], "-d") ||
     !strcmp(argv[n], "--display"))
  {
   if(argc <= ++n)
   {
    bad_option(n);
   }
   opt_display = argv[n];
  }
  else if(!strcmp(argv[n], "-m") ||
          !strcmp(argv[n], "--milisecs"))
  {
   if(argc <= ++n)
   {
    bad_option(n);
   }
   opt_milisecs = strtol(argv[n], NULL, 0);
  }
  else if(!strcmp(argv[n], "-b") ||
          !strcmp(argv[n], "--bgcolor"))
  {
   if(argc <= ++n)
   {
    bad_option(n);
   }
   opt_bgcolor = argv[n];
  }
  else if(!strcmp(argv[n], "-f") ||
          !strcmp(argv[n], "--offcolor"))
  {
   if(argc <= ++n)
   {
    bad_option(n);
   }
   opt_offcolor = argv[n];
  }
  else if(!strcmp(argv[n], "-o") ||
          !strcmp(argv[n], "--oncolor"))
  {
   if(argc <= ++n)
   {
    bad_option(n);
   }
   opt_oncolor = argv[n];
  }
  else if(!strcmp(argv[n], "-w") ||
          !strcmp(argv[n], "--window"))
  {
   opt_window = 1;
  }
  else if(!strcmp(argv[n], "-nw") ||
          !strcmp(argv[n], "--no-window"))
  {
   opt_window = 0;
  }
  else if(!strcmp(argv[n], "-s") ||
          !strcmp(argv[n], "--shape"))
  {
   opt_shape = 1;
  }
  else if(!strcmp(argv[n], "-ns") ||
          !strcmp(argv[n], "--no-shape"))
  {
   opt_shape = 0;
  }
  else if(!strcmp(argv[n], "-u") ||
          !strcmp(argv[n], "--buffers"))
  {
   opt_buffers = 1;
  }
  else if(!strcmp(argv[n], "-nb") ||
          !strcmp(argv[n], "--no-buffers"))
  {
   opt_buffers = 0;
  }

  else if(!strcmp(argv[n], "-c") ||
          !strcmp(argv[n], "--cache"))
  {
   opt_cache = 1;
  }
  else if(!strcmp(argv[n], "-nc") ||
          !strcmp(argv[n], "--no-cache"))
  {
   opt_cache = 0;
  }
  else
  {
   fprintf(stderr, "Bad option. Try `%s --help' for more information.\n", argv[0]);
   exit(1);
  }
 }
}

#include "common.c"

void draw_graph(int x, int y, int width, int height, int val)
{
 XDrawRectangle(display, buffer, offgc, x, y, width - 1, height - 1);
 XFillRectangle(display, buffer, backgc, x + 1, y + 1, width - 2, height - 2);
 if(val > 0)
 {
  XDrawRectangle(display, buffer, ongc, x, y + height - val, width - 1, val - 1);
  if(val >= 3)
   XDrawRectangle(display, buffer, ongc, x + 1, y + height - val + 1,
                  width - 3, val - 3);
  if(val >= 9)
  {
   XFillRectangle(display, buffer, offgc, x + 4, y + height - val + 4, 3, val - 8);
   XFillRectangle(display, buffer, offgc, x + 8, y + height - val + 4, 3, val - 8);
   XFillRectangle(display, buffer, offgc, x + 12, y + height - val + 4, 3, val - 8);
  }
 }
}

void draw_window()
{
 long long int mem, swp;
 int mem_graph, swp_graph;

 mem = mem_used;
 if(!opt_buffers) mem -= mem_buffers;
 if(!opt_cache) mem -= mem_cached;
 swp = swp_used;
 mem_graph = mem * 54 / mem_total;
 swp_graph = (swp_total > 0) ? swp * 54 / swp_total : 0;
 if(mem_graph != oldmem) draw_graph(19, 5, 19, 54, mem_graph);
 if(swp_graph != oldswp) draw_graph(40, 5, 19, 54, swp_graph);
 redraw = 0;
 if(mem_graph != oldmem || swp_graph != oldswp) redraw = 1;
 oldmem = mem_graph;
 oldswp = swp_graph;
}

void proc()
{
 int i;
 fd_set fs;
 int fd = ConnectionNumber(display);

 process_events();
 FD_ZERO(&fs); FD_SET(fd, &fs);
 i = select(fd + 1, &fs, 0, 0, &tv);
 if(i == -1)
 {
  fprintf(stderr, "Error with select(): %s", strerror(errno));
  exit(1);
 }
 if(!i)
 {
  mem_getusage();
  draw_window();
  if(redraw) update_window();
  tv.tv_sec = opt_milisecs / 1000;
  tv.tv_usec = (opt_milisecs % 1000) * 1000;
 }
}

void free_stuff()
{
 XUnmapWindow(display, mapwindow);
 XDestroyWindow(display, iconwindow);
 XDestroyWindow(display, window);
 XFreePixmap(display, buffer);
 XFreePixmap(display, iconpixmap);
 XFreePixmap(display, pixmask);
 XFreePixmap(display, pixmap);
 XFreeGC(display, offgc);
 XFreeGC(display, ongc);
 XFreeGC(display, backgc);
 XCloseDisplay(display);
}

static void set_buffers(menuitem_t *i)
{
 opt_buffers = i->checked;
 tv.tv_sec = 0;
 tv.tv_usec = 0;
}

static void set_cache(menuitem_t *i)
{
 opt_cache = i->checked;
 tv.tv_sec = 0;
 tv.tv_usec = 0;
}

int main(int carg, char **varg)
{
 menuitem_t *i;

 argc = carg;
 argv = varg;
 parse_args();
 make_window();

 menu_init(display);
 m = menu_new();

 i = menu_append(m, "Buffers");
 i->i = -1; i->checked = opt_buffers; i->callback = set_buffers;

 i = menu_append(m, "Cache");
 i->i = -1; i->checked = opt_cache; i->callback = set_cache;

 menu_append(m, "Exit");

 mem_init();
 while(!exitloop)
  proc();
 free_stuff();
 return 0;
}
