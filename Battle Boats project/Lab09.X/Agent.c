#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "Message.h"
#include "BattleBoats.h"
#include "Agent.h"
#include "Field.h"
#include "Negotiation.h"
#include "FieldOled.h"
#include "Oled.h"
#include <xc.h>
#include <stdbool.h>

// Define the current state and turn counter as global variables
static AgentState currentState;
static int turnCounter = 0;

#define BB_EVENT_SETUP_BOATS 0x01

// Agent initialization function
void AgentInit(void) {
    currentState = AGENT_STATE_START;
    turnCounter = 0;
}

// Define a struct to hold various agent values
typedef struct {
    NegotiationData rand1st;
    NegotiationData commitmentNum;
    NegotiationData rand2nd;
    Field agentField;
    Field opponentField;
    GuessData agentGuess;
    AgentState currentState;
    FieldOledTurn myTurn;
} AgentValues;

// Global variable to hold agent information
static AgentValues AgentInfo;

// Function declarations
void HandleStartState(BB_Event event, Message *messageToSend);
void HandleChallengingState(BB_Event event, Message *messageToSend);
void HandleAcceptingState(BB_Event event, Message *messageToSend);
void HandleDefendingState(BB_Event event, Message *messageToSend);
void HandleWaitingToSendState(BB_Event event, Message *messageToSend);

// Function to run the agent state machine
Message AgentRun(BB_Event event) {
    OledInit();
    Message messageToSend;
    messageToSend.type = MESSAGE_NONE;

    if (currentState == AGENT_STATE_START) {
        HandleStartState(event, &messageToSend);
    } else if (currentState == AGENT_STATE_CHALLENGING) {
        HandleChallengingState(event, &messageToSend);
    } else if (currentState == AGENT_STATE_ACCEPTING) {
        HandleAcceptingState(event, &messageToSend);
    } else if (currentState == AGENT_STATE_DEFENDING) {
        HandleDefendingState(event, &messageToSend);
    } else if (currentState == AGENT_STATE_WAITING_TO_SEND) {
        HandleWaitingToSendState(event, &messageToSend);
    } else if (currentState == AGENT_STATE_END_SCREEN || currentState == AGENT_STATE_SETUP_BOATS) {
        // No specific handling for these states
    } else {
        printf("Error, try again");
    }

    return messageToSend;
}

// Function to handle the start state
void HandleStartState(BB_Event event, Message *messageToSend) {
    if (event.type == BB_EVENT_START_BUTTON) {
        AgentInfo.rand1st = rand();
        AgentInfo.commitmentNum = NegotiationHash(AgentInfo.rand1st);

        messageToSend->type = MESSAGE_CHA;
        messageToSend->param0 = AgentInfo.commitmentNum;

        OledClear(OLED_COLOR_BLACK);
        char negotiationMessage[100];
        sprintf(negotiationMessage, "CHALLENGING\n %5d = A\n %5d = hash_A\n", AgentInfo.rand1st, AgentInfo.commitmentNum);
        OledDrawString(negotiationMessage);
        FieldInit(&AgentInfo.agentField, &AgentInfo.opponentField);
        FieldAIPlaceAllBoats(&AgentInfo.agentField);

        AgentSetState(AGENT_STATE_CHALLENGING);
    } else if (event.type == BB_EVENT_CHA_RECEIVED) {
        AgentInfo.commitmentNum = event.param0;
        AgentInfo.rand2nd = rand();

        messageToSend->type = MESSAGE_ACC;
        messageToSend->param0 = AgentInfo.rand2nd;

        OledClear(OLED_COLOR_BLACK);
        char negotiationMessage[100];
        sprintf(negotiationMessage, "ACCEPTING\n %5d = B\n %5d = hash_A\n", AgentInfo.rand2nd, AgentInfo.commitmentNum);
        OledDrawString(negotiationMessage);

        FieldInit(&AgentInfo.agentField, &AgentInfo.opponentField);
        FieldAIPlaceAllBoats(&AgentInfo.agentField);

        AgentSetState(AGENT_STATE_ACCEPTING);
    } else if (event.type == BB_EVENT_RESET_BUTTON) {
        AgentSetState(AGENT_STATE_START);
        OledClear(OLED_COLOR_BLACK);
        char endScreen[50];
        sprintf(endScreen, "START \nPress BTN4 to restart!");
        OledDrawString(endScreen);
    }
}

// Function to handle the challenging state
void HandleChallengingState(BB_Event event, Message *messageToSend) {
    if (event.type == BB_EVENT_ACC_RECEIVED) {
        AgentInfo.rand2nd = event.param0;

        messageToSend->type = MESSAGE_REV;
        messageToSend->param0 = AgentInfo.rand1st;

        NegotiationOutcome outcome = NegotiateCoinFlip(AgentInfo.rand1st, AgentInfo.rand2nd);

        if (outcome == HEADS) {
            AgentSetState(AGENT_STATE_WAITING_TO_SEND);
        } else {
            AgentInfo.myTurn = FIELD_OLED_TURN_THEIRS;
            AgentSetState(AGENT_STATE_DEFENDING);
        }
    } else if (event.type == BB_EVENT_RESET_BUTTON) {
        AgentSetState(AGENT_STATE_START);
        OledClear(OLED_COLOR_BLACK);
        char endScreen[50];
        sprintf(endScreen, "START \nPress BTN4 to restart!");
        OledDrawString(endScreen);
    }
}

// Function to handle the accepting state
void HandleAcceptingState(BB_Event event, Message *messageToSend) {
    if (event.type == BB_EVENT_ACC_RECEIVED) {
        currentState = AGENT_STATE_DEFENDING;
        messageToSend->type = MESSAGE_ACC;
        messageToSend->param0 = event.param0;
    }
}

// Function to handle the defending state
void HandleDefendingState(BB_Event event, Message *messageToSend) {
    if (event.type == BB_EVENT_SHO_RECEIVED) {
        // Register enemy attack
        AgentInfo.agentGuess.row = event.param0;
        AgentInfo.agentGuess.col = event.param1;

        SquareStatus result = FieldRegisterEnemyAttack(&AgentInfo.agentField, &AgentInfo.agentGuess);

        // Update OLED display
        OledClear(OLED_COLOR_BLACK);
        FieldOledDrawScreen(&AgentInfo.agentField, &AgentInfo.opponentField, AgentInfo.myTurn, turnCounter);

        // Prepare response message
        messageToSend->type = MESSAGE_RES;
        messageToSend->param0 = AgentInfo.agentGuess.row;
        messageToSend->param1 = AgentInfo.agentGuess.col;
        messageToSend->param2 = result;

        // Check boat statuses
        uint8_t boatStatus = FieldGetBoatStates(&AgentInfo.agentField);
        uint8_t oppStatus = FieldGetBoatStates(&AgentInfo.opponentField);

        if (boatStatus == 0x00 || oppStatus == 0x00) {
            // End the game if all boats are sunk
            AgentSetState(AGENT_STATE_END_SCREEN);
        } else {
            // Continue game otherwise
            AgentSetState(AGENT_STATE_WAITING_TO_SEND);
            AgentInfo.myTurn = FIELD_OLED_TURN_MINE;
        }

        // Clear the OLED display
        OledClear(OLED_COLOR_BLACK);
    } else if (event.type == BB_EVENT_RESET_BUTTON) {
        // Handle reset button
        AgentSetState(AGENT_STATE_START);
        OledClear(OLED_COLOR_BLACK);
        char endScreen[50];
        sprintf(endScreen, "START \nPress BTN4 to restart!");
        OledDrawString(endScreen);
    }
}

// Function to handle the waiting to send state
void HandleWaitingToSendState(BB_Event event, Message *messageToSend) {
    if (event.type == BB_EVENT_MESSAGE_SENT) {
        turnCounter++;

        AgentInfo.agentGuess = FieldAIDecideGuess(&AgentInfo.opponentField);

        messageToSend->type = MESSAGE_SHO;
        messageToSend->param0 = AgentInfo.agentGuess.row;
        messageToSend->param1 = AgentInfo.agentGuess.col;

        AgentSetState(AGENT_STATE_ATTACKING);

        OledClear(OLED_COLOR_BLACK);
        FieldOledDrawScreen(&AgentInfo.agentField, &AgentInfo.opponentField, AgentInfo.myTurn, turnCounter);
    } else if (event.type == BB_EVENT_RESET_BUTTON) {
        AgentSetState(AGENT_STATE_START);
        OledClear(OLED_COLOR_BLACK);
        char endScreen[50];
        sprintf(endScreen, "START \nPress BTN4 to restart!");
        OledDrawString(endScreen);
    }
}

// Function to get the current state of the agent
AgentState AgentGetState(void) {
    return currentState;
}

// Function to set the current state of the agent
void AgentSetState(AgentState newState) {
    currentState = newState;
}
