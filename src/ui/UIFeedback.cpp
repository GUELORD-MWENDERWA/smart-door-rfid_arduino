#include "UIFeedback.h"

UIFeedback::UIFeedback(uint8_t ledGreen, uint8_t ledRed, uint8_t buzzerPin)
    : ledG(ledGreen), ledR(ledRed), buzz(buzzerPin) {}

void UIFeedback::begin() {
    pinMode(ledG, OUTPUT);
    pinMode(ledR, OUTPUT);
    pinMode(buzz, OUTPUT);
    digitalWrite(ledG, LOW);
    digitalWrite(ledR, LOW);
    digitalWrite(buzz, LOW);
}

void UIFeedback::signal(FeedbackType type) {
    switch(type) {
        case FeedbackType::ACCESS_GRANTED:
            digitalWrite(ledG, HIGH);
            beep(1000, 100);
            delay(50);
            digitalWrite(ledG, LOW);
            break;

        case FeedbackType::ACCESS_DENIED:
            digitalWrite(ledR, HIGH);
            beep(300, 150);
            delay(100);
            digitalWrite(ledR, LOW);
            break;

        case FeedbackType::BADGE_ADDED:
            blinkLED(ledG, 2, 200);
            beep(1000, 150);
            break;

        case FeedbackType::BADGE_DELETED:
            blinkLED(ledR, 2, 200);
            beep(300, 150);
            break;

        case FeedbackType::RESET_DONE:
            digitalWrite(ledG, HIGH);
            digitalWrite(ledR, HIGH);
            beep(1000, 800);
            delay(200);
            digitalWrite(ledG, LOW);
            digitalWrite(ledR, LOW);
            break;

        case FeedbackType::ERROR:
            blinkLED(ledR, 3, 100);
            beep(300, 100);
            break;
    }
}

void UIFeedback::blinkLED(uint8_t ledPin, uint8_t times, uint16_t duration) {
    for (uint8_t i = 0; i < times; i++) {
        digitalWrite(ledPin, HIGH);
        delay(duration);
        digitalWrite(ledPin, LOW);
        delay(duration);
    }
}

void UIFeedback::beep(uint16_t freq, uint16_t duration) {
    tone(buzz, freq, duration);
}
