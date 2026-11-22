#include <stdio.h>
#include <stdbool.h>

#define SLOTS 10

/**
 * the states a set slot can be in
 */
typedef enum {
    // this slot is fresh, and available for use
    FREE = 0,
    // there's a value in this slot
    IN_USE,
    // there was previously a value in this slot but it can be reused for new adds
    TOMBSTONED,
} slot_state;

/**
 * a model of a slot in the set
 */
typedef struct {
    int value;
    slot_state state;
} slot;

/**
 * the model of the set, with SLOTS capacity
 */
typedef struct {
    slot slots[SLOTS];
    int count;
} set;

/**
 * Turn an int into a hash for use in the set
 */
unsigned int hash(int value) {
    return (unsigned int)value;
}

/**
 * Turn a hash into a slot index, with wrapping
 */
int slot_index(unsigned int hash) {
    return hash % SLOTS;
}

/**
 * Locate the slot index of a given value
 *
 * returns -1 if the value was not found
 */
int set_find_slot(set *s, int value) {
    unsigned int h = slot_index(hash(value));

    for (int i = 0; i < SLOTS; i++) {
        unsigned int wh = (h+i) % SLOTS;

        if (s->slots[wh].state == FREE) return -1;

        if (s->slots[wh].state == IN_USE) {
            if (s->slots[wh].value == value) {
                // unsigned int can be cast to an int safely
                return wh;
            }
        }
    }

    return -1;
}

/**
 * Returns true if the value is in the set, else false
 */
bool set_contains(set *s, int value) {
    return set_find_slot(s, value) >= 0;
}

/**
 * Adds a value to the set.
 *
 * returns true if the value was added, or already existed in the set
 */
bool set_add(set *s, int value) {
    unsigned int h = slot_index(hash(value));

    // value is already here
    if (set_contains(s, value)) return true;

    for (int i = 0; i < SLOTS; i++) {
        unsigned int wh = (h+i) % SLOTS;

        // if slot is free or tombstoned, add and return
        if (s->slots[wh].state != IN_USE) {
            s->slots[wh].value = value;
            s->slots[wh].state = IN_USE;
            s->count++;

            return true;
        }
    }

    return false;
}

/**
 * Remove a value from the set.
 *
 * returns true if the value was removed, or false if the value wasn't in the set
 */
bool set_remove(set *s, int value) {
    int si = set_find_slot(s, value);

    if (si >= 0) {
        s->slots[si].state = TOMBSTONED;
        s->count--;

        return true;
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

    bool r = set_remove(&myset, 3);
    printf("Removed 3. Does it contain 3 now? %s (%d)\n", set_contains(&myset, 3) ? "yes" : "no", r);
    r = set_remove(&myset, 3);
    printf("Tried to remove 3 again. Does it contain 3 now? %s (%d)\n", set_contains(&myset, 3) ? "yes" : "no", r);
    set_add(&myset, 3);
    set_remove(&myset, 3);
    set_add(&myset, 13);
    printf("set contains 3: %d, set contains 13: %d, set count: %d\n", set_contains(&myset, 3), set_contains(&myset, 13), myset.count);


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

    set t = {0};
    set_add(&t, 3);    // slot 3
    set_add(&t, 13);   // slot 4
    set_remove(&t, 13);
    printf("3? %d  13? %d\n", set_contains(&t, 3), set_contains(&t, 13));

    return 0;
}
