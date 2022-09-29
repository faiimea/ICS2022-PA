/***************************************************************************************
* Copyright (c) 2014-2022 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#include "sdb.h"

#define NR_WP 32
/*
typedef struct watchpoint {
  int NO;
  struct watchpoint *next;
*/
 	/* TODO: Add more members if necessary */
	/*char expr[32];
	word_t value;
} WP;*/

static WP wp_pool[NR_WP] = {};
static WP *head = NULL, *free_ = NULL;

void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i;
    wp_pool[i].next = (i == NR_WP - 1 ? NULL : &wp_pool[i + 1]);
  }

  head = NULL;
  free_ = wp_pool;
}

/* TODO: Implement the functionality of watchpoint */

WP* new_wp(char *exp) {
	if (free_ == NULL){
		assert(0);
	}
	
	WP *temp = free_;
	free_ = free_->next;
	temp->next = NULL;
	strcpy(temp->expr, exp);
	bool success = false;
	temp->value = expr(exp, &success);

	if (head == NULL) {
		head = temp;
	}
	else {
		WP *t;
		t = head;
		while(t->next != NULL){
			t = t->next;
		}
		t->next = temp;
	}
	return temp;
}

void free_wp(WP *wp){
	if (wp == NULL){
		assert(0);
	}
	WP *t = head;
	if (wp == head) {
		head = NULL;
	}
	else {
		while (t->next != wp) {
			t = t->next;
		}
		t = wp->next;
	}
	wp->next = NULL;

	if (free_ == NULL) {
		free_ = wp;
	}
	else {
		WP *p = free_;
		while	(p->next != NULL) {
			p = p->next;
		}
		p->next = wp;
	}
}

bool is_changed() {
	bool success = false;
	WP *wp = head;
	while (wp != NULL) {
		word_t value = expr(wp->expr, &success);
		if (value == wp->value) {
			return false;
		}
		else {
			return true;
		}
		wp = wp->next;
	}
	return false;
}

void delete_wp(int NO) {
	WP *wp = head;
	if (head != NULL) {
		while(wp != NULL) {
			if (NO == wp->NO) {
				printf("Successfully delete watchpoint %d, %s", NO, wp->expr);
				return;
			}
		}
	}
}

void print_wp(){
	WP *wp = head;
	if (wp == NULL) {
		printf("There`s no watchpoint.\n");
	}
	else {
		while (wp != NULL) {
			printf("%d, %s\n", wp->NO, wp->expr);
			wp = wp->next;
		}
	}
}
