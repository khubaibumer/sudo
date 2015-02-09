/*
 * Copyright (c) 2010-2015 Todd C. Miller <Todd.Miller@courtesan.com>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <config.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#ifdef STDC_HEADERS
# include <stdlib.h>
# include <stddef.h>
#else
# ifdef HAVE_STDLIB_H
#  include <stdlib.h>
# endif
#endif /* STDC_HEADERS */
#ifdef HAVE_STRING_H
# if defined(HAVE_MEMORY_H) && !defined(STDC_HEADERS)
#  include <memory.h>
# endif
# include <string.h>
#endif /* HAVE_STRING_H */
#ifdef HAVE_STRINGS_H
# include <strings.h>
#endif /* HAVE_STRINGS_H */
#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif /* HAVE_UNISTD_H */
#include <netinet/in.h>  
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>

#include "sudoers.h"
#include "interfaces.h"

#ifndef INADDR_NONE
# define INADDR_NONE ((unsigned int)-1)
#endif

static struct interface_list interfaces;

/*
 * Parse a space-delimited list of IP address/netmask pairs and
 * store in a list of interface structures.
 */
void
set_interfaces(const char *ai)
{
    char *addrinfo, *addr, *mask;
    struct interface *ifp;
    debug_decl(set_interfaces, SUDOERS_DEBUG_NETIF)

    addrinfo = sudo_estrdup(ai);
    for (addr = strtok(addrinfo, " \t"); addr != NULL; addr = strtok(NULL, " \t")) {
	/* Separate addr and mask. */
	if ((mask = strchr(addr, '/')) == NULL)
	    continue;
	*mask++ = '\0';

	/* Parse addr and store in list. */
	ifp = sudo_ecalloc(1, sizeof(*ifp));
	if (strchr(addr, ':')) {
	    /* IPv6 */
#ifdef HAVE_STRUCT_IN6_ADDR
	    ifp->family = AF_INET6;
	    if (inet_pton(AF_INET6, addr, &ifp->addr.ip6) != 1 ||
		inet_pton(AF_INET6, mask, &ifp->netmask.ip6) != 1)
#endif
	    {
		sudo_efree(ifp);
		continue;
	    }
	} else {
	    /* IPv4 */
	    ifp->family = AF_INET;
	    if (inet_pton(AF_INET, addr, &ifp->addr.ip4) != 1 ||
		inet_pton(AF_INET, mask, &ifp->netmask.ip4) != 1) {
		sudo_efree(ifp);
		continue;
	    }
	}
	SLIST_INSERT_HEAD(&interfaces, ifp, entries);
    }
    sudo_efree(addrinfo);
    debug_return;
}

struct interface_list *
get_interfaces(void)
{
    return &interfaces;
}

void
dump_interfaces(const char *ai)
{
    char *cp, *addrinfo;
    debug_decl(set_interfaces, SUDOERS_DEBUG_NETIF)

    addrinfo = sudo_estrdup(ai);

    sudo_printf(SUDO_CONV_INFO_MSG, _("Local IP address and netmask pairs:\n"));
    for (cp = strtok(addrinfo, " \t"); cp != NULL; cp = strtok(NULL, " \t"))
	sudo_printf(SUDO_CONV_INFO_MSG, "\t%s\n", cp);

    sudo_efree(addrinfo);
    debug_return;
}
