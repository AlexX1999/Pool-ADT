#include <assert.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include "pool.h"

struct box {
  char *start;
  char *end;
  struct box *next;
  struct box *prev;
  int space_ahead;
};

struct pool {
  char *storage;
  int available;
  struct box *head;
  int gap;   // available space between storage and the first "box"
};


struct pool *pool_create(int size) {
  assert(size > 0);
  struct pool *my_pool = malloc(sizeof(struct pool));
  my_pool->storage = malloc(size * sizeof(char));
  my_pool->available = size;
  my_pool->gap = size;
  my_pool->head = NULL;
  return my_pool;
}


bool pool_destroy(struct pool *p) {
  assert(p);
  if (p->head) {
    return false;
  }
  else {
    free(p->storage);
    free(p);
    return true;
  }
}



char *pool_alloc(struct pool *p, int size) {
  assert(p);
  assert(size > 0);
  if (size > p->available) {
    return NULL;
  }
  else {
    if (p->head == NULL) {
      struct box *my_box = malloc(sizeof(struct box));
      p->available = p->available - size;
      my_box->start = p->storage;
      my_box->end = my_box->start + (size - 1);
      my_box->next = NULL;
      my_box->prev = NULL;
      my_box->space_ahead = p->available;
      p->head = my_box;
      p->gap = 0;
      return my_box->start;
    }
    else if (p->gap >= size) {
      struct box *new_box = malloc(sizeof(struct box));
      new_box->next = p->head;
      new_box->prev = NULL;
      (p->head)->prev = new_box;
      p->head = new_box;
      new_box->start = p->storage;
      new_box->end = new_box->start + size - 1;
      new_box->space_ahead = p->gap - size;
      p->gap = 0;
      p->available = p->available - size;
        return new_box->start;
    }
    else {
      struct box *current = p->head;
      while (current) {
        if (current->space_ahead >= size) {
          struct box *new = malloc(sizeof(struct box));
          new->start = current->end + 1;
          new->end = new->start + (size - 1);
          new->space_ahead = current->space_ahead - size;
          current->space_ahead = 0;
          new->prev = current;
          new->next = current->next;
          current->next = new;
          p->available = p->available - size;
          if (new->next) {
            (new->next)->prev = new;
          }
          return new->start;
        }
        current = current->next;
      }
      return NULL;
    }
  }
}


bool pool_free(struct pool *p, char *addr) {
  assert(p);
  assert(addr);
  struct box *current = p->head;
  while(current) {
    if (current->start == addr) {
      int space = current->end - current->start + 1;
      p->available += space;
      if (current->prev == NULL) {
        p->head = current->next;
        p->gap = p->gap + space + current->space_ahead;
      }
      if (current->prev) {
        current->prev->space_ahead += space;
        (current->prev)->next = current->next;
      }
      if (current->next) {
        (current->next)->prev = current->prev;
      }
      free(current);
      return true;
    }
    current = current->next;
  }
  return false;
}


char *pool_realloc(struct pool *p, char *addr, int size) {
  assert(p);
  assert(size > 0);
  assert(addr);
  struct box *current = p->head;
  while (current) {
    if (current->start == addr) {
      int space = current->end - current->start + 1;
      if (size <= space || (space + current->space_ahead) >= size) {
        current->end = current->start + size - 1;
        current->space_ahead = current->space_ahead + space - size;
        p->available = p->available + space - size;
        return current->start;
      }
      else {
        char *s = pool_alloc(p, size);
        if (s == NULL) {
          return NULL;
        }
        for (int i = 0; i < space; ++i) {
          *(s + i) = *(current->start + i);
        }
        pool_free(p, current->start);
        return s;
      }
    }
    current = current->next;
  }
  return NULL;
}



void pool_print_active(struct pool *p) {
  assert(p);
  if (p->head == NULL) {
    printf("active: none\n");
  }
  else {
    int space = p->head->end - p->head->start + 1;
    int pos = p->head->start - p->storage;
    printf("active: %d [%d]", pos, space);
    struct box *current = p->head->next;
    while (current) {
      int size = current->end - current->start + 1;
      int index = current->start - p->storage;
      printf(", %d [%d]", index, size);
      current = current->next;
    }
    printf("\n");
  }
}



void pool_print_available(struct pool *p) {
  assert(p);
  if (p->available == 0) {
    printf("available: none\n");
  }
  else {
    if (p->gap) {
      printf("available: 0 [%d]", p->gap);
      struct box *p1 = p->head;
      while (p1) {
        int index = p1->end + 1 - p->storage;
        printf(", %d [%d]", index, p1->space_ahead);
        p1 = p1->next;
      }
      printf("\n");
    }
    else {
      int pos = p->head->end + 1 - p->storage;
      printf("available: %d [%d]", pos, p->head->space_ahead);
      struct box *p2 = p->head->next;
      while (p2) {
        int position = p2->end + 1 - p->storage;
        printf(", %d [%d]", position, p2->space_ahead);
        p2 = p2->next;
      }
      printf("\n");
    }
  }
}

