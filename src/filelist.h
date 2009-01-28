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

#ifndef __FILELIST_H__
#define __FILELIST_H__


#include "ui.h"

enum
{
	DIRECTORY,
	DEVICE,
	NORMAL,
	EXECUTABLE,
	FIFO,
	UNKNOWN
};

void quick_view_file(FileView * view);
void change_directory(FileView *view, char *directory);
void load_dir_list(FileView *view, int reload);
void draw_dir_list(FileView *view, int top, int pos);
char * get_current_file_name(FileView *view);
void moveto_list_pos(FileView *view, int pos);
int find_file_pos_in_list(FileView *view, char *file);
void get_all_selected_files(FileView *view);
void free_selected_file_array(FileView *view);
void erase_current_line_bar(FileView *view);

#endif
