// std includes

// test includes
#include "Test_common_includes.h"

// project includes

// defines
#define TRACE_OUTPUT
#define CAN_ID_1 0x123
#define CAN_ID_2 0x321

// variables

const static uint8_t node1Increment = 10;
const static uint8_t node2Increment = 1;
const static uint8_t canDataLengthMax = 8;

static uint8_t node1Data[canDataLengthMax];
static uint8_t receivedData[canDataLengthMax];
static uint8_t canDataLength = canDataLengthMax;

volatile bool newDataReceivedNode1 = false;

// test feature includes requiring the above defined variables
extern CANXMC CAN;

void receiveEventNode1(int packetSize) {
    uint8_t count = 0;
    while (CAN.available() && count < 8) {
        receivedData[count++] = CAN.read();
    }
    newDataReceivedNode1 = true;
}

// Method invoked before a test suite is run.
void CAN_connected_node1_suiteSetup() {
    CAN.begin();
    CAN.filter(CAN_ID_2, 0x7FF); // Set filter to receive messages with CAN_ID_2
    CAN.onReceive(receiveEventNode1);
}

// Method invoked after a test suite is run.
void CAN_connected_node1_suiteTearDown() { CAN.end(); }

// define test group name
TEST_GROUP(CAN_connected_node1);
TEST_GROUP(CAN_connected_node1Internal);

// Setup method called by Unity before every individual test defined for this test group.
static TEST_SETUP(CAN_connected_node1Internal) {
    memset(node1Data, 0, sizeof(node1Data));
    memset(receivedData, 0, sizeof(receivedData));
}

// Tear down method called by Unity after every individual test defined for this test group.
static TEST_TEAR_DOWN(CAN_connected_node1Internal) {}

void testSendReceive(uint8_t dataLength, uint8_t iterations) {
    canDataLength = dataLength;

    for (uint8_t loop = 0; loop < iterations; ++loop) {
        TEST_ASSERT_TRUE(CAN.beginPacket(CAN_ID_1));
        TEST_ASSERT_EQUAL_UINT8(canDataLength, CAN.write(node1Data, canDataLength));
        TEST_ASSERT_EQUAL(1, CAN.endPacket());

        while (!newDataReceivedNode1) {
        }

        if (newDataReceivedNode1) {
            for (uint8_t i = 0; i < canDataLength; ++i) {
                TEST_ASSERT_EQUAL_UINT8(node1Data[i] + node2Increment, receivedData[i]);
            }
            newDataReceivedNode1 = false;

#ifdef TRACE_OUTPUT
            printArray("\nSent Data", node1Data, canDataLength);
            printArray("Received Data", receivedData, canDataLength);
#endif
            for (uint8_t i = 0; i < canDataLength; ++i) {
                node1Data[i] = receivedData[i] + node1Increment;
            }
        }
    }
}

TEST_IFX(CAN_connected_node1Internal, checkPingPong) { testSendReceive(3, 5); }

static TEST_GROUP_RUNNER(CAN_connected_node1Internal) {
    RUN_TEST_CASE(CAN_connected_node1Internal, checkPingPong);
}

// Bundle all tests to be executed for this test group
TEST_GROUP_RUNNER(CAN_connected_node1) {
    CAN_connected_node1_suiteSetup();

    RUN_TEST_GROUP(CAN_connected_node1Internal);

    CAN_connected_node1_suiteTearDown();
}
