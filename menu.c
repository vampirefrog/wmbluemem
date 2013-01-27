/*
 * menu
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <X11/Xlib.h>

#ifdef USE_XFT
# include <X11/Xft/Xft.h>
#endif

#include "menu.h"

#define DEF_FONT "-*-charter-medium-r-*-*-12-*-*-*-*-*-*-*"

#define DEF_BG "black"
#define DEF_FG "rgb:87/d7/ff"

#define DEF_SEL "rgb:00/95/e0"
#define DEF_SEL_FG "white"

static Display	*	display;
static int			screen;
static int			depth;
static Visual *		visual;
static Colormap		colormap;
static Window		rootwindow;

#ifdef USE_XFT
void fill_color(XftColor *col, char *name)
#else
void fill_color(unsigned long *col, char *name)
#endif
{
 XColor c, c1;

 XLookupColor(display, colormap, name, &c, &c1);
 XAllocColor(display, colormap, &c1);
#ifdef USE_XFT
 col->pixel = c1.pixel;
 col->color.red = c1.red;
 col->color.green = c1.green;
 col->color.blue = c1.blue;
 col->color.alpha = 0xFFFF;
#else
 *col = c1.pixel;
#endif
}

int menu_init(Display *d)
{
 if(!d) return 0;

 display = d;

 screen = DefaultScreen(display);
 rootwindow = RootWindow(display, screen);
 depth = DefaultDepth(display, screen);
 visual = DefaultVisual(display, screen);
 colormap = DefaultColormap(display, screen);

 return 1;
}

menu_t *menu_new()
{
 menu_t *m;
 XSetWindowAttributes a;
 XGCValues v;

 m = malloc(sizeof(menu_t));

 m->first = m->last = NULL;
 m->numitems = 0;

 fill_color(&m->bg, DEF_BG);
 fill_color(&m->fg, DEF_FG);
 fill_color(&m->sel, DEF_SEL);
 fill_color(&m->sel_fg, DEF_SEL_FG);

#ifdef USE_XFT
 a.background_pixel = m->bg.pixel;
#else
 a.background_pixel = m->bg;
#endif
 a.override_redirect = True;
 a.event_mask = ButtonPressMask | ButtonReleaseMask | PointerMotionMask | ExposureMask | EnterWindowMask | LeaveWindowMask;
 m->window = XCreateWindow(display, rootwindow, 0, 0, 128, 128,
 						0, depth, InputOutput, visual,
						CWBackPixel | CWOverrideRedirect | CWEventMask, &a);

#ifdef USE_XFT
 m->font = XftFontOpen(display,  screen,
	XFT_FAMILY,	XftTypeString,	"charter",
	XFT_SIZE,	XftTypeDouble,	9.0,
	NULL);
// m->font = XftFontOpenPattern(display, XftNameParse(DEF_FONT));
 m->draw = XftDrawCreate(display, m->window, visual, colormap);
#else
 m->font = XLoadQueryFont(display, DEF_FONT);
 if(!m->font)
 {
  m->font = XLoadQueryFont(display, "fixed");
  if(!m->font) { fprintf(stderr, "Could not load font\n"); return NULL; }
 }
#endif

 v.function = GXcopy;
#ifdef USE_XFT
 v.foreground = m->fg.pixel;
 v.background = m->bg.pixel;
 m->gc = XCreateGC(display, rootwindow, GCFunction | GCForeground | GCBackground, &v);
#else
 v.foreground = m->fg;
 v.background = m->bg;
 v.font = m->font->fid;
 m->gc = XCreateGC(display, rootwindow, GCFunction | GCForeground | GCBackground | GCFont, &v);
#endif

 return m;
}

menuitem_t *menu_append(menu_t *m, char *string)
{
 if(!m) return NULL;

 if(!m->first)
 {
  m->first = malloc(sizeof(menuitem_t));
  m->first->next = NULL;
  m->first->prev = NULL;
  m->last = m->first;
 }
 else
 {
  m->last->next = malloc(sizeof(menuitem_t));
  m->last->next->next = NULL;
  m->last->next->prev = m->last->next;
  m->last = m->last->next;
 }
 m->last->title = strdup(string);
 m->last->i = 0;
 m->last->callback = NULL;
 m->last->checked = -1;
 m->numitems++;

 return m->last;
}

static void draw_menuitem(menu_t *m, int y, int on, int width)
{
 menuitem_t *i;
 int n = 0;
 int h = m->font->ascent + m->font->descent + 4;
#ifdef USE_XFT
 XftColor *bg, *fg;
#else
 unsigned long bg, fg;
#endif

 if(y < 0) return;

 for(i = m->first; i; i = i->next, n++)
 {
  if(y == n)
  {
   if(on)
   {
#ifdef USE_XFT
    bg = &m->sel; fg = &m->sel_fg;
#else
    bg = m->sel; fg = m->sel_fg;
#endif
   }
   else
   {
#ifdef USE_XFT
    bg = &m->bg; fg = &m->fg;
#else
    bg = m->bg; fg = m->fg;
#endif
   }
#ifdef USE_XFT
   XftDrawRect(m->draw, bg, 0, n * h, width, h);
   XSetForeground(display, m->gc, fg->pixel);
   if(i->checked == 1) XFillRectangle(display, m->window, m->gc, 2, 2 + n * h, h - 4, h - 4);
   else if(i->checked == 0) XDrawRectangle(display, m->window, m->gc, 2, 2 + n * h, h - 5, h - 5);
   XftDrawString8(m->draw, fg, m->font, 2 + h, 2 + n * h + m->font->ascent, (unsigned char *)i->title, strlen(i->title));
#else
   XSetForeground(display, m->gc, bg);
   XFillRectangle(display, m->window, m->gc, 0, n * h, width, h);
   XSetForeground(display, m->gc, fg);
   if(i->checked == 1) XFillRectangle(display, m->window, m->gc, 2, 2 + n * h, h - 4, h - 4);
   else if(i->checked == 0) XDrawRectangle(display, m->window, m->gc, 2, 2 + n * h, h - 5, h - 5);
   XDrawString(display, m->window, m->gc, 2 + h, 2 + n * h + m->font->ascent, i->title, strlen(i->title));
#endif
   return;
  }
 }
}

int menu_pop(menu_t *m)
{
 menuitem_t *i;
 XEvent ev;
 Window root_return, child_return;
 int root_x, root_y, win_x, win_y;
 unsigned int mask_ret;
 int y = -1, y2, w, h, height, width;
 int done = 0;
 XWindowAttributes a;

 if(!m) return 0;
 if(!m->first) return 0;

 XGetWindowAttributes(display, rootwindow, &a);

 width = 0;
 height = 0;
 h = m->font->ascent + m->font->descent + 4;
 for(i = m->first; i; i = i->next)
 {
  height += h;

#ifdef USE_XFT
  XGlyphInfo g;

  XftTextExtents8(display, m->font, (unsigned char *)i->title, strlen(i->title), &g);
  w = g.xOff + 2;
#else
  w = XTextWidth(m->font, i->title, strlen(i->title)) + 2;
#endif
  if(width < w) width = w;
 }

 width += h + 4;

 XQueryPointer(display, rootwindow, &root_return, &child_return,
				&root_x, &root_y, &win_x, &win_y, &mask_ret);

 if(root_x < 0) root_x = 0; if(root_y < 0) root_y = 0;
 if(root_x + width > a.width) root_x -= width + 1; else root_x += 2;
 if(root_y + height > a.height) root_y -= height + 1; else root_y += 2;

 XMoveResizeWindow(display, m->window, root_x, root_y, width, height);
 XMapRaised(display, m->window);
 XGrabPointer(display, m->window, False, PointerMotionMask | ButtonPressMask | ButtonReleaseMask, GrabModeAsync, GrabModeAsync, None, None, CurrentTime);

 while(!done)
 {
  XNextEvent(display, &ev);
  if(ev.xany.window != m->window) continue;
  switch(ev.type)
  {
   case ButtonRelease:
    if(ev.xbutton.x_root < root_x || ev.xbutton.y_root < root_y ||
	   ev.xbutton.x_root >= root_x + width ||
	   ev.xbutton.y_root >= root_y + height) break;
   case ButtonPress:
    done = 1;
	y = (ev.xbutton.y) / h;
	if(y >= m->numitems) y = -1;
    if(ev.xbutton.x < 0 || ev.xbutton.x > width - 1) y = -1;
	if(ev.xbutton.y < 0) y = -1;
//	  printf("pressed. y = %d\n", y);
	break;
   case EnterNotify:
   case LeaveNotify:
   case MotionNotify:
    y2 = y;
    y = ev.xmotion.y / h;
	if(y >= m->numitems) y = -1;
    if(ev.xmotion.x < 0 || ev.xmotion.x > width - 1) y = -1;
	if(ev.xmotion.y < 0) y = -1;
//	printf("y %d  y2 %d\n", y, y2);
	
	if(y2 != y)
	{
	 draw_menuitem(m, y2, 0, width);
	 draw_menuitem(m, y, 1, width);
	}
	break;
   case Expose:
    for(y = 0; y < m->numitems; y++)
     draw_menuitem(m, y, 0, width);
    break;
  }
 }

 XUngrabPointer(display, CurrentTime);
 XUnmapWindow(display, m->window);

 for(i = m->first, y2 = 0; i; i = i->next, y2++)
  if(y == y2)
  {
   if(i->checked >= 0) i->checked = !i->checked;
   if(i->callback) i->callback(i); return i->i;
  }

 return -1;
}

int menu_free(menu_t *m)
{
#ifdef USE_XFT
#else
 XFreeFont(display, m->font);
#endif
 XDestroyWindow(display, m->window);
 return 1;
}
