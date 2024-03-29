/*
  Simple DirectMedia Layer
  Copyright (C) 1997-2012 Sam Lantinga <slouken@libsdl.org>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/
#include "SDL_config.h"

#if SDL_VIDEO_DRIVER_BWINDOW
#include "../SDL_sysvideo.h"

#include "SDL_BWin.h"
#include <new>

/* Define a path to window's BWIN data */
#ifdef __cplusplus
extern "C" {
#endif

static inline SDL_BWin *_ToBeWin(SDL_Window *window) {
	return ((SDL_BWin*)(window->driverdata));
}

static inline SDL_BApp *_GetBeApp() {
	return ((SDL_BApp*)be_app);
}

static int _InitWindow(_THIS, SDL_Window *window) {
	uint32 flags = 0;
	BRect bounds(
        window->x,
        window->y,
        window->x + window->w - 1,	//BeWindows have an off-by-one px w/h thing
        window->y + window->h - 1
    );
    
    if(window->flags & SDL_WINDOW_FULLSCREEN) {
    	/* TODO: Add support for this flag */
    	printf(__FILE__": %d!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n",__LINE__);
    }
    if(window->flags & SDL_WINDOW_OPENGL) {
    	/* TODO: Add support for this flag */
    }
    if(!(window->flags & SDL_WINDOW_RESIZABLE)) {
    	flags |= B_NOT_RESIZABLE | B_NOT_ZOOMABLE;
    }
    if(window->flags & SDL_WINDOW_BORDERLESS) {
    	/* TODO: Add support for this flag */
    }

    SDL_BWin *bwin = new(std::nothrow) SDL_BWin(bounds, flags);
    if(bwin == NULL)
    	return ENOMEM;

    window->driverdata = bwin;
    int32 winID = _GetBeApp()->GetID(window);
    bwin->SetID(winID);

    return 0;
}

int BE_CreateWindow(_THIS, SDL_Window *window) {
	if(_InitWindow(_this, window) == ENOMEM)
		return ENOMEM;
	
	/* Start window loop */
    _ToBeWin(window)->Show();
    return 0;
}

int BE_CreateWindowFrom(_THIS, SDL_Window * window, const void *data) {

	SDL_BWin *otherBWin = (SDL_BWin*)data;
	if(!otherBWin->LockLooper())
		return -1;
	
	/* Create the new window and initialize its members */
	window->x = (int)otherBWin->Frame().left;
	window->y = (int)otherBWin->Frame().top;
	window->w = (int)otherBWin->Frame().Width();
	window->h = (int)otherBWin->Frame().Height();
	
	/* Set SDL flags */
	if(!(otherBWin->Flags() & B_NOT_RESIZABLE)) {
		window->flags |= SDL_WINDOW_RESIZABLE;
	}
	
	/* If we are out of memory, return the error code */
	if(_InitWindow(_this, window) == ENOMEM)
		return ENOMEM;
	
	/* TODO: Add any other SDL-supported window attributes here */
    _ToBeWin(window)->SetTitle(otherBWin->Title());
    
    /* Start window loop and unlock the other window */
    _ToBeWin(window)->Show();
    
    otherBWin->UnlockLooper();
    return 0;
}

void BE_SetWindowTitle(_THIS, SDL_Window * window) {
	BMessage msg(BWIN_SET_TITLE);
	msg.AddString("window-title", window->title);
	_ToBeWin(window)->PostMessage(&msg);
}

void BE_SetWindowIcon(_THIS, SDL_Window * window, SDL_Surface * icon) {
	/* FIXME: Icons not supported by BeOs/Haiku */
}

void BE_SetWindowPosition(_THIS, SDL_Window * window) {
	BMessage msg(BWIN_MOVE_WINDOW);
	msg.AddInt32("window-x", window->x);
	msg.AddInt32("window-y", window->y);
	_ToBeWin(window)->PostMessage(&msg);
}

void BE_SetWindowSize(_THIS, SDL_Window * window) {
	BMessage msg(BWIN_RESIZE_WINDOW);
	msg.AddInt32("window-w", window->w - 1);
	msg.AddInt32("window-h", window->h - 1);
	_ToBeWin(window)->PostMessage(&msg);
}

void BE_ShowWindow(_THIS, SDL_Window * window) {
	BMessage msg(BWIN_SHOW_WINDOW);
	_ToBeWin(window)->PostMessage(&msg);
}

void BE_HideWindow(_THIS, SDL_Window * window) {
	BMessage msg(BWIN_HIDE_WINDOW);
	_ToBeWin(window)->PostMessage(&msg);
}

void BE_RaiseWindow(_THIS, SDL_Window * window) {
	BMessage msg(BWIN_SHOW_WINDOW);	/* Activate this window and move to front */
	_ToBeWin(window)->PostMessage(&msg);
}

void BE_MaximizeWindow(_THIS, SDL_Window * window) {
	BMessage msg(BWIN_MAXIMIZE_WINDOW);
	_ToBeWin(window)->PostMessage(&msg);
}

void BE_MinimizeWindow(_THIS, SDL_Window * window) {
	BMessage msg(BWIN_MINIMIZE_WINDOW);
	_ToBeWin(window)->PostMessage(&msg);
}

void BE_RestoreWindow(_THIS, SDL_Window * window) {
	BMessage msg(BWIN_RESTORE_WINDOW);
	_ToBeWin(window)->PostMessage(&msg);
}

void BE_SetWindowFullscreen(_THIS, SDL_Window * window,
		SDL_VideoDisplay * display, SDL_bool fullscreen) {
	/* Haiku tracks all video display information */
	BMessage msg(BWIN_FULLSCREEN);
	msg.AddBool("fullscreen", fullscreen);
	_ToBeWin(window)->PostMessage(&msg);
	
}

int BE_SetWindowGammaRamp(_THIS, SDL_Window * window, const Uint16 * ramp) {
	/* FIXME: Not BeOs/Haiku supported */
	return -1;
}

int BE_GetWindowGammaRamp(_THIS, SDL_Window * window, Uint16 * ramp) {
	/* FIXME: Not BeOs/Haiku supported */
	return -1;
}


void BE_SetWindowGrab(_THIS, SDL_Window * window) {
	/* TODO: Implement this! */
}

void BE_DestroyWindow(_THIS, SDL_Window * window) {
	_ToBeWin(window)->LockLooper();	/* This MUST be locked */
	_GetBeApp()->ClearID(_ToBeWin(window));
	_ToBeWin(window)->Quit();
	window->driverdata = NULL;
}

SDL_bool BE_GetWindowWMInfo(_THIS, SDL_Window * window,
                                    struct SDL_SysWMinfo *info) {
	/* FIXME: What is the point of this? What information should be included? */
	return SDL_FALSE;
}




 
#ifdef __cplusplus
}
#endif

#endif /* SDL_VIDEO_DRIVER_BWINDOW */
