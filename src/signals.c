/* vifm
 * Copyright (C) 2001 Ken Steen.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */

#define _WIN32_WINNT 0x0500
#include<windows.h>

#include "menus.h"


void
check_messages()
{
	MSG msg;
	HWND console_win = GetConsoleWindow();


	if(console_win == NULL)
		show_error_msg("Console win", "console window handle error ");

	while(PeekMessage(&msg, console_win, 0, 0, PM_NOREMOVE))
	{
		switch(msg.message)
		{
			case WM_SIZE:
				show_error_msg("size changed", "size changed");
				break;
			case WM_CLOSE:
				show_error_msg("close", "close");
				break;
			case WM_DESTROY:
				show_error_msg("destroy", "destroy");
				break;
			default:
				show_error_msg("error", "message received ");
				return;
		}
	}

}
