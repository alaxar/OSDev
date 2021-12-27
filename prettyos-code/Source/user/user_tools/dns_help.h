#ifndef DNS_HELP_H
#define DNS_HELP_H

#include "types.h"

IP4_t getAddrByName(const char* name);
void showDNSQuery(const char* name);

#endif
