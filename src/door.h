#ifndef _DOOR_H
#define _DOOR_H

#include "config_parser.h"

typedef enum _flag_stat {
	DONT_CARE,  /* 0 */
	SET,        /* 1 */
	NOT_SET     /* 2 */
} flag_stat;

/* knock/event tuples */
typedef struct opendoor {
	char name[128];
	unsigned short seqcount;
	unsigned short sequence[SEQ_MAX];
	unsigned short protocol[SEQ_MAX];
	char *target;
	time_t seq_timeout;
	char *start_command;
	char *start_command6;
	time_t cmd_timeout;
	char *stop_command;
	char *stop_command6;
	flag_stat flag_fin;
	flag_stat flag_syn;
	flag_stat flag_rst;
	flag_stat flag_psh;
	flag_stat flag_ack;
	flag_stat flag_urg;
	FILE *one_time_sequences_fd;
	char *pcap_filter_exp;
	char *pcap_filter_expv6;
} opendoor_t;

void init_door(opendoor_t *door, char *name);

/* Disable the door by removing it from the doors list and free all allocated memory.
 */
void close_door(opendoor_t *door);
void free_door(opendoor_t *door);

#endif