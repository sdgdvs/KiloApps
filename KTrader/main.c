#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    int credits;
    int fuel;
    int maxFuel;
    int cargo;
    int maxCargo;
    int locationId;
} PlayerState;

const char* planets[] = {"Earth", "Mars", "Venus"};
int travelCosts[3][3] = {
    {0, 20, 15},
    {20, 0, 25},
    {15, 25, 0}
};

void printDashboard(PlayerState* state) {
    printf("==========================================\n");
    printf("KTrader Space Trading Sim\n");
    printf("==========================================\n");
    printf("Credits: %d\n", state->credits);
    printf("Fuel:    %d / %d\n", state->fuel, state->maxFuel);
    printf("Cargo:   %d / %d\n", state->cargo, state->maxCargo);
    printf("Location: %s\n", planets[state->locationId]);
    printf("==========================================\n");
}

void travel(PlayerState* state, int targetId) {
    int cost = travelCosts[state->locationId][targetId];
    if (cost == 0) {
        printf("> You are already there.\n");
        return;
    }
    if (state->fuel >= cost) {
        state->fuel -= cost;
        state->locationId = targetId;
        printf("> Hyperspace jump complete. Arrived at %s. Used %d fuel.\n", planets[targetId], cost);
    } else {
        printf("> Insufficient fuel to reach %s!\n", planets[targetId]);
    }
}

int main() {
    PlayerState state = {1000, 100, 100, 0, 20, 0};
    int running = 1;
    char input[10];

    printf("> Welcome to KTrader, Captain.\n");

    while (running) {
        printf("\n");
        printDashboard(&state);
        printf("Available destinations:\n");
        for (int i = 0; i < 3; i++) {
            if (i != state.locationId) {
                printf(" %d: Travel to %s (Cost: %d fuel)\n", i, planets[i], travelCosts[state.locationId][i]);
            }
        }
        printf(" 9: Quit\n");
        printf("Enter command: ");
        
        if (fgets(input, sizeof(input), stdin) != NULL) {
            int cmd = atoi(input);
            if (cmd == 9) {
                running = 0;
            } else if (cmd >= 0 && cmd < 3 && cmd != state.locationId) {
                travel(&state, cmd);
            } else {
                printf("> Invalid command.\n");
            }
        } else {
            running = 0;
        }
    }
    printf("> Goodbye, Captain.\n");
    return 0;
}
