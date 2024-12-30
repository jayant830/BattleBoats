#include <stdio.h>
#include <stdlib.h>
#include "Field.h"

static int passed, failed;

#define EXAMPLETEST(expr) \
    if (expr) { \
        printf("Test Passed: %s:%d\n", __FILE__, __LINE__); \
        passed++; \
    } else { \
        printf("Test failed: %s:%d: %s\n", __FILE__, __LINE__, #expr); \
        failed++; \
    }

void PrintField(const Field *f) {
    for (int row = 0; row < FIELD_ROWS; row++) {
        for (int col = 0; col < FIELD_COLS; col++) {
            printf("%d ", f->grid[row][col]);
        }
        printf("\n");
    }
    printf("\n");
}

void TestFieldInit() {
    Field myField, opponentField;
    FieldInit(&myField, &opponentField);
    int squareCount = 1;
    for (int row = 0; row < FIELD_ROWS; row++) {
        for (int col = 0; col < FIELD_COLS; col++) {
        EXAMPLETEST(FieldGetSquareStatus(&myField, row, col) == FIELD_SQUARE_EMPTY);
        EXAMPLETEST(FieldGetSquareStatus(&opponentField, row, col) == FIELD_SQUARE_UNKNOWN);
        printf("Number of squares tested %d\n", squareCount++);
        }
    }
}

void TestFieldAddBoat() {
    Field myField;
    FieldInit(&myField, NULL);

    EXAMPLETEST(FieldAddBoat(&myField, 0, 0, FIELD_DIR_EAST, FIELD_BOAT_TYPE_SMALL) == 1);
    EXAMPLETEST(FieldAddBoat(&myField, 1, 0, FIELD_DIR_EAST, FIELD_BOAT_TYPE_MEDIUM) == 1);
    EXAMPLETEST(FieldAddBoat(&myField, 2, 0, FIELD_DIR_EAST, FIELD_BOAT_TYPE_LARGE) == 1);
    EXAMPLETEST(FieldAddBoat(&myField, 3, 0, FIELD_DIR_EAST, FIELD_BOAT_TYPE_HUGE) == 1);


    EXAMPLETEST(FieldGetSquareStatus(&myField, 0, 0) == FIELD_SQUARE_SMALL_BOAT);
    EXAMPLETEST(FieldGetSquareStatus(&myField, 0, 2) == FIELD_SQUARE_SMALL_BOAT);
    EXAMPLETEST(FieldGetSquareStatus(&myField, 1, 0) == FIELD_SQUARE_MEDIUM_BOAT);
    EXAMPLETEST(FieldGetSquareStatus(&myField, 1, 3) == FIELD_SQUARE_MEDIUM_BOAT);
    EXAMPLETEST(FieldGetSquareStatus(&myField, 2, 0) == FIELD_SQUARE_LARGE_BOAT);
    EXAMPLETEST(FieldGetSquareStatus(&myField, 2, 4) == FIELD_SQUARE_LARGE_BOAT);
    EXAMPLETEST(FieldGetSquareStatus(&myField, 3, 0) == FIELD_SQUARE_HUGE_BOAT);
    EXAMPLETEST(FieldGetSquareStatus(&myField, 3, 5) == FIELD_SQUARE_HUGE_BOAT);
    PrintField(&myField);
}

void TestFieldRegisterEnemyAttack() {
    Field myField;
    GuessData guess;

    FieldInit(&myField, NULL);
    FieldAddBoat(&myField, 0, 0, FIELD_DIR_EAST, FIELD_BOAT_TYPE_SMALL);

    guess.row = 0;
    guess.col = 0;
    EXAMPLETEST(FieldRegisterEnemyAttack(&myField, &guess) == FIELD_SQUARE_SMALL_BOAT);
    EXAMPLETEST(guess.result == RESULT_HIT);
    EXAMPLETEST(FieldGetSquareStatus(&myField, 0, 0) == FIELD_SQUARE_HIT);
}

void TestFieldUpdateKnowledge() {
   
    Field opponentField;
    Field myField;
    GuessData guess;
    FieldInit(&myField, &opponentField);

    guess.row = 0;
    guess.col = 0;
    guess.result = FIELD_SQUARE_MISS;
    
    EXAMPLETEST(FieldUpdateKnowledge(&opponentField, &guess) == FIELD_SQUARE_UNKNOWN);

    
}

void TestFieldGetSquareStatus() {
    Field myField;
    FieldInit(&myField, NULL);

    EXAMPLETEST(FieldGetSquareStatus(&myField, 0, 0) == FIELD_SQUARE_EMPTY);
}

void TestFieldGetBoatStates() {
    Field myField;

    FieldInit(&myField, NULL);
    FieldAddBoat(&myField, 0, 0, FIELD_DIR_EAST, FIELD_BOAT_TYPE_SMALL);
    FieldAddBoat(&myField, 1, 0, FIELD_DIR_EAST, FIELD_BOAT_TYPE_MEDIUM);
    FieldAddBoat(&myField, 2, 0, FIELD_DIR_EAST, FIELD_BOAT_TYPE_LARGE);
    FieldAddBoat(&myField, 3, 0, FIELD_DIR_EAST, FIELD_BOAT_TYPE_HUGE);

    EXAMPLETEST(FieldGetBoatStates(&myField) == (FIELD_BOAT_STATUS_SMALL | FIELD_BOAT_STATUS_MEDIUM | FIELD_BOAT_STATUS_LARGE | FIELD_BOAT_STATUS_HUGE));
}

void TestFieldAI() {
    Field myField, opponentField;

    FieldInit(&myField, &opponentField);

    GuessData aiGuess = FieldAIDecideGuess(&opponentField);
    EXAMPLETEST(aiGuess.row < FIELD_ROWS && aiGuess.col < FIELD_COLS);
    printf("My Field:\n");
    PrintField(&myField);

    printf("Opponent's Field:\n");
    PrintField(&opponentField);

    EXAMPLETEST(FieldAIPlaceAllBoats(&myField) == 1);

}

int main() {
    passed = 0;
    failed = 0;
   
    TestFieldInit();
    TestFieldAddBoat();
    TestFieldRegisterEnemyAttack();
    TestFieldUpdateKnowledge();
    TestFieldGetSquareStatus();
    TestFieldGetBoatStates();
    TestFieldAI();

    printf("Passed: %d\n", passed);
    printf("Failed: %d\n", failed);

    return (failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
