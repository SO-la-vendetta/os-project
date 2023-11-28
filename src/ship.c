#define _GNU_SOURCE

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <signal.h>

#include "include/const.h"
#include "include/sem.h"
#include "include/shm_general.h"
#include "include/shm_port.h"
#include "include/shm_ship.h"
#include "include/shm_cargo.h"
#include "include/utils.h"
#include "include/shm_offer_demand.h"
#include "include/cargo_list.h"
#include "include/msg_commerce.h"

#define GET_DISTANCE(dest)\
	(sqrt(pow(dest.x - ship_shm_get_coords(state.ship, state.id).x, 2) + pow(dest.y - ship_shm_get_coords(state.ship, state.id).y, 2)))

void signal_handler(int signal);

void init_location(void);
struct coord pick_first_destination_port(void);
void trade(void);
void move(struct coord destination_port);
void send_commerce_mgs(int port_id, struct commerce_msg *msg);

void close_all(void);
void loop(void);
struct coord find_new_destination_port(void);

struct state {
	int id;
	shm_general_t *general;
	shm_port_t *port;
	shm_ship_t *ship;
	shm_cargo_t *cargo;

	shm_demand_t *demand;
	o_list_t *cargo_hold;

	int curr_port_id;
	int current_day;
};

struct state state;

int main(int argc, char *argv[])
{
	struct sigaction sa;
	sigset_t mask;
	bzero(&state, sizeof (struct state));
	bzero(&sa, sizeof(sa));
	sa.sa_handler = &signal_handler;

	sigfillset(&mask);
	sa.sa_mask = mask;
	sigaction(SIGINT, &sa, NULL);
	sigaction(SIGSEGV, &sa, NULL);

	state.id = (int)strtol(argv[1], NULL, 10);
	general_shm_attach(&state.general);
	state.port = port_shm_attach(state.general);
	state.ship = ship_shm_attach(state.general);
	state.cargo = shm_cargo_attach(state.general);
	state.demand = demand_shm_init(state.general);

	state.cargo_hold = cargo_list_create(state.general);

	srand(time(NULL) * getpid());
	init_location();

	sigemptyset(&mask);
	sa.sa_mask = mask;
	sigaction(SIGDAY, &sa, NULL);
	sigaction(SIGSTORM, &sa, NULL);
	sigaction(SIGMAELSTROM, &sa, NULL);

	sem_execute_semop(get_sem_port_init_id(), 0, 0, 0);
	sem_execute_semop(get_sem_start_id(), 0, 0, 0);

	dprintf(1, "ship %d: reached loop\n", state.id);
	loop();
}

void handle_message(void)
{
	struct commerce_msg msg;
	int sender_id, cargo_id, quantity, expiry_date, status;

	if (!msg_commerce_receive(get_msg_out_id(state.general), state.id,
				  &sender_id, &cargo_id, &quantity,
				  &expiry_date, NULL, &status, FALSE)) {
		return;
	}
	switch (status) {
	case STATUS_ACCEPTED:
		/* Struttura per salvare il trading da fare con la barca */
		break;
	case STATUS_LOAD_ACCEPTED:
		cargo_list_add(state.cargo_hold, cargo_id, quantity, expiry_date);
	case STATUS_REFUSED:
	default:
		break;
	}
}

void loop(void) {
	int receiver_port;
	int day;
	int id_dest_port;
	struct coord dest_port_coords;
	struct commerce_msg msg;

	dest_port_coords = pick_first_destination_port();
	move(dest_port_coords);
	trade();

	/*receiver_port = RANDOM_INTEGER(0, get_porti(state.general) - 1);
	dprintf(1, "ship %d: sending message to port %d\n", state.id, receiver_port);
	msg = msg_commerce_create(receiver_port, state.id, 1, 10, 8, 0);
	msg_commerce_send(get_msg_in_id(state.general), &msg);*/
	while (1) {
		day = get_current_day(state.general);
		if (state.current_day < day) {
			/* TODO: new day operations */
			state.current_day = day;
		}
/*		dest_port_coords = find_new_destination_port();
		move(dest_port_coords);
		trade();*/
	}
}

/**
 * @brief initializes ship's location.
 */
void init_location(void)
{
	struct coord coords;
	/* generate a random location on the map */
	coords.x = RANDOM_DOUBLE(0, get_lato(state.general));
	coords.y = RANDOM_DOUBLE(0, get_lato(state.general));

	ship_shm_set_coords(state.ship, state.id, coords);
	ship_shm_set_is_moving(state.ship, state.id, TRUE);
	/* TODO
	 * ship_shm_set_dump_with_cargo(state.ship, state.id, FALSE);*/
}

struct coord pick_first_destination_port(void)
{
	int target_port;
	target_port = RANDOM_INTEGER(0, get_porti(state.general) - 1);
	state.curr_port_id = target_port;
	return port_shm_get_coords(state.port, target_port);
}

/**
 * @brief simulates the movement of the ship and updates the location.
 */
void move(struct coord destination_port)
{
	double distance;
	double time_required;
/*	dprintf(1, "ship %d: before x: %lf y: %lf\n", state.id, ship_shm_get_coords(state.ship, state.id).x, ship_shm_get_coords(state.ship, state.id).y);*/
	ship_shm_set_is_moving(state.ship, state.id, TRUE);
	/* calculate distance between actual position and destination */
	distance = GET_DISTANCE(destination_port);
	/* calculate time required to arrive (in days) */
	time_required = distance / get_speed(state.general);
	convert_and_sleep(time_required);
	/* set new location */
	ship_shm_set_coords(state.ship, state.id, destination_port);
/*	dprintf(1, "ship %d: after x: %lf y: %lf\n", state.id, ship_shm_get_coords(state.ship, state.id).x, ship_shm_get_coords(state.ship, state.id).y);*/
	ship_shm_set_is_moving(state.ship, state.id, FALSE);
}

struct coord find_new_destination_port(void)
{
	int cargo_type;
	int port, best_port = -1;
	int n_ports;
	int port_demand;
	int sale_amount, sale_best_amount = 0;
	int amount_not_expired;
	double distance, best_distance;

	n_ports = get_porti(state.general);
	for (port = 0; port < n_ports; port++) {
		if (port == state.curr_port_id) continue;

		/* Check port distance */
		distance = GET_DISTANCE(port_shm_get_coords(state.port, port));

		sale_amount = 0;
		for (cargo_type = 0; cargo_type < get_merci(state.general); cargo_type++) {
			amount_not_expired = get_not_expired_by_day(state.cargo_hold, cargo_type, state.current_day + (int) distance);
			port_demand = demand_shm_get(state.demand, port, cargo_type);
			sale_amount += MIN(amount_not_expired, port_demand);
		}

		if (best_port == -1 || sale_amount > sale_best_amount
		    || (sale_best_amount == sale_amount && distance < best_distance)) {
			best_port = port;
			sale_best_amount = sale_amount;
			best_distance = distance;
		}
	}

	/* Send message to port */

	state.curr_port_id = best_port;
	return port_shm_get_coords(state.port, best_port);
}

void trade(void)
{

}

void signal_handler(int signal)
{
	switch (signal) {
	case SIGDAY:
		break;
	case SIGSTORM:
		dprintf(1, "Ship %d: Received SIGSTORM signal.\n", state.id);
		ship_shm_set_dump_had_storm(state.ship, state.id);
		convert_and_sleep(get_storm_duration(state.general) / 24.0);
		break;
	case SIGMAELSTROM:
		dprintf(1, "Ship %d: Received SIGMAELSTROM signal.\n",
			state.id);
		ship_shm_set_is_dead(state.ship, state.id);
		close_all();
	case SIGSEGV:
		dprintf(1, "Received SIGSEGV signal.\n");
		dprintf(2, "ship.c: id: %d: Segmentation fault. Terminating.\n",
			state.id);
	case SIGINT:
		close_all();
	}
}

void close_all(void)
{
	cargo_list_delete(state.cargo_hold, state.general);
	port_shm_detach(state.port);
	ship_shm_detach(state.ship);
	shm_cargo_detach(state.cargo);
	general_shm_detach(state.general);
	exit(0);
}
