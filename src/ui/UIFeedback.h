#ifndef UI_FEEDBACK_H
#define UI_FEEDBACK_H

#include <Arduino.h>

enum class FeedbackType {
    ACCESS_GRANTED,
    ACCESS_DENIED,
    BADGE_ADDED,
    BADGE_DELETED,
    RESET_DONE,
    ERROR
};

class UIFeedback {
public:
    UIFeedback(uint8_t ledGreen, uint8_t ledRed, uint8_t buzzerPin);

    void begin();
    void signal(FeedbackType type);

private:
    uint8_t ledG;
    uint8_t ledR;
    uint8_t buzz;

    void blinkLED(uint8_t ledPin, uint8_t times, uint16_t duration);
    void beep(uint16_t freq, uint16_t duration);
};

#endif
