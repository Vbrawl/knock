#include "door.h"
#include <string.h>
#include <stdlib.h>

void init_door(opendoor_t *door, char *name) {
	strncpy(door->name, name, sizeof(door->name)-1);
	door->name[sizeof(door->name)-1] = '\0';
	door->target = 0;
	door->seqcount = 0;
	door->seq_timeout  = SEQ_TIMEOUT; /* default sequence timeout (seconds)  */
	door->start_command = NULL;
	door->start_command6 = NULL;
	door->cmd_timeout = CMD_TIMEOUT; /* default command timeout (seconds) */
	door->stop_command = NULL;
	door->stop_command6 = NULL;
	door->flag_fin = DONT_CARE;
	door->flag_syn = DONT_CARE;
	door->flag_rst = DONT_CARE;
	door->flag_psh = DONT_CARE;
	door->flag_ack = DONT_CARE;
	door->flag_urg = DONT_CARE;
	door->one_time_sequences_fd = NULL;
	door->pcap_filter_exp = NULL;
	door->pcap_filter_expv6 = NULL;
}

void close_door(opendoor_t *door)
{
	doors = list_remove(doors, door);
	free_door(door);
}

void free_door(opendoor_t *door)
{
	if(door) {
		free(door->target);
		free(door->start_command);
		free(door->stop_command);
		if(door->one_time_sequences_fd) {
			fclose(door->one_time_sequences_fd);
		}
		free(door->pcap_filter_exp);
		free(door);
	}
}