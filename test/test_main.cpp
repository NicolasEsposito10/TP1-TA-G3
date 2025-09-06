#include <Arduino.h>
#include <unity.h>

void setUp(void) {
}

void tearDown(void) {
}

void test_example() {
    TEST_ASSERT_EQUAL(1, 1);
}

void setup() {
    UNITY_BEGIN();
    RUN_TEST(test_example);
    UNITY_END();
}

void loop() {
}