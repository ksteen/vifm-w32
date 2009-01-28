/* vifm Copyright (C) 2001 Ken Steen.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place - Suite 330, Boston, MA 02111-1307, USA
 */

#include<string.h>
#include<unistd.h>
#include<time.h>
#include<signal.h>
#include<sys/types.h>
#include<sys/stat.h>
//#include<sys/wait.h>
#include<fcntl.h>
#include<windows.h>

#include"background.h" 
#include"menus.h"
#include"status.h"
#include"utils.h"

struct Jobs_List *jobs = NULL;
struct Finished_Jobs *fjobs = NULL;

/*
static void 
add_background_job(int pid, char *cmd, int fd) 
{
	Jobs_List *new;

	new = (Jobs_List *)malloc(sizeof(Jobs_List)); 
	new->pid = pid;
	new->cmd = strdup(cmd);
	new->next = jobs;
	new->fd = fd; 
	new->error_buf = (char *)calloc(1, sizeof(char));
	new->running = 1;
	jobs = new;
}
*/

void 
add_finished_job(int pid, int status)
{
	Finished_Jobs *new;

	new = (Finished_Jobs *)malloc(sizeof(Finished_Jobs)); 
	new->pid = pid;
	new->remove = 0;
	new->next = fjobs;
	fjobs = new;
}

void
check_background_jobs(void)
{
/*
	Jobs_List *p = jobs;
	Jobs_List *prev = 0;
	Finished_Jobs *fj = NULL;
	sigset_t new_mask;
	fd_set ready;
	int maxfd;
	int nread;
	struct timeval ts;


	if (!p)
		return;
*/

	/*
	 * SIGCHLD  needs to be blocked anytime the Finished_Jobs list 
	 * is accessed from anywhere except the received_sigchld().
	 */
/*
	sigemptyset(&new_mask);
	sigaddset(&new_mask, SIGCHLD);
	sigprocmask(SIG_BLOCK, &new_mask, NULL);

	fj = fjobs;

	ts.tv_sec = 0;
	ts.tv_usec = 1000;

	while (p)
	{
*/
		/* Mark any finished jobs */
/*
		while (fj)
		{
			if (p->pid == fj->pid)
			{
				p->running = 0;
				fj->remove = 1;
			}
			fj = fj->next;
		}
*/

		/* Setup pipe for reading */

/*
		FD_ZERO(&ready);
		maxfd = 0;
		FD_SET(p->fd, &ready);
		maxfd = (p->fd > maxfd ? p->fd : maxfd);

		if ((select(maxfd +1, &ready, NULL, NULL, &ts) > 0))
		{
			char buf[256];

			nread = read(p->fd, buf, sizeof(buf) -1);

			if (nread)
			{
				p->error_buf = (char *) realloc(p->error_buf, 
						sizeof(buf));

				strncat(p->error_buf, buf, sizeof(buf) - 1);
			}
			if (strlen(p->error_buf) > 1)
			{
				show_error_msg(" Background Process Error ", p->error_buf);
				my_free(p->error_buf);
				p->error_buf = (char *) calloc(1, sizeof(char));
			}
		}
*/

		/* Remove any finished jobs. */
/*
		if (!p->running)
		{
			Jobs_List *j = p;
			if (prev)
				prev->next = p->next;
			else 
				jobs = p->next;

			p = p->next;
			my_free(j->cmd);

			if (strlen(j->error_buf))
				my_free(j->error_buf);

			my_free(j);
		}
		else
		{
			prev = p;
			p = p->next;
		}
	}
	 
*/
	/* Clean up Finished Jobs list */
/*
	fj = fjobs;
	if (fj)
	{
		Finished_Jobs *prev = 0;
		while (fj)
		{
			if (fj->remove)
			{
				Finished_Jobs *j = fj;

				if (prev)
					prev->next = fj->next;
				else
					fjobs = fj->next;

				fj = fj->next;
				my_free(j);
			}
			else
			{
				prev = fj;
				fj = fj->next;
			}
		}
	}
*/

	/* Unblock SIGCHLD signal */
//	sigprocmask(SIG_UNBLOCK, &new_mask, NULL);
}

/* Only used for deleting and putting of files so that the changes show 
 * up immediately in the file lists.
 */
int
background_and_wait_for_errors(char *cmd)
{
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	int retval = 1;
	memset(&pi, 0, sizeof(pi));
	memset(&pi, 0, sizeof(si));
	si.cb = sizeof(si);

	retval = CreateProcess(NULL, cmd, 0, 0, 0, 0, 0, 0, &si, &pi);
	CloseHandle(pi.hThread);
	WaitForSingleObject(pi.hProcess, 0);
	CloseHandle(pi.hProcess);
	
	if(!retval)
	{
		DWORD dw = GetLastError();
		char buf[256];
		
		snprintf(buf, 256, "error is %d  -",(int) dw);
		show_error_msg("error in delete", buf);

	}


	return 0;

}

int 
start_background_job(char *cmd)
{ 
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	int retval = 1;
	memset(&pi, 0, sizeof(pi));
	memset(&pi, 0, sizeof(si));
	si.cb = sizeof(si);

	retval = CreateProcess(NULL, cmd, 0, 0, 0, 0, 0, 0, &si, &pi);
	CloseHandle(pi.hThread);
	WaitForSingleObject(pi.hProcess, 0);
	CloseHandle(pi.hProcess);
	

	return 0;
}
