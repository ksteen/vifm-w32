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

#include<sys/types.h>

typedef struct Jobs_List {
	int fd;
	int pid;
	char *cmd;
	char *error_buf;
	int running;
	struct Jobs_List *next;
} Jobs_List;

typedef struct Finished_Jobs {
	int pid;
	int remove;
	struct Finished_Jobs *next;
} Finished_Jobs;

extern struct Jobs_List *jobs;
extern struct Finished_Jobs *fjobs;

int start_background_job(char *cmd);
int background_and_wait_for_errors(char *cmd);
void add_finished_job(int pid, int status);
void check_background_jobs(void);
void update_jobs_list(void);

