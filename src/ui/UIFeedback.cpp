#include "UIFeedback.h"

UIFeedback::UIFeedback(uint8_t ledGreen, uint8_t ledRed, uint8_t buzzerPin)
    : ledG(ledGreen), ledR(ledRed), buzz(buzzerPin)
{
}

void UIFeedback::begin() {
    pinMode(ledG, OUTPUT);
    pinMode(ledR, OUTPUT);
    pinMode(buzz, OUTPUT);

    digitalWrite(ledG, LOW);
    digitalWrite(ledR, LOW);

    Serial.print(F("[UI] LED GREEN PIN="));
    Serial.print(ledG);
    Serial.print(F(" LED RED PIN="));
    Serial.print(ledR);
    Serial.print(F(" BUZZER PIN="));
    Serial.println(buzz);
}

void UIFeedback::signal(FeedbackType type) {

    Serial.print(F("[UI] Feedback: "));
    Serial.println(feedbackToString(type));

    switch (type) {

        case FeedbackType::ACCESS_GRANTED:
            blinkLED(ledG, 2, 150);
            beep(2000, 100);
            break;

        case FeedbackType::ACCESS_DENIED:
            blinkLED(ledR, 2, 200);
            beep(500, 200);
            break;

        case FeedbackType::BADGE_ADDED:
            blinkLED(ledG, 3, 120);
            beep(1800, 120);
            break;

        case FeedbackType::BADGE_DELETED:
            blinkLED(ledR, 3, 120);
            beep(1200, 120);
            break;

        case FeedbackType::SCAN_BADGE:
            blinkLED(ledG, 1, 400);
            beep(2500, 80);
            break;

        case FeedbackType::CONFIRM_RESET:
            blinkLED(ledR, 1, 500);
            beep(1000, 150);
            break;

        case FeedbackType::RESET_DONE:
            blinkLED(ledG, 4, 100);
            beep(2200, 200);
            break;

        case FeedbackType::LOCKED:
            blinkLED(ledR, 5, 100);
            beep(400, 500);
            break;

        case FeedbackType::CANCELLED:
            blinkLED(ledR, 1, 300);
            beep(800, 100);
            break;

        case FeedbackType::ERROR:
        default:
            blinkLED(ledR, 4, 100);
            beep(300, 400);
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

const char* UIFeedback::feedbackToString(FeedbackType type) {
    switch (type) {
        case FeedbackType::ACCESS_GRANTED: return "ACCESS_GRANTED";
        case FeedbackType::ACCESS_DENIED: return "ACCESS_DENIED";
        case FeedbackType::BADGE_ADDED: return "BADGE_ADDED";
        case FeedbackType::BADGE_DELETED: return "BADGE_DELETED";
        case FeedbackType::SCAN_BADGE: return "SCAN_BADGE";
        case FeedbackType::CONFIRM_RESET: return "CONFIRM_RESET";
        case FeedbackType::RESET_DONE: return "RESET_DONE";
        case FeedbackType::ERROR: return "ERROR";
        case FeedbackType::LOCKED: return "LOCKED";
        case FeedbackType::CANCELLED: return "CANCELLED";
        default: return "UNKNOWN";
    }
}
