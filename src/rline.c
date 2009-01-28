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

#include<ctype.h>
#include<sys/types.h> /* Needed for Fink with Mac OS X */
#include<dirent.h> /* DIR */
#include<curses.h>
#include<signal.h>
#include<string.h>

#include"commands.h"
#include"config.h"
#include"keys.h"
#include"menus.h"
#include"status.h"
#include"utils.h"

static char *
check_for_command(char *string)
{
	int pos = -1;
	char *command = (char *)NULL;

	if ((pos = command_is_reserved(string)) > -1)
		command = strdup(reserved_commands[pos]);

	else if ((pos = is_user_command(string)) > -1)
		command = strdup(command_list[pos].name);

	return command;
}

static char *
check_for_executable(char *string)
{
	char *temp = (char *)NULL;

	if (!string)
		return NULL;

	if (string[0] == '!')
	{
		if (strlen(string) > 2)
		{
			if (string[1] == '!')
				temp = strdup(string + 2);
			else
				temp = strdup(string + 1);
		}
		else if (strlen(string) > 1)
			temp = strdup(string + 1);
	}
	return temp;
}

static char *
get_last_word(char * string)
{

	char * temp = (char *)NULL;

	if (!string)
		return NULL;
		
	/*:command filename */
	temp = strrchr(string, ' ');

	if(temp)
	{
		temp++;
		return temp;
	}
 /* :!filename or :!!filename */
	temp = check_for_executable(string);
	if (temp)
		return temp;

	return NULL;
}

/* This is from the taken mainly from the readline program it just
 * returns the first file name in the directory that matches 
 * the string passed to the function.  I couldn't get readline to handle 
 * the Ctrl-c or Esc keys correctly so we are doing this the hard way.
 */
static char *
filename_completion(char *string)
{

	DIR *dir;
	int x = 0;
	struct dirent *d;
	char * dirname = (char *)NULL;
	char * filename = (char *)NULL;
	char * temp =  (char *)NULL;
	int found = 0;
	int filename_len = 0;

	if (!string)
		return NULL;

	if (!strncmp(string, "~/", 2))
	{
		char * homedir = getenv("HOME");

		dirname = (char *)malloc((strlen(homedir) + strlen(string) + 1));

		if (dirname == NULL)
			return NULL;

		snprintf(dirname, strlen(homedir) + strlen(string) + 1, "%s/%s",
				homedir, string + 2);

		filename = strdup(dirname);
	}
	else
	{
		dirname = strdup(string);
		filename = strdup(string);
	}

	temp = strrchr(dirname, '/');

	if (temp)
	{
		strcpy(filename, ++temp);
		*temp = '\0';
	}
	else
	{
		dirname[0] = '.';
		dirname[1] = '\0';
	}


	dir = opendir(dirname);
	filename_len = strlen(filename);

	if(dir == NULL)
	{
		my_free(filename);
		my_free(dirname);
		return NULL;
	}

	for (x = 0; (d = readdir(dir)); x++)
	{
		if (!strncmp(d->d_name, filename, filename_len))
		{
			found++;
			break;
		}
	}
	
	closedir(dir);


	if (found)
	{
		int isdir = 0;
		if (is_dir(d->d_name))
		{
			isdir = 1;
		}
		else if (strcmp(dirname, "."))
		{
			char * tempfile = (char *)NULL;
			int len = strlen(dirname) + strlen(d->d_name) + 1;
			tempfile = (char *)malloc((len) * sizeof(char));
			if (!tempfile)
				return NULL;
			snprintf(tempfile, len, "%s%s", dirname, d->d_name);
			if (is_dir(tempfile))
				isdir = 1;
			else
				temp = strdup(d->d_name);

			my_free(tempfile);
		}
		else
			temp = strdup(d->d_name);

		if (isdir)
		{
			char * tempfile = (char *)NULL;
			tempfile = (char *) malloc((strlen(d->d_name) + 2) * sizeof(char));
			if (!tempfile)
				return NULL;
			snprintf(tempfile, strlen(d->d_name) + 2, "%s/", d->d_name);
			temp = strdup(tempfile);

			my_free(tempfile);
		}

		my_free(filename);
		my_free(dirname);

		return temp;
	}
	else
	{
		my_free(filename);
		my_free(dirname);
		return NULL;
	}
}

char *
my_rl_gets(int type)
{
	static char *line_read = (char *)NULL;
	int key;
	int pos;
	int index = 0;
	int done = 0;
	int abort = 0;
	int len = 0;
	int cmd_pos = -1;
	int prompt_len;
	char prompt[8];

/* If the buffer has already been allocated, free the memory. */
	if (line_read)
	{
		my_free(line_read);
		line_read = (char *)NULL;
	}

	if (type == GET_VISUAL_COMMAND)
		prompt_len = pos = 6;
	else
		prompt_len = pos = 1;

	curs_set(1);
	werase(status_bar);

	if (type == GET_COMMAND || type == MENU_COMMAND)
		snprintf(prompt, sizeof(prompt), ":");
	else if (type == GET_SEARCH_PATTERN || type == MENU_SEARCH)
		snprintf(prompt, sizeof(prompt), "/");
	/*
	else if (type == MAPPED_COMMAND || type == MAPPED_SEARCH)
	{
		snprintf(buf, sizeof(buf), "%s", string);
		mvwaddstr(status_bar, 0, 0, string);
		pos = strlen(string);
		index = pos -1;
		len = index;
	}
	*/
	else if (type == GET_VISUAL_COMMAND)
		snprintf(prompt, sizeof(prompt), ":'<,'>");

	mvwaddstr(status_bar, 0, 0, prompt);

  while (!done)
  {
	  if(curr_stats.freeze)
		  continue;

	  curs_set(1);
		flushinp();
		curr_stats.getting_input = 1;
		key = wgetch(status_bar);

		switch (key)
		{
			case 27: /* ascii Escape */
			case 3: /* ascii Ctrl C */
				done = 1;
				abort = 1;
				break;
			case 13: /* ascii Return */
				done = 1;
				break;
			case 9: /* ascii Tab */
				{
					char *last_word = (char *)NULL;
					char *raw_name = (char *)NULL;
					char *filename = (char *)NULL;

					if (type == MENU_SEARCH || type == MENU_COMMAND)
						break;

					if(! line_read)
						break;

					last_word = get_last_word(line_read);

					if (last_word)
					{
						raw_name = filename_completion(last_word);
						if (raw_name)
							filename = escape_filename(raw_name, 1);
					}
					/* :partial_command */
					else
					{
						char *complete_command = check_for_command(line_read);

						if (complete_command)
						{
							line_read = strdup(complete_command);

							mvwaddstr(status_bar, 0, prompt_len, line_read);
							pos = index = strlen(line_read);
							pos++;

							my_free(complete_command);
							break;
						}
						else
							break;
					}

					if (filename)
					{
						char *temp = (char *)NULL;

						/* :command /some/directory/partial_filename */
						if ((temp = strrchr(line_read, '/')))
						{
							temp++;
							*temp = '\0';

							line_read = (char *)realloc(line_read, (strlen(line_read) +
										strlen(filename) + 1) * sizeof(char));

							if (!line_read)
							{
								my_free(raw_name);
								my_free(filename);
								break;
							}

							strcat(line_read, filename);
							my_free(filename);
							my_free(raw_name);

						}
						/* :command partial_filename */
						else if ((temp = strrchr(line_read, ' ')))
						{
							temp++;
							*temp = '\0';

							line_read = (char *)realloc(line_read, (strlen(line_read) + 
										strlen(filename) + 1) * sizeof(char));
							if (!line_read)
							{
								my_free(filename);
								break;
							}

							strcat(line_read, filename);
							my_free(raw_name);
							my_free(filename);
						}
						/* :!partial_filename or :!!partial_filename */
						else if ((temp = strrchr(line_read, '!')))
						{
							temp++;
							*temp = '\0';

							line_read = (char *)realloc(line_read, (strlen(line_read) + 
										strlen(filename) + 1) * sizeof(char));

							if (!line_read)
							{
								my_free(raw_name);
								my_free(filename);
								break;
							}

							strcat(line_read, filename);
							my_free(raw_name);
							my_free(filename);
						}
						/* error */
						else
						{
							show_error_msg(" Debug Error ",
									"Harmless error in rline.c line 388");
							my_free(raw_name);
							my_free(filename);
							break;
						}

						mvwaddstr(status_bar, 0, prompt_len, line_read);
						pos = index = strlen(line_read);
						pos++;
					}
					else
						break;
				}
				break;
			case KEY_UP:
				{
					if (type == GET_COMMAND)
					{
						if (0 > cfg.cmd_history_num)
							break;

						cmd_pos++;

						if(cmd_pos > cfg.cmd_history_num)
							cmd_pos = 0;

						werase(status_bar);
						mvwaddch(status_bar, 0, 0, ':');
						mvwaddstr(status_bar, 0, 1, cfg.cmd_history[cmd_pos]);
						pos = strlen(cfg.cmd_history[cmd_pos]) + 1;
						line_read = (char *)realloc(line_read,
								strlen(cfg.cmd_history[cmd_pos]) + 1);
						strcpy(line_read, cfg.cmd_history[cmd_pos]);
						index = strlen(line_read);

						if (cmd_pos >= cfg.cmd_history_len - 1)
							cmd_pos = cfg.cmd_history_len - 1;
					}
					else if (type == GET_SEARCH_PATTERN)
					{
						if (0 > cfg.search_history_num)
							break;

						cmd_pos++;

						if(cmd_pos > cfg.search_history_num)
							cmd_pos = 0;

						werase(status_bar);
						mvwaddch(status_bar, 0, 0, '/');
						mvwaddstr(status_bar, 0, 1, cfg.search_history[cmd_pos]);
						pos = strlen(cfg.search_history[cmd_pos]) + 1;
						line_read = (char *)realloc(line_read,
								strlen(cfg.search_history[cmd_pos]) + 1);
						strcpy(line_read, cfg.search_history[cmd_pos]);
						index = strlen(line_read);

						if (cmd_pos >= cfg.search_history_len - 1)
							cmd_pos = cfg.search_history_len - 1;
					}
				}
				break;
			case KEY_DOWN:
				{
					if(type == GET_COMMAND)
					{
						if (0 > cfg.cmd_history_num)
							break;

						cmd_pos--;

						if (cmd_pos < 0)
							cmd_pos = cfg.cmd_history_num;

						werase(status_bar);
						mvwaddch(status_bar, 0, 0, ':');
						mvwaddstr(status_bar, 0, 1, cfg.cmd_history[cmd_pos]);
						pos = strlen(cfg.cmd_history[cmd_pos]) + 1;

						line_read = (char *)realloc(line_read,
								strlen(cfg.cmd_history[cmd_pos]) + 1);
						strcpy(line_read, cfg.cmd_history[cmd_pos]);
						index = strlen(line_read);

					}
					else if (type == GET_SEARCH_PATTERN)
					{
						if (0 > cfg.search_history_num)
							break;

						cmd_pos--;

						if (cmd_pos < 0)
							cmd_pos = cfg.search_history_num;

						werase(status_bar);
						mvwaddch(status_bar, 0, 0, '/');
						mvwaddstr(status_bar, 0, 1, cfg.search_history[cmd_pos]);
						pos = strlen(cfg.search_history[cmd_pos]) + 1;

						line_read = (char *)realloc(line_read,
								strlen(cfg.search_history[cmd_pos]) + 1);
						strcpy(line_read, cfg.search_history[cmd_pos]);
						index = strlen(line_read);

					}
				}
				break;
			/* This needs to be changed to a value that is read from 
			 * the termcap file.
			 */
			case 127: /* ascii Delete  */
			case 8: /* ascii Backspace  ascii Ctrl H */
			case KEY_BACKSPACE: /* ncurses BACKSPACE KEY */
				{
					pos--;
					index--;
					len--;
					if (type == GET_VISUAL_COMMAND)
					{
						if (pos < 6)
							pos = 6;
						if (index < 0)
							index = 0;
					}
					else
					{
						if (pos < 1)
							pos = 1;
						if (index < 0)
							index = 0;
					}
					mvwdelch(status_bar, 0, pos);
					if (line_read)
						line_read[index] = '\0';
				}
				break;
			default:
				if (key > 31 && key < 127) 
				{
					line_read = (char *)realloc(line_read, (index + 2) * (sizeof(char)));

					if (line_read == NULL)
						return NULL;

					mvwaddch(status_bar, 0, pos, key);
					line_read[index] = key;
					index++;
					line_read[index] = '\0';
					if (index > 62)
					{
						abort = 1;
						done = 1;
					}
					pos++;
					len++;
				}
				break;
		}
		curr_stats.getting_input = 0;
  }
	curs_set(0);
	werase(status_bar);
	wnoutrefresh(status_bar);

	if (abort)
		return NULL;

	/*
	if (line_read && * line_read)
		add_to_command_history(type, line_read);
		*/

	return line_read;
}

