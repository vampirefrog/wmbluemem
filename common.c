void make_window()
{
 XSetWindowAttributes  windowattributes;
 XGCValues             gcvalues;
 XpmAttributes         xpmattributes;
 XSizeHints           *sizehints;
 XWMHints             *wmhints;
 XClassHint           *classhint;
 XpmColorSymbol        xpmsymbols[3];
 GC                    tmpgc;
 int                   shapeevent, shapeerror; 

 display = XOpenDisplay(opt_display);
 if(!display)
 {
  printf("Cannot connect to display `%s'.\n", opt_display);
  exit(1);
 }
 screen = DefaultScreen(display);
 rootwindow = RootWindow(display, screen);
 colormap = DefaultColormap(display, screen);
 visual = DefaultVisual(display, screen);
 depth = DefaultDepth(display, screen);
 bgcolor = get_color(opt_bgcolor);
 oncolor = get_color(opt_oncolor);
 offcolor = get_color(opt_offcolor);
 gcvalues.graphics_exposures = False;
 gcvalues.function = GXcopy;
 gcvalues.foreground = bgcolor;
 backgc = XCreateGC(display, rootwindow,
                GCGraphicsExposures | GCFunction | GCForeground,
		&gcvalues);
 gcvalues.foreground = oncolor;
 ongc = XCreateGC(display, rootwindow,
                GCGraphicsExposures | GCFunction | GCForeground,
		&gcvalues);
 gcvalues.foreground = offcolor;
 offgc = XCreateGC(display, rootwindow,
                GCGraphicsExposures | GCFunction | GCForeground,
		&gcvalues);
 xpmattributes.valuemask = XpmColormap | XpmCloseness | XpmColorSymbols;
 xpmattributes.colormap = colormap;
 xpmattributes.closeness = 65536;
 xpmsymbols[0].name = "Background";
 xpmsymbols[0].value = opt_bgcolor;
 xpmsymbols[1].name = "On";
 xpmsymbols[1].value = opt_oncolor;
 xpmsymbols[2].name = "Off";
 xpmsymbols[2].value = opt_offcolor;
 xpmattributes.colorsymbols = xpmsymbols;
 xpmattributes.numsymbols = 3;
 XpmCreatePixmapFromData(display, rootwindow, pixmap_xpm,
                         &pixmap, &pixmask, &xpmattributes);
 XpmCreatePixmapFromData(display, rootwindow, icon_xpm,
                         &iconpixmap, None, NULL);
 buffer = XCreatePixmap(display, rootwindow,
                        WINDOW_WIDTH, WINDOW_HEIGHT, depth);
 XSetClipMask(display, backgc, pixmask);
 XSetClipOrigin(display, backgc, WINDOW_WIDTH, 0);
 XCopyArea(display, pixmap, pixmap, backgc, 
           0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_WIDTH, 0);
 XSetClipMask(display, backgc, None);
 XSetClipOrigin(display, backgc, 0, 0);
 XCopyArea(display, pixmap, pixmap, backgc,
           WINDOW_WIDTH, 0, WINDOW_WIDTH, WINDOW_HEIGHT, 0, 0);
 XCopyArea(display, pixmap, buffer, backgc,
           0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, 0, 0);
 tmpgc = XCreateGC(display, pixmask, 0, NULL);
 XSetForeground(display, tmpgc, 0);
 XFillRectangle(display, pixmask, tmpgc,
                WINDOW_WIDTH, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
 XFillRectangle(display, pixmask, tmpgc,
                0, WINDOW_HEIGHT, WINDOW_WIDTH * 2, WINDOW_HEIGHT);
 XFreeGC(display, tmpgc);
 windowattributes.background_pixmap = pixmap;
 windowattributes.colormap = colormap;
 windowattributes.event_mask = ExposureMask | 
                               KeyPressMask |
                               KeyReleaseMask |
			       ButtonPressMask |
			       ButtonReleaseMask |
                               PointerMotionMask |
			       PropertyChangeMask;
 window = XCreateWindow(display, rootwindow,
                        0, 0, WINDOW_WIDTH, WINDOW_HEIGHT,
			0, depth, InputOutput, visual,
			CWBackPixmap |
			CWColormap |
			CWEventMask,
			&windowattributes);
 iconwindow = XCreateWindow(display, rootwindow,
                            0, 0, WINDOW_WIDTH, WINDOW_HEIGHT,
			    0, depth, InputOutput, visual,
			    CWBackPixmap |
			    CWColormap |
			    CWEventMask,
			    &windowattributes);
 if(XShapeQueryExtension(display, &shapeevent, &shapeerror) && opt_shape)
 {
  XShapeCombineMask(display, window, ShapeBounding,
                    0, 0, pixmask, ShapeSet);
  XShapeCombineMask(display, iconwindow, ShapeBounding,
                    0, 0, pixmask, ShapeSet);
 }
 mapwindow = opt_window ? iconwindow : window;
 XStoreName(display, mapwindow, WINDOW_NAME);
 XSetIconName(display, mapwindow, ICON_NAME);
 sizehints = XAllocSizeHints();
 sizehints->flags = PMinSize | PMaxSize;
 sizehints->min_width = WINDOW_WIDTH;
 sizehints->min_height = WINDOW_HEIGHT;
 sizehints->max_width = WINDOW_WIDTH;
 sizehints->max_height = WINDOW_HEIGHT;
 XSetWMNormalHints(display, mapwindow, sizehints);
 XFree(sizehints);
 wmhints = XAllocWMHints();
 wmhints->flags = InputHint |
                  WindowGroupHint |
		  IconWindowHint |
		  IconPixmapHint |
		  StateHint;
 wmhints->input = True;
 wmhints->window_group = window;
 wmhints->icon_window = iconwindow;
 wmhints->icon_pixmap = iconpixmap;
 wmhints->initial_state = WithdrawnState;
 XSetWMHints(display, window, wmhints);
 XFree(wmhints);
 classhint = XAllocClassHint();
 classhint->res_name = CLASS_NAME;
 classhint->res_class = CLASS_CLASS;
 XSetClassHint(display, mapwindow, classhint);
 XFree(classhint);
 XSetCommand(display, window, argv, argc);
 wm_delete_window = XInternAtom(display, "WM_DELETE_WINDOW", False);
 XSetWMProtocols(display, mapwindow, &wm_delete_window, 1);
 if(opt_window)
 {
  mwmhints.flags = MWM_HINTS_DECORATIONS;
  mwmhints.decorations = 0;
  _motif_wm_hints = XInternAtom(display, "_MOTIF_WM_HINTS", 0);
  XChangeProperty(display, mapwindow,
                  _motif_wm_hints,
                  _motif_wm_hints,
                  32, PropModeReplace,
                  (unsigned char *)&mwmhints, 5);
 }
 XMapRaised(display, mapwindow);
}

void update_window()
{
 XCopyArea(display, buffer, window, backgc,
           0, 0, WINDOW_WIDTH, WINDOW_WIDTH, 0, 0);
 XCopyArea(display, buffer, iconwindow, backgc,
           0, 0, WINDOW_WIDTH, WINDOW_WIDTH, 0, 0);
 XSync(display, False);
}

void process_events()
{
 KeySym keysym;

 while(XPending(display))
 {
  XNextEvent(display, &event);
  switch(event.type)
  {
   case Expose:
    update_window();
    break;
   case KeyPress:
    keysym = XLookupKeysym(&event.xkey, 0);
    switch(keysym)
    { 
     case XK_Escape:
     case XK_Q:
     case XK_q:
      exitloop = 1;
      break;
    }
    break;
   case ButtonPress:
    if(event.xbutton.button == Button3) { if(menu_pop(m) == 0) exitloop = 1; }
    else if(event.xbutton.button == Button1 && opt_window)
    {
     oldx = event.xbutton.x;
     oldy = event.xbutton.y;
    }
    break;
   case MotionNotify:
    if(event.xmotion.state & Button1Mask && opt_window)
     XMoveWindow(display, mapwindow,
                 event.xmotion.x_root - oldx,
                 event.xmotion.y_root - oldy);
    break;
   case ClientMessage:
    if(event.xclient.data.l[0] == wm_delete_window)
     exitloop = 1;
    break;
  }
 }
}
