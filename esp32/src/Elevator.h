#include <Arduino.h>

#define FLOOR_TIME 1500

class Elevator {
private:
    unsigned int floors = 1;
    unsigned int current_floor = UINT32_MAX;
    unsigned int selected_floor = UINT32_MAX;
    unsigned int last_update = 0;

public:
    Elevator(unsigned int floors): floors(floors) {}

    void start(unsigned int new_current_floor, unsigned int new_target_floor) {
        current_floor = new_current_floor;
        selected_floor = new_target_floor;
        last_update = millis();
    }

    bool moveToFloor(unsigned int floor) {
        // if not started
        if (current_floor == UINT32_MAX || selected_floor == UINT32_MAX) return false;

        // if moving
        if (!isIdle()) return false;

        if (floor >= floors) return false;
        if (floor == selected_floor) return false;

        selected_floor = floor;
        last_update = millis();

        return true;
    }

    bool update() {
        // if not started
        if (current_floor == UINT32_MAX || selected_floor == UINT32_MAX) return false;

        if (current_floor == selected_floor || millis() - last_update < FLOOR_TIME) return false;

        if (selected_floor < current_floor) current_floor--;
        else if (selected_floor > current_floor) current_floor++;

        last_update = millis();

        return true;
    }

    unsigned int getFloors() { return floors; }
    unsigned int getCurrentFloor() { return current_floor; }
    unsigned int getSelectedFloor() { return selected_floor; }
    unsigned int isIdle() { return current_floor == selected_floor; }
};