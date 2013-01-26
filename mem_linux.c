/*
 *  mem_linux.c - get memory status for linux
 *
 *  Copyright (C) 2003 Mihai Draghicioiu <mihai.draghicioiu@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Street #330, Boston, MA 02111-1307, USA.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/utsname.h>

long long int mem_total, mem_used, mem_free, mem_shared, mem_buffers, mem_cached;
long long int swp_total, swp_used, swp_free;

static int isnewformat;

/* Code taken from wmmemmon 0.7.0 */
void mem_init()
{
 struct utsname un;
 int version, patchlevel, sublevel;

 /* get kernel version */
 if(uname(&un) == -1)
  perror("uname()");
 sscanf (un.release, "%d.%d.%d", &version, &patchlevel, &sublevel);

 /* new format ? (kernel >= 2.5.1pre?) */
 /* see linux/fs/proc/proc_misc.c */
 if (version >= 2 && patchlevel >= 5 && sublevel >= 1)
  isnewformat = 1;
}

static void mem_24()
{
 FILE *file;

 file = fopen("/proc/meminfo", "r");
 if(!file)
 {
  perror("/proc/meminfo");
  exit(1);
 }
 while(fgetc(file)!='\n'){}
 fscanf(file, "%*s %Ld %Ld %Ld %Ld %Ld %Ld",
        &mem_total, &mem_used, &mem_free, &mem_shared, &mem_buffers, &mem_cached);
 fscanf(file, "%*s %Ld %Ld %Ld",
        &swp_total, &swp_used, &swp_free);
 fclose(file);
}

static void mem_26()
{
 FILE *file;
 char  buf[80];
 char *p;
 int   i;

 file = fopen("/proc/meminfo", "r");
 if(!file)
 {
  perror("/proc/meminfo");
  exit(1);
 }
 fgets(buf, 80, file); strtok(buf, " "); p = strtok(NULL, " ");
 mem_total = strtoll(p, NULL, 10);
 fgets(buf, 80, file); strtok(buf, " "); p = strtok(NULL, " ");
 mem_free = strtoll(p, NULL, 10);
 fgets(buf, 80, file); strtok(buf, " "); p = strtok(NULL, " ");
 mem_buffers = strtoll(p, NULL, 10);
 fgets(buf, 80, file); strtok(buf, " "); p = strtok(NULL, " ");
 mem_cached = strtoll(p, NULL, 10);
 for(i = 1; i <= 7; i++)
  fgets(buf, 80, file);
 fgets(buf, 80, file); strtok(buf, " "); p = strtok(NULL, " ");
 swp_total = strtoll(p, NULL, 10);
 fgets(buf, 80, file); strtok(buf, " "); p = strtok(NULL, " ");
 swp_free = strtoll(p, NULL, 10);
 fclose(file);
 mem_used = mem_total - mem_free;
 swp_used = swp_total - swp_free;
}

void mem_getusage()
{
 if(isnewformat) mem_26();
 else mem_24();
}
