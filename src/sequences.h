#ifndef _SEQUENCES_H
#define _SEQUENCES_H

#include "door.h"

/* Search from the current position in the one time sequence file for the next
 * valid sequence and insert it into the door structure. Returns the position of
 * the beginning of the found line within the file or a negative value if no
 * valid sequence has been found.
 */
long get_next_one_time_sequence(opendoor_t *door);

/* Read a new sequence from the one time sequences file and update the door.
 */
int get_new_one_time_sequence(opendoor_t *door);

/* Parse a port:protocol sequence. Returns a positive integer on error.
 */
int parse_port_sequence(char *sequence, opendoor_t *door);


/* Get the position (beginning of line) in the one time sequence file of the
 * current sequence such that we know where to insert a '#' to disable the
 * sequence in the one time sequence file
 */
long get_current_one_time_sequence_position(opendoor_t *door);


/* Remove a one time sequence from the corresponding file (after a successful
 * knock attempt)
 */
int disable_used_one_time_sequence(opendoor_t *door);


#endif