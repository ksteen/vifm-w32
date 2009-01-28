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
#include<signal.h>  /* signal() */
#include<stdlib.h> /* malloc */
#include<sys/stat.h> /* stat */
//#include<dirent.h> /* DIR */
//#include<pwd.h> /* getpwent() */
#include<string.h>
#include<time.h>
#include<windows.h>
//#include<termios.h> /* struct winsize */
//#include<sys/ioctl.h>

#include "color_scheme.h"
#include "config.h" /* for menu colors */
#include "file_info.h"
#include "filelist.h"
#include "keys.h"
#include "menus.h"
#include "status.h"
#include "ui.h"  
#include "utils.h"

static void
finish(char *message)
{
	endwin();
	write_config_file();
	system("clear");
	printf("%s", message);
	exit(0);
}

void
update_pos_window(FileView *view)
{
	char buf[13];

	if(curr_stats.freeze)
		return;
	werase(pos_win);
	snprintf(buf, sizeof(buf), "%d-%d ", view->list_pos +1, view->list_rows);
	mvwaddstr(pos_win, 0, 13 - strlen(buf), buf);
	wnoutrefresh(pos_win);
}

void
write_stat_win(char *message)
{
	werase(stat_win);
	wprintw(stat_win, "%s", message);
	wnoutrefresh(stat_win);
}

void
update_stat_window(FileView *view)
{
	char name_buf[20];
//	char perm_buf[26];
	char size_buf[56];
//	char uid_buf[26];
	//struct passwd *pwd_buf;
	int x, y;

	getmaxyx(stat_win, y, x);
	snprintf(name_buf, sizeof(name_buf), "%s", get_current_file_name(view));
	describe_file_size(size_buf, sizeof(size_buf), view);
	
/*
	if((pwd_buf = getpwuid(view->dir_entry[view->list_pos].uid)) == NULL)
	{
		snprintf (uid_buf, sizeof(uid_buf), "  %d", 
				(int) view->dir_entry[view->list_pos].uid);
	}
	else
	{
		snprintf(uid_buf, sizeof(uid_buf), "  %s", pwd_buf->pw_name);
	}
	get_perm_string(perm_buf, sizeof(perm_buf), 
			view->dir_entry[view->list_pos].mode);
*/
	werase(stat_win);

	mvwaddstr(stat_win, 0, 2, name_buf);
	mvwaddstr(stat_win, 0, 20, size_buf);
//	mvwaddstr(stat_win, 0, 36, perm_buf);
//	mvwaddstr(stat_win, 0, 46, uid_buf);
	snprintf(name_buf, sizeof(name_buf), "%d %s filtered", 
			view->filtered, view->filtered == 1 ? "file" : "files");


	if(view->filtered > 0)
		mvwaddstr(stat_win, 0, x - (strlen(name_buf) +2) , name_buf);  

	wnoutrefresh(stat_win);
	update_pos_window(view);
}

void
status_bar_message(char *message)
{
	werase(status_bar);
	wprintw(status_bar, "%s", message);
	wnoutrefresh(status_bar);
}


int
setup_ncurses_interface()
{
	int screen_x, screen_y;
	int i, x, y;

	initscr();
	noecho();
	nonl();
	raw();

	getmaxyx(stdscr, screen_y, screen_x);
	/* screen is too small to be useful*/
	if(screen_y < 10)
		finish("Terminal is too small to run vifm\n");
	if(screen_x < 30)
		finish("Terminal is too small to run vifm\n");

	if(! has_colors())
		finish("Vifm requires a console that can support color.\n");

	start_color();

	x = 0;

	for (i = 0; i < cfg.color_scheme_num; i++)
	{
		for(x = 0; x < 12; x++)
			init_pair(col_schemes[i].color[x].name,
				col_schemes[i].color[x].fg, col_schemes[i].color[x].bg);
	}
	
	werase(stdscr);

	menu_win = newwin(screen_y - 1, screen_x , 0, 0);
	wbkgdset(menu_win, COLOR_PAIR(WIN_COLOR));
	werase(menu_win);

	sort_win = newwin(14, 30, (screen_y -12)/2, (screen_x -30)/2);
	wbkgdset(sort_win, COLOR_PAIR(WIN_COLOR));
	werase(sort_win);

	change_win = newwin(20, 30, (screen_y -20)/2, (screen_x -30)/2);
	wbkgdset(change_win, COLOR_PAIR(WIN_COLOR));
	werase(change_win);

	error_win = newwin(10, screen_x -2, (screen_y -10)/2, 1);
	wbkgdset(error_win, COLOR_PAIR(WIN_COLOR));
	werase(error_win);

	lborder = newwin(screen_y - 2, 1, 0, 0);

	wbkgdset(lborder, COLOR_PAIR(BORDER_COLOR));

	werase(lborder);

	if (curr_stats.number_of_windows == 1)
		lwin.title = newwin(0, screen_x -2, 0, 1);
	else
		lwin.title = newwin(0, screen_x/2 -1, 0, 1);
		
	wattrset(lwin.title, A_BOLD);
	wbkgdset(lwin.title, COLOR_PAIR(BORDER_COLOR));

	werase(lwin.title);

	if (curr_stats.number_of_windows == 1)
		lwin.win = newwin(screen_y - 3, screen_x -2, 1, 1);
	else
		lwin.win = newwin(screen_y - 3, screen_x/2 -2, 1, 1);

	keypad(lwin.win, TRUE);
	wbkgdset(lwin.win, COLOR_PAIR(WIN_COLOR));
	wattrset(lwin.win, A_BOLD);
	wattron(lwin.win, A_BOLD);
	werase(lwin.win);
	getmaxyx(lwin.win, y, x);
	lwin.window_rows = y -1;
	lwin.window_width = x -1;

	mborder = newwin(screen_y, 2, 0, screen_x/2 -1);

	wbkgdset(mborder, COLOR_PAIR(BORDER_COLOR));

	werase(mborder);

	if (curr_stats.number_of_windows == 1)
		rwin.title = newwin(0, screen_x -2  , 0, 1);
	else
		rwin.title = newwin(1, screen_x/2 -1  , 0, screen_x/2 +1);

	wbkgdset(rwin.title, COLOR_PAIR(BORDER_COLOR));
	wattrset(rwin.title, A_BOLD);
	wattroff(rwin.title, A_BOLD);

	werase(rwin.title);

	if (curr_stats.number_of_windows == 1)
		rwin.win = newwin(screen_y - 3, screen_x -2 , 1, 1);
	else
		rwin.win = newwin(screen_y - 3, screen_x/2 -2 , 1, screen_x/2 +1);

	keypad(rwin.win, TRUE);
	wattrset(rwin.win, A_BOLD);
	wattron(rwin.win, A_BOLD);
	wbkgdset(rwin.win, COLOR_PAIR(WIN_COLOR));
	werase(rwin.win);
	getmaxyx(rwin.win, y, x);
	rwin.window_rows = y - 1;
	rwin.window_width = x -1;

	rborder = newwin(screen_y - 2, 1, 0, screen_x -1);

	wbkgdset(rborder, COLOR_PAIR(BORDER_COLOR));

	werase(rborder);

	stat_win = newwin(1, screen_x, screen_y -2, 0);

	wbkgdset(stat_win, COLOR_PAIR(BORDER_COLOR));

	werase(stat_win);

	status_bar = newwin(1, screen_x - 19, screen_y -1, 0);
	keypad(status_bar, TRUE);
	wattrset(status_bar, A_BOLD);
	wattron(status_bar, A_BOLD);
	wbkgdset(status_bar, COLOR_PAIR(STATUS_BAR_COLOR));
	werase(status_bar);

	pos_win = newwin(1, 13, screen_y - 1, screen_x -13);
	wattrset(pos_win, A_BOLD);
	wattron(pos_win, A_BOLD);
	wbkgdset(pos_win, COLOR_PAIR(STATUS_BAR_COLOR));
	werase(pos_win);

	num_win = newwin(1, 6, screen_y - 1, screen_x -19);
	wattrset(num_win, A_BOLD);
	wattron(num_win, A_BOLD);
	wbkgdset(num_win, COLOR_PAIR(STATUS_BAR_COLOR));
	werase(num_win);


	wnoutrefresh(lwin.title);
	wnoutrefresh(lwin.win);
	wnoutrefresh(rwin.win);
	wnoutrefresh(rwin.title);
	wnoutrefresh(stat_win);
	wnoutrefresh(status_bar);
	wnoutrefresh(pos_win);
	wnoutrefresh(num_win);
	wnoutrefresh(lborder);
	wnoutrefresh(mborder);
	wnoutrefresh(rborder);

	return 1;
}

void
redraw_window(void)
{
	int color_scheme = 0;

	endwin();

	resize_term(0, 0);

	
	//setup_ncurses_interface();
	color_scheme = check_directory_for_color_scheme(lwin.curr_dir);
	wbkgdset(lwin.title, COLOR_PAIR(BORDER_COLOR + color_scheme));
	wbkgdset(lwin.win, COLOR_PAIR(WIN_COLOR + color_scheme));

	color_scheme = check_directory_for_color_scheme(rwin.curr_dir);
	wbkgdset(rwin.title, COLOR_PAIR(BORDER_COLOR + color_scheme));
	wbkgdset(rwin.win, COLOR_PAIR(WIN_COLOR + color_scheme));

	curs_set(0);

	doupdate();

	load_dir_list(other_view, 0);
	load_dir_list(curr_view, 0);

	curr_stats.freeze = 0;
	curr_stats.need_redraw = 0;

}


