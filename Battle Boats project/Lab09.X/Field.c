#include "Field.h"
#include <stddef.h>  
#include <stdio.h>   
#include "BOARD.h"
#include <stdbool.h>

#define STANDARD_ERROR 0
#define SUCCESS 1

void FieldPrint_UART(Field *own_field, Field *opp_field) {
    printf(" Own field:\n");
    for (int i = 0; i < FIELD_ROWS; i++) {
        for (int j = 0; j < FIELD_COLS; j++) {
            printf(" %d", own_field->grid[i][j]);
        }
        printf("\n");
    }

    printf("\n Opponent's field:\n");
    for (int i = 0; i < FIELD_ROWS; i++) {
        for (int j = 0; j < FIELD_COLS; j++) {
            printf(" %d", opp_field->grid[i][j]);
        }
        printf("\n");
    }
    printf("\n");
}

void FieldInit(Field *own_field, Field *opp_field) {
    // Initialize own_field
    for (uint8_t row = 0; row < FIELD_ROWS; row++) {
        for (uint8_t col = 0; col < FIELD_COLS; col++) {
            own_field->grid[row][col] = FIELD_SQUARE_EMPTY;
        }
    }
    own_field->smallBoatLives = 0;
    own_field->mediumBoatLives = 0;
    own_field->largeBoatLives = 0;
    own_field->hugeBoatLives = 0;

    // Initialize opp_field if it's not NULL
    if (opp_field != NULL) {
        for (uint8_t row = 0; row < FIELD_ROWS; row++) {
            for (uint8_t col = 0; col < FIELD_COLS; col++) {
                opp_field->grid[row][col] = FIELD_SQUARE_UNKNOWN;
            }
        }
        opp_field->smallBoatLives = 0;
        opp_field->mediumBoatLives = 0;
        opp_field->largeBoatLives = 0;
        opp_field->hugeBoatLives = 0;
    }
}

SquareStatus FieldGetSquareStatus(const Field *field, uint8_t row, uint8_t col) {
    if (row >= FIELD_ROWS || col >= FIELD_COLS) {
        return FIELD_SQUARE_INVALID;
    }
    return field->grid[row][col];   
}

SquareStatus FieldSetSquareStatus(Field *field, uint8_t row, uint8_t col, SquareStatus status) {
    if (row >= FIELD_ROWS || col >= FIELD_COLS) {
        return FIELD_SQUARE_INVALID;
    }
    field->grid[row][col] = status;
    return status;
}

uint8_t FieldAddBoat(Field *field, uint8_t row, uint8_t col, BoatDirection dir, BoatType boat_type) {
    uint8_t boat_length = 0;
    SquareStatus square_status = FIELD_SQUARE_EMPTY;
    uint8_t *boat_lives = NULL;

    switch (boat_type) {
        case FIELD_BOAT_TYPE_SMALL:
            boat_length = 3;
            square_status = FIELD_SQUARE_SMALL_BOAT;
            boat_lives = &field->smallBoatLives;
            break;
        case FIELD_BOAT_TYPE_MEDIUM:
            boat_length = 4;
            square_status = FIELD_SQUARE_MEDIUM_BOAT;
            boat_lives = &field->mediumBoatLives;
            break;
        case FIELD_BOAT_TYPE_LARGE:
            boat_length = 5;
            square_status = FIELD_SQUARE_LARGE_BOAT;
            boat_lives = &field->largeBoatLives;
            break;
        case FIELD_BOAT_TYPE_HUGE:
            boat_length = 6;
            square_status = FIELD_SQUARE_HUGE_BOAT;
            boat_lives = &field->hugeBoatLives;
            break;
        default:
            return STANDARD_ERROR;
    }

    if (dir == FIELD_DIR_EAST) {
        if (col + boat_length > FIELD_COLS) {
            return STANDARD_ERROR;
        }
        for (uint8_t i = 0; i < boat_length; i++) {
            if (field->grid[row][col + i] != FIELD_SQUARE_EMPTY) {
                return STANDARD_ERROR;
            }
        }
        for (uint8_t i = 0; i < boat_length; i++) {
            field->grid[row][col + i] = square_status;
        }
    } else if (dir == FIELD_DIR_SOUTH) {
        if (row + boat_length > FIELD_ROWS) {
            return STANDARD_ERROR;
        }
        for (uint8_t i = 0; i < boat_length; i++) {
            if (field->grid[row + i][col] != FIELD_SQUARE_EMPTY) {
                return STANDARD_ERROR;
            }
        }
        for (uint8_t i = 0; i < boat_length; i++) {
            field->grid[row + i][col] = square_status;
        }
    } else {
        return STANDARD_ERROR;
    }

    *boat_lives = boat_length;
    return SUCCESS;
}

SquareStatus FieldRegisterEnemyAttack(Field *field, GuessData *gData) {
    SquareStatus previousStatus = field->grid[gData->row][gData->col];
    switch (previousStatus) {
        case FIELD_SQUARE_EMPTY:
            field->grid[gData->row][gData->col] = FIELD_SQUARE_MISS;
            gData->result = RESULT_MISS;
            break;
        case FIELD_SQUARE_SMALL_BOAT:
        case FIELD_SQUARE_MEDIUM_BOAT:
        case FIELD_SQUARE_LARGE_BOAT:
        case FIELD_SQUARE_HUGE_BOAT:
            field->grid[gData->row][gData->col] = FIELD_SQUARE_HIT;
            gData->result = RESULT_HIT;
            switch (previousStatus) {
                case FIELD_SQUARE_SMALL_BOAT:
                    if (--field->smallBoatLives == 0) {
                        gData->result = RESULT_SMALL_BOAT_SUNK;
                    }
                    break;
                case FIELD_SQUARE_MEDIUM_BOAT:
                    if (--field->mediumBoatLives == 0) {
                        gData->result = RESULT_MEDIUM_BOAT_SUNK;
                    }
                    break;
                case FIELD_SQUARE_LARGE_BOAT:
                    if (--field->largeBoatLives == 0) {
                        gData->result = RESULT_LARGE_BOAT_SUNK;
                    }
                    break;
                case FIELD_SQUARE_HUGE_BOAT:
                    if (--field->hugeBoatLives == 0) {
                        gData->result = RESULT_HUGE_BOAT_SUNK;
                    }
                    break;
                default:
                    break;
            }
            break;
        case FIELD_SQUARE_UNKNOWN:
        case FIELD_SQUARE_HIT:
        case FIELD_SQUARE_MISS:
        case FIELD_SQUARE_CURSOR:
        case FIELD_SQUARE_INVALID:
            break;
    }
    return previousStatus;
}

SquareStatus FieldUpdateKnowledge(Field *opp_field, const GuessData *own_guess) {
    // Retrieve the previous status of the square
    SquareStatus previousStatus = opp_field->grid[own_guess->row][own_guess->col];

    // Update the opponent's field based on the guess result
    switch (own_guess->result) {
        case RESULT_MISS:
            opp_field->grid[own_guess->row][own_guess->col] = FIELD_SQUARE_EMPTY;
            break;
        case RESULT_HIT:
            opp_field->grid[own_guess->row][own_guess->col] = FIELD_SQUARE_HIT;
            break;
        case RESULT_SMALL_BOAT_SUNK:
            opp_field->grid[own_guess->row][own_guess->col] = FIELD_SQUARE_HIT;
            opp_field->smallBoatLives = 0;
            break;
        case RESULT_MEDIUM_BOAT_SUNK:
            opp_field->grid[own_guess->row][own_guess->col] = FIELD_SQUARE_HIT;
            opp_field->mediumBoatLives = 0;
            break;
        case RESULT_LARGE_BOAT_SUNK:
            opp_field->grid[own_guess->row][own_guess->col] = FIELD_SQUARE_HIT;
            opp_field->largeBoatLives = 0;
            break;
        case RESULT_HUGE_BOAT_SUNK:
            opp_field->grid[own_guess->row][own_guess->col] = FIELD_SQUARE_HIT;
            opp_field->hugeBoatLives = 0;
            break;
        default:
            // If the result is not recognized, do not change the status
            break;
    }

    return previousStatus;
}


uint8_t FieldGetBoatStates(const Field *field) {
    uint8_t boatStates = 0;
    if (field->smallBoatLives > 0) boatStates |= 0x01;
    if (field->mediumBoatLives > 0) boatStates |= 0x02;
    if (field->largeBoatLives > 0) boatStates |= 0x04;
    if (field->hugeBoatLives > 0) boatStates |= 0x08;
    return boatStates;
}

uint8_t FieldAIPlaceAllBoats(Field *own_field) {
    int boat_types[4] = {FIELD_BOAT_TYPE_SMALL, FIELD_BOAT_TYPE_MEDIUM, FIELD_BOAT_TYPE_LARGE, FIELD_BOAT_TYPE_HUGE};
    int directions[2] = {FIELD_DIR_EAST, FIELD_DIR_SOUTH};

    for (int i = 0; i < 4; i++) {
        int boat_type = boat_types[i];
        bool placed = 0; // Initialize to false

        for (int row = 0; row < FIELD_ROWS; row++) {
            for (int col = 0; col < FIELD_COLS; col++) {
                for (int dir = 0; dir < 2; dir++) {
                    if (FieldAddBoat(own_field, row, col, directions[dir], boat_type) == SUCCESS) {
                        placed = 1;
                        break;
                    }
                }
                if (placed) break; // Break out of column loop
            }
            if (placed) break; // Break out of row loop
        }

        if (!placed) return STANDARD_ERROR; // If not placed, return error
    }

    return SUCCESS;
}

GuessData FieldAIDecideGuess(const Field *opp_field) {
    GuessData guess = {0, 0, FIELD_SQUARE_UNKNOWN};
    for (uint8_t row = 0; row < FIELD_ROWS; row++) {
        for (uint8_t col = 0; col < FIELD_COLS; col++) {
            if (opp_field->grid[row][col] == FIELD_SQUARE_UNKNOWN) {
                guess.row = row;
                guess.col = col;
                return guess;
            }
        }
    }
    return guess;
}
