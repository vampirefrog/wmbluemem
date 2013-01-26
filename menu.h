/*
 * menu.h - simple X11 menu, with Xft support
 */

#ifndef __MENU_H__
#define __MENU_H__

#ifdef USE_XFT
# include <X11/Xft/Xft.h>
#endif

typedef struct menuitem_s
{
 struct menuitem_s *next, *prev;
 char *title;
 int i;
 void (* callback)(struct menuitem_s *);
 int checked; /* 0 off  1 on  -1 disabled */
} menuitem_t;

typedef struct
{
 menuitem_t		*first, *last;
 int			numitems;
 Window			window;
 GC				gc;
#ifdef USE_XFT
 XftFont *		font;
 XftDraw *		draw;
 XftColor		bg, fg;
 XftColor		sel, sel_fg;
#else
 XFontStruct *	font;
 unsigned long	bg, fg;
 unsigned long  sel, sel_fg;
#endif
} menu_t;

int menu_init(Display *display);
menu_t *menu_new(void);
menuitem_t *menu_append(menu_t *menu, char *string);
int menu_pop(menu_t *menu);
int menu_free(menu_t *menu);

#endif /* __MENU_H__ */
