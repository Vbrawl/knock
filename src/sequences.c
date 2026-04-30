#include "sequences.h"
#include "utils.h"
#include <string.h>
#include "dprint.h"
#include <stdlib.h>
#include <netinet/in.h>

/* Search from the current position in the one time sequence file for the next
 * valid sequence and insert it into the door structure. Returns the position of
 * the beginning of the found line within the file or a negative value if no
 * valid sequence has been found.
 */
long get_next_one_time_sequence(opendoor_t *door)
{
	char line[PATH_MAX+1];
	int pos;

	pos = ftell(door->one_time_sequences_fd);
	while(fgets(line, PATH_MAX, door->one_time_sequences_fd)) {
		trim(line);
		if(strlen(line) == 0 || line[0] == '#') {
			pos = ftell(door->one_time_sequences_fd);
			continue;
		}
		if(parse_port_sequence(line, door) > 0) {
			/* continue searching if parse_port_sequnce returned with an error */
			continue;
		}
		return(pos);
	}
	/* no valid line found */
	return(-1);
}

/* Read a new sequence from the one time sequences file and update the door.
 */
int get_new_one_time_sequence(opendoor_t *door)
{
	rewind(door->one_time_sequences_fd);
	if(get_next_one_time_sequence(door) < 0) {
		/* disable the door by removing it from the doors list if there are no sequences anymore */
		fprintf(stderr, "no more sequences left in the one time sequences file for door %s --> disabling the door\n", door->name);
		logprint("no more sequences left in the one time sequences file for door %s --> disabling the door\n", door->name);
		close_door(door);
		return(1);
	}
	dprint_sequence(door, "new sequence for door %s: ", door->name);

	return(0);
}

/* Parse a port:protocol sequence. Returns a positive integer on error.
 */
int parse_port_sequence(char *sequence, opendoor_t *door)
{
	char *num;
	char *protocol;
	char *port;
	int portnum;

	door->seqcount = 0;	/* reset seqcount */
	while((num = strsep(&sequence, ","))) {
		if(door->seqcount >= SEQ_MAX) {
			fprintf(stderr, "config: section %s: too many ports in knock sequence\n", door->name);
			logprint("error: section %s: too many ports in knock sequence\n", door->name);
			return(1);
		}
		port = strsep(&num, ":");
		/* convert to 4-byte int first so we can easily detect a short overflow */
		portnum = atoi(port);
		if(portnum > 65535) {
			fprintf(stderr, "config: section %s: port %s is invalid\n", door->name, port);
			return(1);
		}
		door->sequence[door->seqcount++] = (unsigned short)portnum;
		if((protocol = strsep(&num, ":"))){
			protocol = strtoupper(trim(protocol));
			if(!strcmp(protocol, "TCP")){
				door->protocol[door->seqcount-1] = IPPROTO_TCP;
			} else if(!strcmp(protocol, "UDP")) {
				door->protocol[door->seqcount-1] = IPPROTO_UDP;
			} else {
				fprintf(stderr, "config: section %s: unknown protocol in knock sequence\n", door->name);
				logprint("error: section %s: unknown protocol in knock sequence\n", door->name);
				return(1);
			}
		} else {
			door->protocol[door->seqcount-1] = IPPROTO_TCP; /* default protocol */
		}
	}
	return(0);
}

/* Get the position (beginning of line) in the one time sequence file of the
 * current sequence such that we know where to insert a '#' to disable the
 * sequence in the one time sequence file
 */
long get_current_one_time_sequence_position(opendoor_t *door)
{
	opendoor_t pseudo_door;	/* used to compare sequences in the file and the current sequence in door */
	long pos;

	rewind(door->one_time_sequences_fd);
	pseudo_door.one_time_sequences_fd = door->one_time_sequences_fd;

	pos = get_next_one_time_sequence(&pseudo_door);
	while(pos >= 0) {
		if(door->seqcount == pseudo_door.seqcount) {
			if((memcmp((void*) door->sequence, (void*) pseudo_door.sequence, door->seqcount) == 0)
					&& (memcmp((void*) door->protocol, (void*) pseudo_door.protocol, door->seqcount) == 0)) {
				return(pos);
			}
		}
		pos = get_next_one_time_sequence(&pseudo_door);
	}
	return(-1);
}


/* Remove a one time sequence from the corresponding file (after a successful
 * knock attempt)
 */
int disable_used_one_time_sequence(opendoor_t *door)
{
	long pos = get_current_one_time_sequence_position(door);
	if(pos >= 0) {
		if(fseek(door->one_time_sequences_fd, pos, SEEK_SET) < 0) {
			fprintf(stderr, "error while disabling used one time sequence for door %s --> disabling the door\n", door->name);
			logprint("error while disabling used one time sequence for door %s --> disabling the door\n", door->name);
			close_door(door);
			return(1);
		}
		if(fputc('#', door->one_time_sequences_fd) == EOF) {
			fprintf(stderr, "error while disabling used one time sequence for door %s --> disabling the door\n", door->name);
			logprint("error while disabling used one time sequence for door %s --> disabling the door\n", door->name);
			close_door(door);
			return(1);
		}
	}
	return(0);
}