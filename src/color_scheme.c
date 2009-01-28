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

#include<stdio.h>
#include<string.h>
#include<limits.h>
#include<ctype.h>
#include<curses.h>

#include "color_scheme.h"
#include "config.h"
#include "utils.h"

#define MAX_LEN 1024

void
verify_color_schemes()
{

}

void
load_color_scheme(char *name, char *dir)
{


}

static void
load_default_colors()
{

	snprintf(col_schemes[0].name, PATH_MAX, "Default");
	snprintf(col_schemes[0].dir, PATH_MAX, "/");

	col_schemes[0].color[0].name = MENU_COLOR;
	col_schemes[0].color[0].fg = COLOR_WHITE;
	col_schemes[0].color[0].bg = COLOR_BLACK;

	col_schemes[0].color[1].name = BORDER_COLOR;
	col_schemes[0].color[1].fg = COLOR_BLACK;
	col_schemes[0].color[1].bg = COLOR_WHITE;

	col_schemes[0].color[2].name = WIN_COLOR;
	col_schemes[0].color[2].fg = COLOR_WHITE;
	col_schemes[0].color[2].bg = COLOR_BLACK;

	col_schemes[0].color[3].name = STATUS_BAR_COLOR;
	col_schemes[0].color[3].fg = COLOR_WHITE;
	col_schemes[0].color[3].bg = COLOR_BLACK;

	col_schemes[0].color[4].name = CURR_LINE_COLOR;
	col_schemes[0].color[4].fg = COLOR_WHITE;
	col_schemes[0].color[4].bg = COLOR_BLUE;

	col_schemes[0].color[5].name = DIRECTORY_COLOR;
	col_schemes[0].color[5].fg = COLOR_CYAN;
	col_schemes[0].color[5].bg = COLOR_BLACK;

	col_schemes[0].color[6].name = FIFO_COLOR;
	col_schemes[0].color[6].fg = COLOR_YELLOW;
	col_schemes[0].color[6].bg = COLOR_BLACK;

	col_schemes[0].color[7].name = DEVICE_COLOR;
	col_schemes[0].color[7].fg = COLOR_RED;
	col_schemes[0].color[7].bg = COLOR_BLACK;

	col_schemes[0].color[8].name = EXECUTABLE_COLOR;
	col_schemes[0].color[8].fg = COLOR_GREEN;
	col_schemes[0].color[8].bg = COLOR_BLACK;

	col_schemes[0].color[9].name = SELECTED_COLOR;
	col_schemes[0].color[9].fg = COLOR_MAGENTA;
	col_schemes[0].color[9].bg = COLOR_BLACK;

	col_schemes[0].color[10].name = CURRENT_COLOR;
	col_schemes[0].color[10].fg = COLOR_MAGENTA;
	col_schemes[0].color[10].bg = COLOR_BLACK;


}


/*
 * convert possible <color_name> to <int>
 */
static int
colname2int(char col[])
{
 /* test if col[] is a number... */
	 if (isdigit(col[0]))
	   return atoi(col);

 /* otherwise convert */
 if(!strcmp(col, "black"))
   return COLOR_BLACK;
 if(!strcmp(col, "blue"))
   return COLOR_BLUE;
 if(!strcmp(col, "green"))
   return COLOR_GREEN;
 if(!strcmp(col, "cyan"))
   return COLOR_CYAN;
 if(!strcmp(col, "red"))
   return COLOR_RED;
 if(!strcmp(col, "magenta"))
   return COLOR_MAGENTA;
 if(!strcmp(col, "yellow"))
   return COLOR_YELLOW;
 if(!strcmp(col, "white"))
   return COLOR_WHITE;
 /* return default color */
 return -1;
}

/*
 * add color
 */
void
add_color(char s1[], char s2[], char s3[])
{
 	int fg, bg;
	int scheme = 0;
	int x = cfg.color_scheme_num -1;
	int y = cfg.color_pairs_num;

	fg = colname2int(s2);
	bg = colname2int(s3);

	if(y > 10)
		y =  (y % 11);

	scheme = ((cfg.color_scheme_num - 1) * 11);


	if(!strcmp(s1, "MENU"))
		col_schemes[x].color[y].name = 0 + scheme;

	if(!strcmp(s1, "BORDER"))
		col_schemes[x].color[y].name = 1 + scheme;

	if(!strcmp(s1, "WIN"))
		col_schemes[x].color[y].name = 2 + scheme;

	if(!strcmp(s1, "STATUS_BAR"))
		col_schemes[x].color[y].name = 3 + scheme;

	if(!strcmp(s1, "CURR_LINE"))
		col_schemes[x].color[y].name = 4 + scheme;

	if(!strcmp(s1, "DIRECTORY"))
		col_schemes[x].color[y].name = 5 + scheme;

	if(!strcmp(s1, "FIFO"))
		col_schemes[x].color[y].name = 6 + scheme;

	if(!strcmp(s1, "DEVICE"))
		col_schemes[x].color[y].name = 7 + scheme;

	if(!strcmp(s1, "EXECUTABLE"))
		col_schemes[x].color[y].name = 8 + scheme;

	if(!strcmp(s1, "SELECTED"))
		col_schemes[x].color[y].name = 9 + scheme;

	if(!strcmp(s1, "CURRENT"))
		col_schemes[x].color[y].name = 10 + scheme;

	col_schemes[x].color[y].fg = fg;
	col_schemes[x].color[y].bg = bg;

	cfg.color_pairs_num++;
}


void
read_color_scheme_file()
{

	FILE *fp;
	char config_file[PATH_MAX];
	char line[MAX_LEN];
	char *s1 = NULL;
	char *s2 = NULL;
	char *s3 = NULL;
	char *sx = NULL;
	int args;

	snprintf(config_file, sizeof(config_file), "%s/colorschemes",
				  cfg.config_dir);

	if((fp = fopen(config_file, "r")) == NULL)
	{
		load_default_colors();

		cfg.color_scheme_num++;
		cfg.color_pairs_num = 11;
		return;
	}

	while(fgets(line, MAX_LEN, fp))
	{
		args = 0;

		if(line[0] == '#')
			continue;

		if((sx = s1 = strchr(line, '=')) != NULL)
		{
			s1++;
			chomp(s1);
			*sx = '\0';
			args = 1;
		}
		else
			continue;
		if((sx = s2 = strchr(s1, '=')) != NULL)
		{
			s2++;
			chomp(s2);
			*sx = '\0';
			args = 2;
		}
		if((args == 2) && ((sx = s3 = strchr(s2, '=')) != NULL))
		{
			s3++;
			chomp(s3);
			*sx = '\0';
			args = 3;
		}

		if(args == 1)
		{
			if(!strcmp(line, "COLORSCHEME"))
			{

				//check if last colorscheme is complete and pad it before starting
				// a new scheme
				// verify_scheme();

					//col_schemes = (Col_scheme *)realloc(col_schemes,
					//		sizeof(Col_scheme *) +1);

				snprintf(col_schemes[cfg.color_scheme_num].name,
						PATH_MAX, "%s", s1);

				cfg.color_scheme_num++;

				if (cfg.color_scheme_num > 8)
					break;

				continue;

			}
			if(!strcmp(line, "DIRECTORY"))
			{
				snprintf(col_schemes[cfg.color_scheme_num - 1].dir,
							PATH_MAX, "%s", s1);

				continue;
			}
		}
		if(!strcmp(line, "COLOR") && args == 3)
		{
			add_color(s1, s2, s3);
		}

	}

	fclose(fp);

	//verify_color_schemes();
	return;
}

void
write_color_scheme_file()
{
	FILE *fp;
	char config_file[PATH_MAX];
	int x, y;
	char buf[128];
	char fg_buf[64];
	char bg_buf[64];

	snprintf(config_file, sizeof(config_file), "%s/colorschemes",
		   	cfg.config_dir);

	if((fp = fopen(config_file, "w")) == NULL)
		return;

	fprintf(fp, "# You can edit this file by hand.\n");
	fprintf(fp, "# The # character at the beginning of a line comments out the line.\n");
	fprintf(fp, "# Blank lines are ignored.\n\n");

	fprintf(fp, "# The Default color scheme is used for any directory that does not have\n");
	fprintf(fp, "# a specified scheme.  A color scheme set for a base directory will also\n");
	fprintf(fp, "# be used for the sub directories.\n\n");

	fprintf(fp, "# The standard pdcurses colors are: \n");
	fprintf(fp, "# Black = 0\n");
	fprintf(fp, "# Blue = 1\n");
	fprintf(fp, "# Green = 2\n");
	fprintf(fp, "# Cyan = 3\n");
	fprintf(fp, "# Red = 4\n");
	fprintf(fp, "# Magenta = 5\n");
	fprintf(fp, "# Yellow = 6\n");
	fprintf(fp, "# White = 7\n\n");

	fprintf(fp, "# COLORSCHEME=OneWordDescription\n");
	fprintf(fp, "# DIRECTORY=/Full/Path/To/Base/Directory\n");
	fprintf(fp, "# COLOR=Window_name=foreground_color_number=background_color_number\n\n");


	for(x = 0; x < cfg.color_scheme_num; x++)
	{
		fprintf(fp, "\nCOLORSCHEME=%s\n", col_schemes[x].name);
		fprintf(fp, "DIRECTORY=%s\n", col_schemes[x].dir);

		for(y = 0; y < 11; y++)
		{

			while(col_schemes[x].color[y].name > 10)
			{
				col_schemes[x].color[y].name =
				   	col_schemes[x].color[y].name - 11;
			}

			switch(col_schemes[x].color[y].name)
			{
				case 0:
					snprintf(buf, sizeof(buf), "MENU");
					break;
				case 1:
					snprintf(buf, sizeof(buf), "BORDER");
					break;
				case 2:
					snprintf(buf, sizeof(buf), "WIN");
					break;
				case 3:
					snprintf(buf, sizeof(buf), "STATUS_BAR");
					break;
				case 4:
					snprintf(buf, sizeof(buf), "CURR_LINE");
					break;
				case 5:
					snprintf(buf, sizeof(buf), "DIRECTORY");
					break;
				case 6:
					snprintf(buf, sizeof(buf), "FIFO");
					break;
				case 7:
					snprintf(buf, sizeof(buf), "DEVICE");
					break;
				case 8:
					snprintf(buf, sizeof(buf), "EXECUTABLE");
					break;
				case 9:
					snprintf(buf, sizeof(buf), "SELECTED");
					break;
				case 10:
					snprintf(buf, sizeof(buf), "CURRENT");
					break;
				default:
					snprintf(buf, sizeof(buf), "# Unknown");
					break;
			};

			switch (col_schemes[x].color[y].fg)
			{
				case 0:
					snprintf(fg_buf, sizeof(fg_buf), "black");
					break;
				case 1:
					snprintf(fg_buf, sizeof(fg_buf), "blue");
					break;
				case 2:
					snprintf(fg_buf, sizeof(fg_buf), "green");
					break;
				case 3:
					snprintf(fg_buf, sizeof(fg_buf), "cyan");
					break;
				case 4:
					snprintf(fg_buf, sizeof(fg_buf), "red");
					break;
				case 5:
					snprintf(fg_buf, sizeof(fg_buf), "magenta");
					break;
				case 6:
					snprintf(fg_buf, sizeof(fg_buf), "yellow");
					break;
				case 7:
					snprintf(fg_buf, sizeof(fg_buf), "white");
					break;
				default:
					snprintf(fg_buf, sizeof(fg_buf), "-1");
					break;
			}

			switch (col_schemes[x].color[y].bg)
			{
				case 0:
					snprintf(bg_buf, sizeof(bg_buf), "black");
					break;
				case 1:
					snprintf(bg_buf, sizeof(bg_buf), "blue");
					break;
				case 2:
					snprintf(bg_buf, sizeof(bg_buf), "green");
					break;
				case 3:
					snprintf(bg_buf, sizeof(bg_buf), "cyan");
					break;
				case 4:
					snprintf(bg_buf, sizeof(bg_buf), "red");
					break;
				case 5:
					snprintf(bg_buf, sizeof(bg_buf), "magenta");
					break;
				case 6:
					snprintf(bg_buf, sizeof(bg_buf), "yellow");
					break;
				case 7:
					snprintf(bg_buf, sizeof(bg_buf), "white");
					break;
				default:
					snprintf(bg_buf, sizeof(bg_buf), "-1");
					break;
			}

			fprintf(fp, "COLOR=%s=%s=%s\n", buf, fg_buf, bg_buf);

		}
	}

	fclose(fp);
	return;
}

/* The return value is the color scheme base number for the colorpairs.
 * There are 11 color pairs for each color scheme.
 *
 * Default returns 0;
 * Second color scheme returns 11
 * Third color scheme returns 22
 *
 * The color scheme with the longest matching directory path is the one that 
 * should be returned.
 */
int
check_directory_for_color_scheme(const char *dir)
{
	int x,y;
	int z = 0;
	int v = 0;

	for(x = 0; x < cfg.color_scheme_num;  x++)
	{
		y = strlen(col_schemes[x].dir);

		if(!strncmp(col_schemes[x].dir, dir, y))
		{
			if (y > z)
			{
				z = y;
				v = x;
			}
		}

	}

	return (v * 11);
}

