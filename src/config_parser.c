#include "config_parser.h"
#include <string.h>
#include "dprint.h"
#include <netinet/in.h>
#include "door.h"
#include <stdlib.h>
#include <ctype.h>
#include "sequences.h"
#include "utils.h"
#include <dirent.h>

PMList *doors = NULL;
int  o_usesyslog = 0;
int  o_verbose   = 0;
int  o_debug     = 0;
int  o_daemon    = 0;
int  o_lookup    = 0;
int  o_skipIpV6  = 0;
char o_int[32]           = "";		/* default (eth0) is set after parseconfig() */
char o_cfg[PATH_MAX]     = "/etc/knockd.conf";
char o_pidfile[PATH_MAX] = "/var/run/knockd.pid";
char o_logfile[PATH_MAX] = "";

int parseconfig_options(char *key, char* ptr, char* include_dir, int *include_dir_specified, int linenum) {
	if(!strcmp(key, "LOGFILE")) {
		strncpy(o_logfile, ptr, PATH_MAX-1);
		o_logfile[PATH_MAX-1] = '\0';
		dprint("config: log file: %s\n", o_logfile);
	} else if(!strcmp(key, "PIDFILE")) {
		strncpy(o_pidfile, ptr, PATH_MAX-1);
		o_pidfile[PATH_MAX-1] = '\0';
		dprint("config: pid file: %s\n", o_pidfile);
	} else if(!strcmp(key, "INTERFACE")) {
		/* set interface only if it has not already been set by the -i switch */
		if(strlen(o_int) == 0) {
			strncpy(o_int, ptr, sizeof(o_int)-1);
			o_int[sizeof(o_int)-1] = '\0';
			dprint("config: interface: %s\n", o_int);
		}
	} else if(!strcmp(key, "INCLUDE_DIR")) {
		strncpy(include_dir, ptr, PATH_MAX-1);
		include_dir[PATH_MAX-1] = '\0';
		*include_dir_specified = 1;
		dprint("config: include directory: %s\n", include_dir);
	} else {
		fprintf(stderr, "config: line %d: syntax error\n", linenum);
		return(1);
	}

	return(0);
}

int parseconfig_sequence(char *ptr, opendoor_t *door) {
	int i = parse_port_sequence(ptr, door);
	if(i > 0) {
		return(i);
	}
	dprint_sequence(door, "config: %s: sequence: ", door->name);
	return(0);
}

int parseconfig_door(char *key, char *ptr, opendoor_t *door, int linenum) {
	if(!strcmp(key, "TARGET")) {
		door->target = malloc(sizeof(char) * (strlen(ptr)+1));
		if(door->target == NULL) {
			perror("malloc");
			exit(1);
		}
		strcpy(door->target, ptr);
		dprint("config: %s: target: %s\n", door->name, door->target);
	} else if(!strcmp(key, "SEQUENCE")) {
		int i = parseconfig_sequence(ptr, door);
		if(i != 0) {
			return(i);
		}
	} else if(!strcmp(key, "SEQUENCE_FILE")) {
		FILE *sfp = fopen(ptr, "r");
		if(sfp == NULL) {
			perror(ptr);
			return(1);
		}
		char buf[PATH_MAX] = "";
		dprint("config: %s: sequence file: %s\n", door->name, ptr);
		size_t sfp_read = fread(buf, sizeof(char), PATH_MAX - 1, sfp);
		fclose(sfp);
		if(sfp_read == 0) {
			fprintf(stderr, "config: line %d: sequence file empty: %s", linenum, ptr);
			return(1);
		}
		buf[sfp_read] = '\0';
		int i = parseconfig_sequence(buf, door);
		if(i != 0) {
			return(i);
		}
	} else if(!strcmp(key, "ONE_TIME_SEQUENCES")) {
		if((door->one_time_sequences_fd = fopen(ptr, "r+")) == NULL) {
			perror(ptr);
			return(1);
		}
		dprint("config: %s: one time sequences file: %s\n", door->name, ptr);
		if(get_new_one_time_sequence(door) == 0) {
			dprint_sequence(door, "config: %s: sequence: ", door->name);
		} else {	/* no more sequences left in the one time sequences file */
			dprint("config: no more sequences left in the one time sequences file %s\n", ptr);
			return(1);
		}
	} else if(!strcmp(key, "SEQ_TIMEOUT") || !strcmp(key, "TIMEOUT")) {
		door->seq_timeout = (time_t)atoi(ptr);
		dprint("config: %s: seq_timeout: %d\n", door->name, door->seq_timeout);
	} else if(!strcmp(key, "START_COMMAND") || !strcmp(key, "COMMAND")) {
		door->start_command = malloc(sizeof(char) * (strlen(ptr)+1));
		if(door->start_command == NULL) {
			perror("malloc");
			exit(1);
		}
		strcpy(door->start_command, ptr);
		dprint("config: %s: start_command: %s\n", door->name, door->start_command);
	} else if(!strcmp(key, "START_COMMAND_6") || !strcmp(key, "COMMAND_6")) {
		door->start_command6 = malloc(sizeof(char) * (strlen(ptr)+1));
		if(door->start_command6 == NULL) {
			perror("malloc");
			exit(1);
		}
		strcpy(door->start_command6, ptr);
		dprint("config: %s: start_command_6: %s\n", door->name, door->start_command6);
	} else if(!strcmp(key, "CMD_TIMEOUT")) {
		door->cmd_timeout = (time_t)atoi(ptr);
		dprint("config: %s: cmd_timeout: %d\n", door->name, door->cmd_timeout);
	} else if(!strcmp(key, "STOP_COMMAND")) {
		door->stop_command = malloc(sizeof(char) * (strlen(ptr)+1));
		if(door->stop_command == NULL) {
			perror("malloc");
			exit(1);
		}
		strcpy(door->stop_command, ptr);
		dprint("config: %s: stop_command: %s\n", door->name, door->stop_command);
	} else if(!strcmp(key, "STOP_COMMAND_6")) {
		door->stop_command6 = malloc(sizeof(char) * (strlen(ptr)+1));
		if(door->stop_command6 == NULL) {
			perror("malloc");
			exit(1);
		}
		strcpy(door->stop_command6, ptr);
		dprint("config: %s: stop_command_6: %s\n", door->name, door->stop_command6);
	} else if(!strcmp(key, "TCPFLAGS")) {
		char *flag;
		strtoupper(ptr);
		while((flag = strsep(&ptr, ","))) {
			/* allow just some flags to be specified */
			if(!strcmp(flag,"FIN")) {
				door->flag_fin = SET;
			} else if(!strcmp(flag,"!FIN")) {
				door->flag_fin = NOT_SET;
			} else if(!strcmp(flag, "SYN")) {
				door->flag_syn = SET;
			} else if(!strcmp(flag, "!SYN")) {
				door->flag_syn = NOT_SET;
			} else if(!strcmp(flag, "RST")) {
				door->flag_rst = SET;
			} else if(!strcmp(flag, "!RST")) {
				door->flag_rst = NOT_SET;
			} else if(!strcmp(flag, "PSH")) {
				door->flag_psh = SET;
			} else if(!strcmp(flag, "!PSH")) {
				door->flag_psh = NOT_SET;
			} else if(!strcmp(flag, "ACK")) {
				door->flag_ack = SET;
			} else if(!strcmp(flag, "!ACK")) {
				door->flag_ack = NOT_SET;
			} else if(!strcmp(flag, "URG")) {
				door->flag_urg = SET;
			} else if(!strcmp(flag, "!URG")) {
				door->flag_urg = NOT_SET;
			} else {
				fprintf(stderr, "config: line %d: unrecognized flag \"%s\"\n",
						linenum, flag);
				return(1);
			}
			dprint("config: tcp flag: %s\n", flag);
		}
	} else {
		fprintf(stderr, "config: line %d: syntax error\n", linenum);
		return(1);
	}

	return(0);
}

/* Parse a config file
 */
int parseconfig(char *configfile, int is_service_config)
{
	FILE *fp = NULL;
	char line[PATH_MAX+1];
	char *ptr = NULL;
	char *key = NULL;
	int linenum = 0;
	char section[256] = "";
	char include_dir[PATH_MAX] = "";
	int include_dir_specified = 0;
	opendoor_t *door = NULL;
	PMList *lp;

	if((fp = fopen(configfile, "r")) == NULL) {
		perror(configfile);
		return(1);
	}

	while(fgets(line, PATH_MAX, fp)) {
		linenum++;
		trim(line);
		if(strlen(line) == 0 || line[0] == '#') {
			continue;
		}
		if(line[0] == '[' && line[strlen(line)-1] == ']') {
			/* new config section */
			ptr = line;
			ptr++;
			strncpy(section, ptr, sizeof(section));
			section[sizeof(section)-1] = '\0';
			section[strlen(section)-1] = '\0';
			dprint("config: new section: '%s'\n", section);
			if(!strlen(section)) {
				fprintf(stderr, "config: line %d: bad section name\n", linenum);
				return(1);
			}
			if(strcmp(section, "options")) {
				/* start a new knock/event record */
				door = malloc(sizeof(opendoor_t));
				if(door == NULL) {
					perror("malloc");
					exit(1);
				}
				init_door(door, section);
				doors = list_add(doors, door);
			}
			else if(is_service_config) {
				fprintf(stderr, "config: line %d: service configs can't have [options] section\n", linenum);
				return(1);
			}
		} else {
			/* directive */
			if(!strlen(section)) {
				fprintf(stderr, "config: line %d: all directives must belong to a section\n", linenum);
				return(1);
			}
			ptr = line;
			key = strsep(&ptr, "=");
			if(key == NULL) {
				fprintf(stderr, "config: line %d: syntax error\n", linenum);
				return(1);
			}
			trim(key);
			key = strtoupper(key);
			if(ptr == NULL) {
				if(!strcmp(key, "USESYSLOG")) {
					o_usesyslog = 1;
					dprint("config: usesyslog\n");
				} else {
					fprintf(stderr, "config: line %d: syntax error\n", linenum);
					return(1);
				}
			} else {
				trim(ptr);
				if(!strcmp(section, "options")) {
					if(is_service_config) {
						// To get here we need options to be modified without going through the loop
						// It's either a bug or an attack attempt, we should simply exit.
						fprintf(stderr, "error: Somehow got inside [options] section on a service config");
						exit(1);
					}
					int scode = parseconfig_options(key, ptr, include_dir, &include_dir_specified, linenum);
					if(scode != 0) {
						return(scode);
					}
				} else {
					if(door == NULL) {
						fprintf(stderr, "config: line %d: \"%s\" can only be used within a Door section\n",
								linenum, key);
						return(1);
					}
					parseconfig_door(key, ptr, door, linenum);
				}
				line[0] = '\0';
			}
		}
	}
	fclose(fp);

	/* sanity checks */
	for(lp = doors; lp; lp = lp->next) {
		door = (opendoor_t*)lp->data;
		if(door->seqcount == 0) {
			fprintf(stderr, "error: section '%s' has an empty knock sequence\n", door->name);
			return(1);
		}
	}

	if(include_dir_specified) {
		DIR *d = opendir(include_dir);
		char service_path[PATH_MAX] = "";
		if(d == NULL) {
			perror("opendir");
		}
		else {
			struct dirent *entry;
			while((entry = readdir(d)) != NULL) {
				if(entry->d_name[0] == '.' && (entry->d_name[1] == '\0' || (entry->d_name[1] == '.' && entry->d_name[2] == '\0'))) {
					continue;
				}

				if(entry->d_type == DT_REG) {
					int path_length = snprintf(service_path, PATH_MAX, "%s/%s", include_dir, entry->d_name);
					if(path_length > PATH_MAX) {
						fprintf(stderr, "error: path too long, skipping: %s\n", entry->d_name);
						continue;
					}
					int scode = parseconfig(service_path, 1);
					if(scode != 0) {
						closedir(d);
						return(scode);
					}
				}
				else {
					fprintf(stderr, "warning: Found non-regular file in the include directory: %s\n", entry->d_name);
				}
			}
			closedir(d);
		}
	}

	return(0);
}
