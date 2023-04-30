/***************************************************************************
 *   crqt-ng                                                               *
 *   Copyright (c) 2022,2023 Aleksey Chernov <valexlin@gmail.com>          *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU General Public License           *
 *   as published by the Free Software Foundation; either version 2        *
 *   of the License, or (at your option) any later version.                *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the Free Software           *
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,            *
 *   MA 02110-1301, USA.                                                   *
 ***************************************************************************/

/*
 * Based on CoolReader code at https://github.com/buggins/coolreader
 * Copyright (C) 2010-2021 by Vadim Lopatin <coolreader.org@gmail.com>
 */

#ifndef APP_PROPS_H
#define APP_PROPS_H

//#define PROP_APP_WINDOW_RECT           "window.rect"
//#define PROP_APP_WINDOW_ROTATE_ANGLE   "window.rotate.angle"
#define PROP_APP_WINDOW_FULLSCREEN       "window.fullscreen"
#define PROP_APP_WINDOW_MINIMIZED        "window.minimized"
#define PROP_APP_WINDOW_MAXIMIZED        "window.maximized"
#define PROP_APP_WINDOW_SHOW_MENU        "window.menu.show"
#define PROP_APP_WINDOW_TOOLBAR_SIZE     "window.toolbar.size"
#define PROP_APP_WINDOW_TOOLBAR_POSITION "window.toolbar.position"
#define PROP_APP_WINDOW_SHOW_STATUSBAR   "window.statusbar.show"
#define PROP_APP_WINDOW_SHOW_SCROLLBAR   "window.scrollbar.show"
#define PROP_APP_WINDOW_STYLE            "window.style"
#define PROP_APP_START_ACTION            "cr3.app.start.action"
#define PROP_APP_CLIPBOARD_AUTOCOPY      "clipboard.autocopy"
#define PROP_APP_SELECTION_COMMAND       "selection.command"
#define PROP_APP_BACKGROUND_IMAGE        "background.image.filename"
#define PROP_APP_LOG_FILENAME            "crengine.log.filename"
#define PROP_APP_LOG_LEVEL               "crengine.log.level"
#define PROP_APP_LOG_AUTOFLUSH           "crengine.log.autoflush"

#endif // APP_PROPS_H
