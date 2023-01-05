/*
 * resource.h
 *
 *  Created on: 29/12/2014
 *      Author: 
 */

#ifndef UTILS_RESOURCE_H_
#define UTILS_RESOURCE_H_

typedef struct {
	double user_ms;
	double system_ms;
} rcpu_usage;

typedef struct {
	long unsigned net_in;
	long unsigned net_out;
} rnet_usage;

rcpu_usage get_cpu_usage();
rnet_usage get_net_usage(char *intf);

#endif /* UTILS_RESOURCE_H_ */
