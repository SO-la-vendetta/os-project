#ifndef OS_PROJECT_SHM_PORT_H
#define OS_PROJECT_SHM_PORT_H

#include <sys/types.h>

#include "shm_general.h"
#include "types.h"
#include "expired.h"

typedef struct shm_port shm_port_t;

shm_port_t *port_initialize(shm_general_t *g);

shm_port_t *port_shm_attach(shm_general_t *c);
void port_shm_detach(shm_port_t *p);
void port_shm_delete(shm_general_t *c);

void port_shm_set_pid(shm_port_t *p, int id, pid_t pid);
void port_shm_send_signal_to_all_ports(shm_port_t *p, shm_general_t *c,
				       int signal);
void port_shm_send_signal_to_port(shm_port_t *p, int id, int signal);

void port_shm_set_coordinates(shm_port_t *p, int id, struct coord coord);
void port_shm_set_docks(shm_port_t *p, int id, int n);
void port_shm_generate_cargo(shm_port_t *p, int id, shm_general_t *c);
void port_shm_remove_expired(shm_port_t *p, expired_t *exp, shm_general_t *c);

struct coord port_shm_get_coords(shm_port_t *p, int id);

pid_t port_shm_get_pid(shm_port_t *p, int id);

#endif