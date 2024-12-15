#pragma once

#define muz_list_record(address, type, field)         ((type*)((char*)(address) - (char*)(&((type*)0)->field)))
#define muz_list_for_each(position, head)             for (position = (head)->next; position != head; position = position->next)
#define muz_list_for_each_safe(position, Safe, head)  for (position = (head)->next, Safe = position->next; position != (head); position = Safe, Safe = position->next)

struct muz_list_entry {
   struct muz_list_entry *prev;
   struct muz_list_entry *next;
};

static void
muz_list_init(struct muz_list_entry *self) {
   self->prev = self;
   self->next = self;
}

static void
muz_list_term(struct muz_list_entry *self) {
   self->prev = (struct muz_list_entry *)0;
   self->next = (struct muz_list_entry *)0;
}

static int
muz_list_is_empty(const struct muz_list_entry *head) {
   return head->next == head;
}

static void
muz_list_add_(struct muz_list_entry *new_, struct muz_list_entry *prev, struct muz_list_entry *next) {
   next->prev = new_;
   new_->next = next;
   new_->prev = prev;
   prev->next = new_;
}

static void
muz_list_push_front(struct muz_list_entry *head, struct muz_list_entry *entry) {
   muz_list_add_(entry, head, head->next);
}

static void
muz_list_push_back(struct muz_list_entry *head, struct muz_list_entry *entry) {
   muz_list_add_(entry, head->prev, head);
}

static struct muz_list_entry *
muz_list_get_front(const struct muz_list_entry *head) {
   struct muz_list_entry *entry = head->next;
   if (entry != head) {
      return entry;
   }

   return 0;
}

static void
muz_list_remove_(struct muz_list_entry *prev, struct muz_list_entry *next) {
   next->prev = prev;
   prev->next = next;
}

static void
muz_list_remove(const struct muz_list_entry *entry) {
   muz_list_remove_(entry->prev, entry->next);
}
