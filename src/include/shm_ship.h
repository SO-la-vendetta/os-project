#ifndef OS_PROJECT_SHM_SHIP_H
#define OS_PROJECT_SHM_SHIP_H

#include "shm_general.h"

typedef struct shm_ship shm_ship_t;

/**
 * @brief Initializes and attaches shared memory for ship data.
 * @param g Pointer to the general shared memory structure.
 * @return Pointer to the attached ship data structure or NULL on failure.
 */
shm_ship_t *shm_ship_initialize(shm_general_t *g);

/**
 * @brief Attaches the process to the shared memory segment for ship data.
 * @param g Pointer to the general shared memory structure.
 * @return Pointer to the attached ship data structure or NULL on failure.
 */
shm_ship_t *shm_ship_attach(shm_general_t *g);
/**
 * @brief Detaches the process from the shared memory segment for ship data.
 * @param g Pointer to the array of ship data in shared memory.
 */
void shm_ship_detach(shm_ship_t *g);
/**
 * @brief Deletes the shared memory segment for ship data.
 * @param g Pointer to the general shared memory structure.
 */
void shm_ship_delete(shm_general_t *g);

/**
 * @brief Sends a signal to all ships in the shared memory structure.
 * @param s Pointer to the array of ship data in shared memory.
 * @param g Pointer to the general shared memory structure.
 * @param signal Signal to send.
 */
void shm_ship_send_signal_to_all_ships(shm_ship_t *s, shm_general_t *g,
				       int signal);
/**
 * @brief Sends a signal to a specific ship in the shared memory structure.
 * @param s Pointer to the array of ship data in shared memory.
 * @param id Identifier of the ship.
 * @param signal Signal to send.
 */
void shm_ship_send_signal_to_ship(shm_ship_t *s, int id, int signal);


/**
 * @brief Sets the process ID for a specific ship in the shared memory structure.
 * @param s Pointer to the array of ship data in shared memory.
 * @param id Identifier of the ship.
 * @param pid Process ID to set.
 */
void shm_ship_set_pid(shm_ship_t *s, int id, pid_t pid);
/**
 * @brief Sets the coordinates for a specific ship in the shared memory structure.
 * @param s Pointer to the array of ship data in shared memory.
 * @param id Identifier of the ship.
 * @param coords Coordinates to set.
 */
void shm_ship_set_coords(shm_ship_t *s, int id, struct coord coords);
/**
 * @brief Marks a specific ship as dead in the shared memory structure.
 * @param s Pointer to the array of ship data in shared memory.
 * @param id Identifier of the ship.
 */
void shm_ship_set_is_dead(shm_ship_t *s, int id);
/**
 * @brief Sets the "is_moving" status for a specific ship in the shared memory structure
 * @param s Pointer to the array of ship data in shared memory.
 * @param id Identifier of the ship.
 */
void shm_ship_set_is_moving(shm_ship_t *s, int id, bool_t value);
/**
 * @brief Sets the "is_at_dock" status for a specific ship in the shared memory structure
 * @param s Pointer to the array of ship data in shared memory.
 * @param id Identifier of the ship.
 */
void shm_ship_set_is_at_dock(shm_ship_t *s, int id, bool_t value);

/* Dump setters */
void shm_ship_set_dump_with_cargo(shm_ship_t *s, int id, bool_t value);
void shm_ship_set_dump_had_storm(shm_ship_t *s, int id);
void shm_ship_set_dump_present(shm_ship_t *s, int ship_id, int id, int quantity);
void shm_ship_set_dump_expired(shm_ship_t *s, int ship_id, int id, int quantity);

/**
 * @brief Gets the process ID of a specific ship in the shared memory structure.
 * @param s Pointer to the array of ship data in shared memory.
 * @param id Identifier of the ship.
 * @return Process ID of the specified ship.
 */
pid_t shm_ship_get_pid(shm_ship_t *s, int id);
/**
 * @brief Gets the "is_dead" status of a specific ship in the shared memory structure.
 * @param s Pointer to the array of ship data in shared memory.
 * @param id Identifier of the ship.
 * @return "is_dead" status of the specified ship.
 */
bool_t shm_ship_get_is_dead(shm_ship_t *s, int id);
/**
 * @brief Gets the "is_moving" status of a specific ship in the shared memory structure.
 * @param s Pointer to the array of ship data in shared memory.
 * @param id Identifier of the ship.
 * @return "is_moving" status of the specified ship.
 */
bool_t shm_ship_get_is_moving(shm_ship_t *s, int id);
/**
 * @brief Gets the coordinates of a specific ship in the shared memory structure.
 * @param s Pointer to the array of ship data in shared memory.
 * @param id Identifier of the ship.
 * @return Coordinates of the specified ship.
 */
struct coord shm_ship_get_coords(shm_ship_t *s, int id);
/**
 * @brief Gets the capacity of a specific ship in the shared memory structure.
 * @param s Pointer to the array of ship data in shared memory.
 * @param id Identifier of the ship.
 * @return Capacity of the specified ship.
 */
int shm_ship_get_capacity(shm_ship_t *s, int id);
/**
 * @brief Gets the "is_at_docks" status of a specific ship in the shared memory structure.
 * @param s Pointer to the array of ship data in shared memory.
 * @param id Identifier of the ship.
 * @return TRUE is ship is at dock. FALSE otherwise.
 */
bool_t shm_ship_get_is_at_dock(shm_ship_t *s, int id);

/* Dump getters */
int shm_ship_get_dump_with_cargo(shm_ship_t *s, int n_ships);
int shm_ship_get_dump_without_cargo(shm_ship_t *s, int n_ships);
int shm_ship_get_dump_at_dock(shm_ship_t *s, int n_ships);
int shm_ship_get_dump_had_storm(shm_ship_t *s, int n_ships);
int shm_ship_get_dump_had_maelstrom(shm_ship_t *s, int n_ships);
int shm_ship_get_dump_is_dead(shm_ship_t *s, int n_ships);
int shm_ship_get_dump_present_by_id(shm_ship_t *s, int id, int type);
int shm_ship_get_dump_expired_by_id(shm_ship_t *s, int id, int type);

void shm_ship_update_capacity(shm_ship_t *s, int ship_id, int update_value);

#endif
