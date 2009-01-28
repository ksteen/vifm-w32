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


#include<curses.h>
//#include<unistd.h>
#include<errno.h> /* errno */
//#include<sys/wait.h> /* waitpid() */
#include<sys/types.h> /* waitpid() */
#include<sys/stat.h> /* stat */
#include<stdio.h>
#include<string.h>

#include<windows.h>
#include<process.h>

#include"background.h"
#include"color_scheme.h"
#include"commands.h"
#include"config.h"
#include"filelist.h"
#include"fileops.h"
#include"filetype.h"
#include"keys.h"
#include"menus.h"
#include"registers.h"
#include"status.h"
#include"ui.h"
#include"utils.h"



void
move_files(FileView *view)
{
	char buf[PATH_MAX];
	char newbuf[PATH_MAX];
	int x;
	int retval = 1;
	int answer = 0;

	if(!view->selected_files)
	{
		view->dir_entry[view->list_pos].selected = 1;
		view->selected_files = 1;
	}

	get_all_selected_files(view);
	yank_selected_files(view);

	for(x = 0; x < view->selected_files; x++)
	{
		if(!strcmp("../", view->selected_filelist[x]))
		{
			show_error_msg(" Background Process Error ", 
					"You cannot move the ../ directory ");
			continue;
		}
		snprintf(buf, sizeof(buf), "%s\\%s", view->curr_dir,
				view->selected_filelist[x]);
		if(view == curr_view)
			snprintf(newbuf, sizeof(newbuf), "%s\\%s", other_view->curr_dir,
				view->selected_filelist[x]);
		else
			snprintf(newbuf, sizeof(newbuf), "%s\\%s", curr_view->curr_dir,
				view->selected_filelist[x]);

		retval = MoveFile(buf, newbuf);
		if(!retval)
		{
			DWORD dw = GetLastError();

			if((int)dw == 183)
			{
				if (answer != 1)
					answer = show_overwrite_file_menu(view->selected_filelist[x]);
				if (answer != 2)
					retval = MoveFileEx(buf, newbuf, MOVEFILE_REPLACE_EXISTING);

			}
			else
				show_error_msg(" Error in moving file ", 
					"Error in moving file. ");
		}

	//	background_and_wait_for_errors(buf);
	}
	free_selected_file_array(view);
	view->selected_files = 0;

}

void
copy_files(FileView *view)
{
	char buf[PATH_MAX];
	char newbuf[PATH_MAX];
	int x;
	int retval = 1;
	int answer = 0;

	if(!view->selected_files)
	{
		view->dir_entry[view->list_pos].selected = 1;
		view->selected_files = 1;
	}

	get_all_selected_files(view);
	yank_selected_files(view);

	for(x = 0; x < view->selected_files; x++)
	{
		if(!strcmp("../", view->selected_filelist[x]))
		{
			show_error_msg(" Background Process Error ", 
					"You cannot copy the ../ directory ");
			continue;
		}
		snprintf(buf, sizeof(buf), "%s\\%s", view->curr_dir,
				view->selected_filelist[x]);
		if(view == curr_view)
			snprintf(newbuf, sizeof(newbuf), "%s\\%s", other_view->curr_dir,
				view->selected_filelist[x]);
		else
			snprintf(newbuf, sizeof(newbuf), "%s\\%s", curr_view->curr_dir,
				view->selected_filelist[x]);

		retval = CopyFile(buf, newbuf, TRUE);
		if(!retval)
		{
			DWORD dw = GetLastError();

			if((int)dw == 80)
			{
				if (answer != 1)
					answer = show_overwrite_file_menu(view->selected_filelist[x]);
				if (answer != 2)
					retval = CopyFile(buf, newbuf, FALSE);

			}
			else
			{
				snprintf(buf, sizeof(buf), "-%d- is error code ", (int)dw);
				show_error_msg(" Error in copying file ", buf);
			}
		}
	}
	free_selected_file_array(view);
	view->selected_files = 0;

}

int 
my_system(char *command)
{

	return 0;
}

static int
execute(char **args)
{
/*
	int pid;

	if((pid = fork()) == 0)
	{
		setsid();
		close(0);
		execvp(args[0], args);
		exit(127);
	}

	return pid;
*/
	return 0;
}

void
yank_selected_files(FileView *view)
{
	int x;
	size_t namelen;
	int old_list = curr_stats.num_yanked_files;



	if(curr_stats.yanked_files)
	{
		for(x = 0; x < old_list; x++)
		{
			if(curr_stats.yanked_files[x])
			{
				my_free(curr_stats.yanked_files[x]);
				curr_stats.yanked_files[x] = NULL;
			}
		}
		my_free(curr_stats.yanked_files);
		curr_stats.yanked_files = NULL;
	}

	curr_stats.yanked_files = (char **)calloc(view->selected_files, 
			sizeof(char *));

	if ((curr_stats.use_register) && (curr_stats.register_saved))
	{
		/* A - Z  append to register otherwise replace */
		if ((curr_stats.curr_register < 65) || (curr_stats.curr_register > 90))
			clear_register(curr_stats.curr_register);
		else
			curr_stats.curr_register = curr_stats.curr_register + 32;
	}

	for(x = 0; x < view->selected_files; x++)
	{
		if(view->selected_filelist[x])
		{
			char buf[PATH_MAX];
			namelen = strlen(view->selected_filelist[x]);
			curr_stats.yanked_files[x] = malloc(namelen +1);
			strcpy(curr_stats.yanked_files[x], view->selected_filelist[x]);
			snprintf(buf, sizeof(buf), "%s/%s", view->curr_dir, 
					view->selected_filelist[x]);
			append_to_register(curr_stats.curr_register, view->selected_filelist[x]);
		}
		else
		{
			x--;
			break;
		}
	}
	curr_stats.num_yanked_files = x;

	strncpy(curr_stats.yanked_files_dir, view->curr_dir,
			sizeof(curr_stats.yanked_files_dir) -1);
}

/* execute command. */
int
file_exec(char *command)
{
	char *args[4];
	pid_t pid;

	args[0] = "sh";
	args[1] = "-c";
	args[2] = command;
	args[3] = NULL;

	pid = execute(args);
	return pid;
}

void
view_file(FileView *view)
{
	char command[PATH_MAX + 5] = "";
	char *filename = escape_filename(get_current_file_name(view), 0);

	snprintf(command, sizeof(command), "%s %s", cfg.vi_command, filename);

	shellout(command, 0);
	my_free(filename);
	curs_set(0);
}

void
handle_file(FileView *view)
{
	if(DIRECTORY == view->dir_entry[view->list_pos].type)
	{
		char *filename = get_current_file_name(view);
		change_directory(view, filename);
		load_dir_list(view, 0);
		moveto_list_pos(view, view->curr_line);
		return;
	}

	if(cfg.vim_filter)
	{
		FILE *fp;
		char filename[PATH_MAX] = "";

		snprintf(filename, sizeof(filename), "%s\\vimfiles", cfg.config_dir);
		fp = fopen(filename, "w");
		snprintf(filename, sizeof(filename), "%s\\%s",
				view->curr_dir,
				view->dir_entry[view->list_pos].name);
		
		endwin();
		fprintf(fp, "%s", filename);
		fclose(fp);
		exit(0);
	}
/*

	if(FILE_ATTRIBUTE_EXECUTABLE == view->dir_entry[view->list_pos].type)
	{
		if(cfg.auto_execute)
		{
			char buf[PATH_MAX];
			snprintf(buf, sizeof(buf), "./%s", get_current_file_name(view));
			shellout(buf, 1);
			return;
		}
		else // Check for a filetype 
		{
			char *program = NULL;

			if((program = get_default_program_for_file(
						view->dir_entry[view->list_pos].name)) != NULL)
			{
				if(strchr(program, '%'))
				{
					int m = 0;
					int s = 0;
					char *command = expand_macros(view, program, NULL, &m, &s);
					shellout(command, 0);
					my_free(command);
					return;
				}
				else
				{
					char buf[PATH_MAX *2];
					char *temp = escape_filename(view->dir_entry[view->list_pos].name, 0);

					snprintf(buf, sizeof(buf), "%s %s", program, temp); 
					shellout(buf, 0);
					my_free(program);
					my_free(temp);
					return;
				}
			} else // vi is set as the default for any extension without a program 
			{
				view_file(view);
			}
			return;
		}
	}
	*/
	if(NORMAL == view->dir_entry[view->list_pos].type)
	{
		char *program = NULL;

		if((program = get_default_program_for_file(
					view->dir_entry[view->list_pos].name)) != NULL)
		{
			if(strchr(program, '%'))
			{
				int s = 0;
				char *command = expand_macros(view, program, NULL, &s);
				shellout(command, 0);
				my_free(command);
				return;
			}
			else
			{
				char buf[PATH_MAX];
				char *temp = escape_filename(view->dir_entry[view->list_pos].name, 0);

				snprintf(buf, sizeof(buf), "%s %s", program, temp); 
				shellout(buf, 0);
				my_free(program);
				my_free(temp);
				return;
			}
		}
		// use Windows default program for file extension

		else if (ShellExecute(NULL, "open", view->dir_entry[view->list_pos].name, NULL, NULL, SW_SHOW) > 32)
			return;
				
		else /* vi is set as the default for any extension without a program */
		view_file(view);

	}
/*
	if(FILE_ATTRIBUTE_REPARSE_POINT == view->dir_entry[view->list_pos].type)
	{
		char linkto[PATH_MAX];
		int len;
		char *filename = strdup(view->dir_entry[view->list_pos].name);
		len = strlen(filename);
		if (filename[len - 1] == '/')
			filename[len - 1] = '\0';

		len = readlink (filename, linkto, sizeof (linkto));

		if (len > 0)
		{
			struct stat s;
			int is_dir = 0;
			int is_file = 0;
			char *dir = NULL;
			char *file = NULL;
			char *link_dup = strdup(linkto);
			linkto[len] = '\0';
			lstat(linkto, &s);
			
			if((s.st_mode & S_IFMT) == S_IFDIR)
			{
				is_dir = 1;
				dir = strdup(linkto);
			}
			else
			{
				int x;
				for(x = strlen(linkto); x > 0; x--)
				{
					if(linkto[x] == '/')
					{
						linkto[x] = '\0';
						lstat(linkto, &s);
						if((s.st_mode & S_IFMT) == S_IFDIR)
						{
							is_dir = 1;
							dir = strdup(linkto);
							break;
						}
					}
				}
				if((file = strrchr(link_dup, '/')))
				{
					file++;
					is_file = 1;
				}
			}
			if(is_dir)
			{
				change_directory(view, dir);
				load_dir_list(view, 0);

				if(is_file)
				{
					int pos = find_file_pos_in_list(view, file);
					if(pos >= 0)
						moveto_list_pos(view, pos);

				}
				else
				{
					moveto_list_pos(view, 0);
				}
			}
			else
			{
				int pos = find_file_pos_in_list(view, link_dup);
				if(pos >= 0)
					moveto_list_pos(view, pos);
			}
			my_free(link_dup);
		}
	  	else
			status_bar_message("Couldn't Resolve Link");
	}
*/
}


int
pipe_and_capture_errors(char *command)
{
/*
  int file_pipes[2];
  int pid;
	int nread;
	int error = 0;
  char *args[4];

  if (pipe (file_pipes) != 0)
      return 1;

  if ((pid = fork ()) == -1)
      return 1;

  if (pid == 0)
    {
			close(1);
			close(2);
			dup(file_pipes[1]);
      close (file_pipes[0]);
      close (file_pipes[1]);

      args[0] = "sh";
      args[1] = "-c";
      args[2] = command;
      args[3] = NULL;
      execvp (args[0], args);
      exit (127);
    }
  else
    {
			char buf[1024];
      close (file_pipes[1]);
			while((nread = read(*file_pipes, buf, sizeof(buf) -1)) > 0)
			{
				buf[nread] = '\0';
				error = nread;
			}
			if(error > 1)
			{
				char title[strlen(command) +4];
				snprintf(title, sizeof(title), " %s ", command);
				show_error_msg(title, buf);
				return 1;
			}
    }
*/
	return 0;
}


void
delete_file(FileView *view)
{
	char buf[PATH_MAX];
	char newbuf[PATH_MAX];
	int x;
	int retval = 1;
	int answer = 0;

	if(!view->selected_files)
	{
		view->dir_entry[view->list_pos].selected = 1;
		view->selected_files = 1;
	}

	get_all_selected_files(view);
	yank_selected_files(view);
	strncpy(curr_stats.yanked_files_dir, cfg.trash_dir,
			sizeof(curr_stats.yanked_files_dir) -1);

	for(x = 0; x < view->selected_files; x++)
	{
		if(!strcmp("../", view->selected_filelist[x]))
		{
			show_error_msg(" Background Process Error ", 
					"You cannot delete the ../ directory ");
			continue;
		}
		snprintf(buf, sizeof(buf), "%s\\%s", view->curr_dir,
				view->selected_filelist[x]);
		snprintf(newbuf, sizeof(newbuf), "%s\\%s", cfg.trash_dir,
				view->selected_filelist[x]);
		retval = MoveFile(buf, newbuf);
		if(!retval)
		{
			DWORD dw = GetLastError();

			if((int)dw == 183)
			{
				if (answer != 1)
					answer = show_overwrite_file_menu(view->selected_filelist[x]);
				if (answer != 2)
					retval = MoveFileEx(buf, newbuf, MOVEFILE_REPLACE_EXISTING);

			}
			else
				show_error_msg("Error in deleting file ", 
					"Error in deleting file. ");
		}

	//	background_and_wait_for_errors(buf);
	}
	free_selected_file_array(view);
	view->selected_files = 0;

	load_dir_list(view, 1);

	moveto_list_pos(view, view->list_pos);
}

void
file_chmod(FileView *view, char *path, char *mode, int recurse_dirs)
{
  char cmd[PATH_MAX + 128] = " ";

	if (recurse_dirs)
		snprintf(cmd, sizeof(cmd), "chmod -R %s %s", mode, path);
	else
		snprintf(cmd, sizeof(cmd), "chmod %s %s", mode, path);

	start_background_job(cmd);

	load_dir_list(view, 1);
	moveto_list_pos(view, view->list_pos);
  
}

static void
reset_change_window(void)
{
	curs_set(0);
	werase(change_win);
	update_all_windows();
	if(curr_stats.need_redraw)
		redraw_window();
}

void
change_file_owner(char *file)
{

}

void
change_file_group(char *file)
{

}

void
set_perm_string(FileView *view, int *perms, char *file)
{
	int i = 0;
	char *add_perm[] = {"u+r", "u+w", "u+x", "u+s", "g+r", "g+w", "g+x", "g+s",
											"o+r", "o+w", "o+x", "o+t"}; 
	char *sub_perm[] = { "u-r", "u-w", "u-x", "u-s", "g-r", "g-w", "g-x", "g-s",
											"o-r", "o-w", "o-x", "o-t"}; 
	char perm_string[64] = " ";

	for (i = 0; i < 12; i++)
	{
		if (perms[i])
			strcat(perm_string, add_perm[i]);
		else
			strcat(perm_string, sub_perm[i]);

		strcat(perm_string, ",");
	}
	perm_string[strlen(perm_string) - 1] = '\0'; /* Remove last , */

	file_chmod(view, file, perm_string, perms[12]);
}

static void
permissions_key_cb(FileView *view, int *perms, int isdir)
{
	int done = 0;
	int abort = 0;
	int top = 3;
	int bottom = 16;
	int curr = 3;
	int permnum = 0;
	int step = 1;
	int col = 9;
	char filename[PATH_MAX];
	char path[PATH_MAX];
	int changed = 0;

	if (isdir)
		bottom = 17;

	snprintf(filename, sizeof(filename), "%s", 
			view->dir_entry[view->list_pos].name);
	snprintf(path, sizeof(path), "%s/%s", view->curr_dir, 
			view->dir_entry[view->list_pos].name);

	curs_set(1);
	wmove(change_win, curr, col);
	wrefresh(change_win);

	while(!done)
	{
		int key = wgetch(change_win);

		switch(key)
		{
			case 'j':
				{
					curr+= step;
					permnum++;

					if(curr > bottom)
					{
						curr-= step;
						permnum--;
					}
					if (curr == 7 || curr == 12)
						curr++;

					wmove(change_win, curr, col);
					wrefresh(change_win);
				}
				break;
			case 'k':
				{
					curr-= step;
					permnum--;
					if(curr < top)
					{
						curr+= step;
						permnum++;
					}

					if (curr == 7 || curr == 12)
						curr--;

					wmove(change_win, curr, col);
					wrefresh(change_win);
				}
				break;
			case 't':
			case 32: /* ascii Spacebar */
				{
					changed++;
					if (perms[permnum])
					{
						perms[permnum] = 0;
						mvwaddch(change_win, curr, col, ' ');
					}
					else
					{
						perms[permnum] = 1;
						mvwaddch(change_win, curr, col, '*');
					}

					wmove(change_win, curr, col);
					wrefresh(change_win);
				}
				break;
			case 3: /* ascii Ctrl C */
			case 27: /* ascii Escape */
				done = 1;
				abort = 1;
				break;
			case 'l': 
			case 13: /* ascii Return */
				done = 1;
				break;
			default:
				break;
		}
	}

	reset_change_window();

	curs_set(0);

	if (abort)
	{
		moveto_list_pos(view, find_file_pos_in_list(view, filename));
		return;
	}

	if (changed)
	{
		set_perm_string(view, perms, path);
		load_dir_list(view, 1);
		moveto_list_pos(view, view->curr_line);
	}

}

static void
change_key_cb(FileView *view, int type)
{
	int done = 0;
	int abort = 0;
	int top = 2;
	int bottom = 8;
	int curr = 2;
	int step = 2;
	int col = 6;
	char filename[PATH_MAX];

	snprintf(filename, sizeof(filename), "%s", 
			view->dir_entry[view->list_pos].name);

	curs_set(0);
	wmove(change_win, curr, col);
	wrefresh(change_win);

	while(!done)
	{
		int key = wgetch(change_win);

		switch(key)
		{
			case 'j':
				{
					mvwaddch(change_win, curr, col, ' ');
					curr+= step;

					if(curr > bottom)
						curr-= step;

					mvwaddch(change_win, curr, col, '*');
					wmove(change_win, curr, col);
					wrefresh(change_win);
				}
				break;
			case 'k':
				{

					mvwaddch(change_win, curr, col, ' ');
					curr-= step;
					if(curr < top)
						curr+= step;

					mvwaddch(change_win, curr, col, '*');
					wmove(change_win, curr, col);
					wrefresh(change_win);
				}
				break;
			case 3: /* ascii Ctrl C */
			case 27: /* ascii Escape */
				done = 1;
				abort = 1;
				break;
			case 'l': 
			case 13: /* ascii Return */
				done = 1;
				break;
			default:
				break;
		}
	}

	reset_change_window();

	if(abort)
	{
		moveto_list_pos(view, find_file_pos_in_list(view, filename));
		return;
	}

	switch(type)
	{
		case FILE_CHANGE:
		{
			if (curr == FILE_NAME)
				rename_file(view);
			else
				show_change_window(view, curr);
			/*
			char * filename = get_current_file_name(view);
			switch(curr)
			{
				case FILE_NAME: 
					rename_file(view);
					break;
				case FILE_OWNER:
					change_file_owner(filename);
					break;
				case FILE_GROUP:
					change_file_group(filename);
					break;
				case FILE_PERMISSIONS:
					show_change_window(view, type);
					break;
				default:
					break;
			}
			*/
		}
		break;
		case FILE_NAME:
			break;
		case FILE_OWNER:
			break;
		case FILE_GROUP:
			break;
		case FILE_PERMISSIONS:
			break;
		default:
			break;
	}
}

void
show_file_permissions_menu(FileView *view, int x)
{
	//mode_t mode = view->dir_entry[view->list_pos].mode;
	char *filename = get_current_file_name(view);
	int perms[] = {0,0,0,0,0,0,0,0,0,0,0,0,0};
	int isdir = 0;

	if (strlen(filename) > x - 2)
		filename[x - 4] = '\0';

	mvwaddnstr(change_win, 1, (x - strlen(filename))/2, filename, x - 2);

	mvwaddstr(change_win, 3, 2, "Owner [ ] Read");
/*
	if (mode & S_IRUSR)
	{
		perms[0] = 1;
		mvwaddch(change_win, 3, 9, '*');
	}
	mvwaddstr(change_win, 4, 6, "  [ ] Write");

	if (mode & S_IWUSR)
	{
		perms[1] = 1;
		mvwaddch(change_win, 4, 9, '*');
	}
	mvwaddstr(change_win, 5, 6, "  [ ] Execute");

	if (mode & S_IXUSR)
	{
		perms[2] = 1;
		mvwaddch(change_win, 5, 9, '*');
	}

	mvwaddstr(change_win, 6, 6, "  [ ] SetUID");
	if (mode & S_ISUID)
	{
		perms[3] = 1;
		mvwaddch(change_win, 6, 9, '*');
	}

	mvwaddstr(change_win, 8, 2, "Group [ ] Read");
	if (mode & S_IRGRP)
	{
		perms[4] = 1;
		mvwaddch(change_win, 8, 9, '*');
	}

	mvwaddstr(change_win, 9, 6, "  [ ] Write");
	if (mode & S_IWGRP)
	{
		perms[5] = 1;
		mvwaddch(change_win, 9, 9, '*');
	}

	mvwaddstr(change_win, 10, 6, "  [ ] Execute");
	if (mode & S_IXGRP)
	{
		perms[6] = 1;
		mvwaddch(change_win, 10, 9, '*');
	}

	mvwaddstr(change_win, 11, 6, "  [ ] SetGID");
	if (mode & S_ISGID)
	{
		perms[7] = 1;
		mvwaddch(change_win, 11, 9, '*');
	}

	mvwaddstr(change_win, 13, 2, "Other [ ] Read");
	if (mode & S_IROTH)
	{
		perms[8] = 1;
		mvwaddch(change_win, 13, 9, '*');
	}

	mvwaddstr(change_win, 14, 6, "  [ ] Write");
	if (mode & S_IWOTH)
	{
		perms[9] = 1;
		mvwaddch(change_win, 14, 9, '*');
	}

	mvwaddstr(change_win, 15, 6, "  [ ] Execute");
	if (mode & S_IXOTH)
	{
		perms[10] = 1;
		mvwaddch(change_win, 15, 9, '*');
	}

	mvwaddstr(change_win, 16, 6, "  [ ] Sticky");
	if (mode & S_ISVTX)
	{
		perms[11] = 1;
		mvwaddch(change_win, 16, 9, '*');
	}

	if (is_dir(filename))
	{
		mvwaddstr(change_win, 17, 6, "  [ ] Set Recursively");
		isdir = 1;
	}
*/

	permissions_key_cb(view, perms, isdir);
}


void
show_change_window(FileView *view, int type)
{
	int x, y;

	wattroff(view->win, COLOR_PAIR(CURR_LINE_COLOR) | A_BOLD);
	curs_set(0);
	doupdate();
	wclear(change_win);

	getmaxyx(stdscr, y, x);
	mvwin(change_win, (y - 20)/2, (x - 30)/2);
	box(change_win, ACS_VLINE, ACS_HLINE);

	curs_set(1);
	wrefresh(change_win);


	switch(type)
	{
		case FILE_CHANGE:
		{
			mvwaddstr(change_win, 0, (x - 20)/2, " Change Current File ");
			mvwaddstr(change_win, 2, 4, " [ ] Name");
			mvwaddstr(change_win, 4, 4, " [ ] Owner");
			mvwaddstr(change_win, 6, 4, " [ ] Group");
			mvwaddstr(change_win, 8, 4, " [ ] Permissions");
			mvwaddch(change_win, 2, 6, '*');
			change_key_cb(view, type);
		}
			break;
		case FILE_NAME: 
			return;
			break;
		case FILE_OWNER:
			return;
			break;
		case FILE_GROUP:
			return;
			break;
		case FILE_PERMISSIONS:
			show_file_permissions_menu(view, x);
			break;
		default:
			break;
	}
}
