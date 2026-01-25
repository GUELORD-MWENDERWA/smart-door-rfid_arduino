#ifndef UI_FEEDBACK_H
#define UI_FEEDBACK_H

#include <Arduino.h>

/*
  UIFeedback
  - Helper to control LEDs and buzzer for feedback patterns
*/

enum class FeedbackType : uint8_t {
    ACCESS_GRANTED,
    ACCESS_DENIED,
    SCAN_BADGE,
    BADGE_ADDED,
    BADGE_DELETED,
    RESET_DONE,
    CONFIRM_RESET,
    ERROR,
    CANCELLED
};

class UIFeedback {
public:
    UIFeedback(uint8_t ledGreenPin, uint8_t ledRedPin, uint8_t buzzerPin = 255);

    void begin();
    void signal(FeedbackType t);

private:
    uint8_t ledGreen;
    uint8_t ledRed;
    uint8_t buzzer;

    void toneBeep(uint16_t freq, uint16_t duration);
};

#endif // UI_FEEDBACK_H