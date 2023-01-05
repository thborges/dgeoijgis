/*
 * resource.c
 *
 *  Created on: 29/12/2014
 *      Author: 
 */

#include <stddef.h>
#include <stdio.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <ifstat.h>
#include "resource.h"

rcpu_usage get_cpu_usage() {
	rcpu_usage cpu_usage = {0.0, 0.0};
	struct rusage ru;
	if (getrusage(RUSAGE_SELF, &ru) == 0) {
		cpu_usage.user_ms = ru.ru_utime.tv_sec * 1000.0 + ru.ru_utime.tv_usec / 1000.0;
		cpu_usage.system_ms = ru.ru_stime.tv_sec * 1000.0 + ru.ru_stime.tv_usec / 1000.0;
	}
	return cpu_usage;
}

rnet_usage get_net_usage(char *intf) {

	rnet_usage net_usage = {0, 0};
    struct ifstat_list ifs;
    struct ifstat_driver driver;
    char *dname = NULL;

    ifs.flags = 0;
    ifs.first = NULL;

    ifstat_add_interface(&ifs, intf, 0);

    if (!ifstat_get_driver(dname, &driver)) {
            fprintf(stderr, "Net stat driver not available\n");
            return net_usage;
    }

    if (driver.open_driver != NULL) {
            if (!driver.open_driver(&driver, NULL)) {
                    fprintf(stderr, "Cannot open net stat driver!\n");
                    return net_usage;
            }
    }

    if (!driver.get_stats(&driver, &ifs)) {
            fprintf(stderr, "Cannot get stats\n");
            return net_usage;
    }

    struct ifstat_data *ptr = ifs.first;
    net_usage.net_in = ifs.first->bin;
    net_usage.net_out = ifs.first->bout;

    ifstat_free_interface(ptr);
	if (driver.close_driver)
	    driver.close_driver(&driver);

	return net_usage;
}
