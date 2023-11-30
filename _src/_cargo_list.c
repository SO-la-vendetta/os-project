#define _GNU_SOURCE

#include <stdlib.h>
#include <stdio.h>

#include "../src/include/shm_ship.h"
#include "../src/include/shm_general.h"
#include "header/_cargo_list.h"

struct node {
	int quantity;
	int expire;
	struct node *next;
};

struct o_list {
	struct node *head;
};

static struct node *create_node(int quantity, int expire);
void cargo_list_add_node(o_list_t *list, int type, struct node *node);
int cargo_list_remove_expired(o_list_t *list, shm_general_t *g, shm_ship_t *s, int ship_id, shm_cargo_t *c);

o_list_t *cargo_list_create(shm_general_t *g)
{
	o_list_t *list;
	int i, n_merci;

	n_merci = get_merci(g);

	list = malloc(sizeof(struct o_list) * get_merci(g));
	if (list == NULL) {
		return NULL;
	}

	for (i = 0; i < n_merci; i++) {
		list[i].head = NULL;
	}

	return list;
}

void cargo_list_add(o_list_t *list, int type, int quantity, int expire)
{
	struct node *node, *prev, *curr;

	if (list == NULL) {
		return;
	}

	prev = NULL;
	curr = list[type].head;

	while (1) {
		/* If list is empty or expiration date is lower than current element */
		if (curr == NULL || curr->expire > expire) {
			node = create_node(quantity, expire);
			if (node == NULL) {
				return;
			}
			node->next = curr;
			if (prev != NULL) {
				prev->next = node;
			} else {
				list[type].head = node;
			}

			break;
		} else if (curr->expire == expire) {
			curr->quantity += quantity;
			break;
		}
		prev = curr;
		curr = curr->next;
	}
}

void cargo_list_add_node(o_list_t *list, int type, struct node *node)
{
	struct node *prev, *curr;

	if (list == NULL) {
		return;
	}

	prev = NULL;
	curr = list[type].head;

	while (1) {
		/* If list is empty or expiration date is lower than current element */
		if (curr == NULL || curr->expire > node->expire) {
			node->next = curr;
			if (prev != NULL) {
				prev->next = node;
			} else {
				list[type].head = node;
			}

			break;
		} else if (curr->expire == node->expire) {
			curr->quantity += node->quantity;
			free(node);
			break;
		}
		prev = curr;
		curr = curr->next;
	}
}

int cargo_list_remove_expired(o_list_t *list, shm_general_t *g, shm_ship_t *s, int ship_id, shm_cargo_t *c)
{
	struct node *tmp;
	int i, n_merci, expire_day, qt = 0;

	if (list == NULL) {
		return 0;
	}

	n_merci = get_merci(g);
	expire_day = get_current_day(g);

	/*qt = calloc(n_merci, sizeof(int));*/

	for (i = 0; i < n_merci; i++) {
		if (list[i].head != NULL &&
		    list[i].head->expire == expire_day) {
			if(s != NULL) {
				shm_cargo_set_dump_daily_expired_on_ship(c, i, list[i].head->quantity);
				shm_ship_update_capacity(s, ship_id, list[i].head->quantity * shm_cargo_get_size(c, i));
			} else {
				shm_cargo_set_dump_daily_expired_in_port(c, i, list[i].head->quantity);
			}
			qt += list[i].head->quantity;
			tmp = list[i].head;
			list[i].head = tmp->next;
			free(tmp);
		}
	}

	return qt;
}

o_list_t *cargo_list_pop_needed(o_list_t *list, shm_general_t *g, int id,
				int quantity)
{
	o_list_t *output;

	struct node *tmp;
	int cnt;

	if (list == NULL) {
		return NULL;
	}

	if (list[id].head == NULL || quantity == 0) {
		return NULL;
	}

	output = cargo_list_create(g);
	if (output == NULL) {
		return NULL;
	}

	cnt = quantity;
	while (list[id].head->next != NULL || cnt > 0) {
		if (list[id].head->quantity <= cnt) {
			cnt -= list[id].head->quantity;
			cargo_list_add(output, id, list[id].head->quantity, list[id].head->expire);
			tmp = list[id].head;
			list[id].head = list[id].head->next;
			free(tmp);
		} else {
			cargo_list_add(output, id, cnt, list[id].head->expire);
			list[id].head->quantity -= cnt;
			break;
		}
	}

	return output;
}

int cargo_list_get_quantity_by_id(o_list_t *list, int id)
{
	struct node *tmp;
	int cnt;

	if (list == NULL) {
		return -1;
	}

	tmp = list[id].head;
	cnt = 0;

	while (tmp != NULL) {
		cnt += tmp->quantity;
		tmp = tmp->next;
	}

	return cnt;
}

int cargo_list_get_quantity(o_list_t *list, shm_general_t *g)
{
	struct node *tmp;
	int cnt;
	int id = 0;

	if (list == NULL) {
		return -1;
	}

	tmp = list[id].head;
	cnt = 0;
	for(id = 0; id < get_merci(g); id++) {
		while (tmp != NULL) {
			cnt += tmp->quantity;
			tmp = tmp->next;
		}
	}
	return cnt;
}

int cargo_list_get_first_expire(o_list_t *list, shm_general_t *g, int id)
{
	if (list == NULL || list[id].head == NULL) {
		return -1;
	}

	return list[id].head->expire;
}

void cargo_list_delete(o_list_t *list, shm_general_t *g)
{
	struct node *curr, *tmp;
	int i, n;

	n = get_merci(g);

	for (i = 0; i < n; i++) {
		curr = list[i].head;

		while (curr != NULL) {
			tmp = curr;
			curr = curr->next;
			free(tmp);
		}
	}

	free(list);
}

void cargo_list_merge(o_list_t *src, o_list_t *merge, shm_general_t *g)
{
	struct node *curr;
	int i, n_merci;

	n_merci = get_merci(g);

	for (i = 0; i < n_merci; i++) {
		curr = merge[i].head;
		while (curr->next != NULL) {
			cargo_list_add_node(src, i, curr);
			curr = curr->next;
		}
	}
}

void cargo_list_print_all(o_list_t *list, shm_general_t *g)
{
	struct node *curr;

	int i, n;

	n = get_merci(g);

	for (i = 0; i < n; i++) {
		curr = list[i].head;
		printf("Element of type %d:\n", i);
		while (curr != NULL) {
			printf("quantity: %d, expire: %d\n", curr->quantity, curr->expire);
			curr = curr->next;
		}
		printf("\n");
	}
}


struct node_msg *cargo_list_pop_order(o_list_t *list, shm_general_t *g)
{
	struct node *tmp;
	struct node_msg *output;

	int i, n;

	output = NULL;
	n = get_merci(g);

	for (i = 0; i < n; i++) {
		if (list[i].head == NULL) {
			continue;
		}

		output = malloc(sizeof(struct node_msg));
		output->quantity = list[i].head->quantity;
		output->expire = list[i].head->expire;
		output->id = i;

		tmp = list[i].head;
		list[i].head = list[i].head->next;
		free(tmp);
		break;
	}

	return output;
}

static struct node *create_node(int quantity, int expire)
{
	struct node *node;

	node = malloc(sizeof(struct node));
	if (node == NULL) {
		return NULL;
	}

	node->quantity = quantity;
	node->expire = expire;
	node->next = NULL;

	return node;
}

int get_not_expired_by_day(o_list_t *list, int cargo_type, int day)
{
	/* Need to test */
	int res;
	struct node *curr;
	res = 0;
	for (curr = list[cargo_type].head; curr != NULL; curr = curr->next) {
		if (curr->expire > day) {
			res += curr->quantity;
		}
	}
	return res;
}

/* wrappers */
int cargo_list_port_remove_expired(o_list_t *list, shm_general_t *g, shm_cargo_t *c)
{
	return cargo_list_remove_expired(list, g, NULL, -1, c);
}

int cargo_list_ship_remove_expired(o_list_t *list, shm_general_t *g, shm_ship_t *s, int ship_id, shm_cargo_t *c)
{
	return cargo_list_remove_expired(list, g, s, ship_id,  c);
}

void cargo_list_pop(o_list_t *list, int *quantity, int *expiration_date) {
	struct node *tmp;

	if (list == NULL || list->head == NULL) {
		*quantity = -1;
		*expiration_date = -1;
		return;
	}

	*quantity = list->head->quantity;
	*expiration_date = list->head->expire;

	tmp = list->head;
	list->head = list->head->next;
	free(tmp);
}

void cargo_list_print_single(o_list_t *list)
{
	struct node *curr;

	curr = list->head;
	dprintf(1, "list!\n");
	if (curr == NULL) {
		dprintf(1, "list null!\n");
		return;
	}
	while (curr != NULL) {
		dprintf(1, "%d %d\n", curr->quantity, curr->expire);
		curr = curr->next;
	}
}