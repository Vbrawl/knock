/*
 *  list.h
 * 
 *  Copyright (c) 2002-2005 by Judd Vinet <jvinet@zeroflux.org>
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
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#ifndef _CONFIG_H
#define _CONFIG_H

#include <stddef.h>
#include <time.h>
#include <stdio.h>
#include <limits.h>
#include "list.h"

#define SEQ_TIMEOUT 25 /* default knock timeout in seconds */
#define CMD_TIMEOUT 10 /* default timeout in seconds between start and stop commands */
#define SEQ_MAX     32 /* maximum number of ports in a knock sequence */

extern PMList *doors;
extern int    o_usesyslog;
extern int    o_verbose;
extern int    o_debug;
extern int    o_daemon;
extern int    o_lookup;
extern int    o_skipIpV6;
extern char   o_int[32];
extern char   o_cfg[PATH_MAX];
extern char   o_pidfile[PATH_MAX];
extern char   o_logfile[PATH_MAX];

int parseconfig(char *configfile, int is_service_config);

#endif