#include "dprint.h"
#include <stdarg.h>
#include <syslog.h>
#include <netinet/in.h>

FILE *logfd = NULL;

void dprint(char *fmt, ...)
{
	va_list args;
	if(o_debug) {
		va_start(args, fmt);
		vprintf(fmt, args);
		va_end(args);
		fflush(stdout);
	}
}

void vprint(char *fmt, ...)
{
	va_list args;
	if(o_verbose) {
		va_start(args, fmt);
		vprintf(fmt, args);
		va_end(args);
		fflush(stdout);
	}
}

/* Output a message to syslog and/or a logfile */
void logprint(char *fmt, ...)
{
	char msg[1024];
	va_list args;
	va_start(args, fmt);
	vsnprintf(msg, 1024, fmt, args);
	va_end(args);
	if(o_usesyslog) {
		syslog(LOG_NOTICE, "%s", msg);
	}
	if(logfd) {
		time_t t;
		struct tm *tm;
		t = time(NULL);
		tm = localtime(&t);

		fprintf(logfd, "[%04d-%02d-%02d %02d:%02d] %s\n", tm->tm_year+1900,
			tm->tm_mon+1, tm->tm_mday, tm->tm_hour, tm->tm_min, msg);
		fflush(logfd);
	}
}

/* Output current sequence of door for debugging */
void dprint_sequence(opendoor_t *door, char *fmt, ...)
{
	va_list args;
	int i;

	if(o_debug) {
		va_start(args, fmt);
		vprintf(fmt, args);
		va_end(args);
		for(i = 0; i < door->seqcount; i++) {
			switch(door->protocol[i]){
				case IPPROTO_UDP:
					printf((i == door->seqcount-1 ? "%u:udp\n" : "%u:udp,"), door->sequence[i]);
					break;
				case IPPROTO_TCP: /* fallthrough */
				default: 
					printf((i == door->seqcount-1 ? "%u:tcp\n" : "%u:tcp,"), door->sequence[i]);
			}
		}
		fflush(stdout);
	}
}