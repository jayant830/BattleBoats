#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "Agent.h"
#include "Message.h"
#include "BattleBoats.h"

int turnCounter;

// Function to mock a BB_Event
BB_Event CreateEvent(BB_EventType type, int param0, int param1) {
    BB_Event event;
    event.type = type;
    event.param0 = param0;
    event.param1 = param1;
    return event;
}

// Helper function to print test results
void PrintTestResult(const char *testName, int passed) {
    if (passed) {
        printf("PASS: %s\n", testName);
    } else {
        printf("FAIL: %s\n", testName);
    }
}

// Test Agent initialization
void TestAgentInit() {
    AgentInit();
    int passed = (AgentGetState() == AGENT_STATE_START && turnCounter == 0);
    PrintTestResult("TestAgentInit", passed);
}

// Test transition from START to CHALLENGING state
void TestStartToChallenging() {
    AgentInit();
    BB_Event startEvent = CreateEvent(BB_EVENT_START_BUTTON, 0, 0);
    Message msg = AgentRun(startEvent);

    int passed = (AgentGetState() == AGENT_STATE_CHALLENGING && msg.type == MESSAGE_CHA);
    PrintTestResult("TestStartToChallenging", passed);
}

// Test transition from CHALLENGING to ATTACKING state
void TestChallengingToAttacking() {
    AgentInit();
    AgentSetState(AGENT_STATE_CHALLENGING);
    BB_Event accReceivedEvent = CreateEvent(BB_EVENT_ACC_RECEIVED, 0, 0);
    Message msg = AgentRun(accReceivedEvent);

    int passed = (AgentGetState() == AGENT_STATE_ATTACKING && msg.type == MESSAGE_SHO);
    PrintTestResult("TestChallengingToAttacking", passed);
}

// Test transition from ATTACKING to DEFENDING state
void TestAttackingToDefending() {
    AgentInit();
    AgentSetState(AGENT_STATE_ATTACKING);
    BB_Event resReceivedEvent = CreateEvent(BB_EVENT_RES_RECEIVED, 0, 0);
    Message msg = AgentRun(resReceivedEvent);

    int passed = (AgentGetState() == AGENT_STATE_DEFENDING && msg.type == MESSAGE_NONE);
    PrintTestResult("TestAttackingToDefending", passed);
}

// Test transition from DEFENDING to WAITING_TO_SEND state
void TestDefendingToWaitingToSend() {
    AgentInit();
    AgentSetState(AGENT_STATE_DEFENDING);
    BB_Event shoReceivedEvent = CreateEvent(BB_EVENT_SHO_RECEIVED, 0, 0);
    Message msg = AgentRun(shoReceivedEvent);

    int passed = (AgentGetState() == AGENT_STATE_WAITING_TO_SEND && msg.type == MESSAGE_RES);
    PrintTestResult("TestDefendingToWaitingToSend", passed);
}

// Test transition from WAITING_TO_SEND to ATTACKING state
void TestWaitingToSendToAttacking() {
    AgentInit();
    AgentSetState(AGENT_STATE_WAITING_TO_SEND);
    BB_Event messageSentEvent = CreateEvent(BB_EVENT_MESSAGE_SENT, 0, 0);
    Message msg = AgentRun(messageSentEvent);

    int passed = (AgentGetState() == AGENT_STATE_ATTACKING && msg.type == MESSAGE_SHO);
    PrintTestResult("TestWaitingToSendToAttacking", passed);
}

// Test transition from START to ACCEPTING state
void TestStartToAccepting() {
    AgentInit();
    BB_Event chaReceivedEvent = CreateEvent(BB_EVENT_CHA_RECEIVED, 0, 0);
    Message msg = AgentRun(chaReceivedEvent);

    int passed = (AgentGetState() == AGENT_STATE_ACCEPTING && msg.type == MESSAGE_NONE);
    PrintTestResult("TestStartToAccepting", passed);
}

// Test transition from ACCEPTING to DEFENDING state
void TestAcceptingToDefending() {
    AgentInit();
    AgentSetState(AGENT_STATE_ACCEPTING);
    BB_Event accReceivedEvent = CreateEvent(BB_EVENT_ACC_RECEIVED, 0, 0);
    Message msg = AgentRun(accReceivedEvent);

    int passed = (AgentGetState() == AGENT_STATE_DEFENDING && msg.type == MESSAGE_ACC);
    PrintTestResult("TestAcceptingToDefending", passed);
}

// Main function to run all tests
int main() {
    srand(time(NULL)); // Initialize random seed
    
    AgentInit();
    TestAgentInit();
    TestStartToChallenging();
    TestChallengingToAttacking();
    TestAttackingToDefending();
    TestDefendingToWaitingToSend();
    TestWaitingToSendToAttacking();
    TestStartToAccepting();
    TestAcceptingToDefending();

    return 0;
}
