#include <stdio.h>
#include <stdbool.h>

#define SLOTS 10

typedef struct {
    int value;
    bool used;
} slot;

typedef struct {
    slot slots[SLOTS];
    int count;
} set;

unsigned int hash(int value) {
    return (unsigned int)value % SLOTS;
}

bool set_add(set *s, int value) {
    unsigned int h = hash(value);

    for (int i = 0; i < SLOTS; i++) {
        unsigned int wh = (h+i) % SLOTS;

        // if slot is free just add and return
        if (!s->slots[wh].used) {
            s->slots[wh].value = value;
            s->slots[wh].used = true;
            s->count++;

            return true;
        } else {
            // if the slot is taken BUT it's a duplicate, just return true!
            if (s->slots[wh].value == value) {
                return true;
            } else {
                // no room at the inn
            //    if (s->count >= SLOTS) return false;
            }
        }
    }

    return false;
}

bool set_contains(set *s, int value) {
    unsigned int h = hash(value);

    for (int i = 0; i < SLOTS; i++) {
        unsigned int wh = (h+i) % SLOTS;

        if (!s->slots[wh].used) {
            return false;
        } else if (s->slots[wh].value == value) {
            // used is true and value matches
            return true;
        }
    }

    return false;
}

int main(void) {
    set myset = {0};

    printf("Adding 3 to set\n");
    set_add(&myset, 3);
    set_add(&myset, 3);
    printf("Adding 13 to set\n");
    set_add(&myset, 13);
    printf("Does set contain 3? %d\n", set_contains(&myset, 3));
    printf("Does set contain 3? %d\n", set_contains(&myset, 3));
    printf("Does set contain 1? %d\n", set_contains(&myset, 1));
    printf("Does set contain 13? %d\n", set_contains(&myset, 13));
    printf("Does set contain -3? %d\n", set_contains(&myset, -3));

    set dupe_set = {0};
    printf("dupe_set count: %d\n", dupe_set.count);
    set_add(&dupe_set, 420);
    printf("dupe_set count: %d\n", dupe_set.count);
    set_add(&dupe_set, 420);
    printf("dupe_set count: %d\n", dupe_set.count);
    set_add(&dupe_set, 69);
    printf("dupe_set count: %d\n", dupe_set.count);
    set_add(&dupe_set, 69);
    printf("dupe_set count: %d\n", dupe_set.count);

    printf("Try to add more than %d elements to a set:\n", SLOTS);
    set full_set = {0};
    for (int i = 0; i < SLOTS + 5; i++) {
        printf("Tried to add %d: %s\n", i, set_add(&full_set, i) ? "yay" : "FULL!");
    }
    printf("add duplicate after full: %s\n", set_add(&full_set, 0) ? "yay" : "BUG!");

    set tricky = {0};
    set_add(&tricky, 3);    // goes to slot 3
    set_add(&tricky, 13);   // collides, goes to slot 4
    // fill the rest so the set is full
    for (int i = 0; i < SLOTS; i++) set_add(&tricky, i);
    printf("%s\n", set_add(&tricky, 13) ? "yay" : "BUG!");

    return 0;
}
