#include "UIFeedback.h"

#ifndef DEBUG_PRINTLN
#define DEBUG_PRINTLN(x) Serial.println(x)
#define DEBUG_PRINT(x) Serial.print(x)
#endif

UIFeedback::UIFeedback(uint8_t ledGreenPin, uint8_t ledRedPin, uint8_t buzzerPin)
    : ledGreen(ledGreenPin), ledRed(ledRedPin), buzzer(buzzerPin)
{
}

void UIFeedback::begin() {
    pinMode(ledGreen, OUTPUT);
    pinMode(ledRed, OUTPUT);
    digitalWrite(ledGreen, LOW);
    digitalWrite(ledRed, LOW);
    if (buzzer != 255) pinMode(buzzer, OUTPUT);
}

void UIFeedback::signal(FeedbackType t) {
    switch (t) {
        case FeedbackType::ACCESS_GRANTED:
            digitalWrite(ledGreen, HIGH);
            digitalWrite(ledRed, LOW);
            toneBeep(2000, 150);
            delay(150);
            digitalWrite(ledGreen, LOW);
            break;
        case FeedbackType::ACCESS_DENIED:
            digitalWrite(ledRed, HIGH);
            digitalWrite(ledGreen, LOW);
            toneBeep(400, 300);
            delay(300);
            digitalWrite(ledRed, LOW);
            break;
        case FeedbackType::SCAN_BADGE:
            // Blink green twice
            for (int i=0;i<2;i++) {
                digitalWrite(ledGreen, HIGH);
                delay(120);
                digitalWrite(ledGreen, LOW);
                delay(80);
            }
            break;
        case FeedbackType::BADGE_ADDED:
            // short success pattern
            digitalWrite(ledGreen, HIGH);
            toneBeep(1500, 80);
            delay(80);
            digitalWrite(ledGreen, LOW);
            break;
        case FeedbackType::BADGE_DELETED:
            digitalWrite(ledGreen, HIGH);
            toneBeep(1200, 80);
            delay(80);
            digitalWrite(ledGreen, LOW);
            break;
        case FeedbackType::RESET_DONE:
            digitalWrite(ledGreen, HIGH);
            toneBeep(2500, 120);
            delay(120);
            digitalWrite(ledGreen, LOW);
            break;
        case FeedbackType::CONFIRM_RESET:
            digitalWrite(ledRed, HIGH);
            toneBeep(600, 120);
            delay(120);
            digitalWrite(ledRed, LOW);
            break;
        case FeedbackType::ERROR:
            digitalWrite(ledRed, HIGH);
            toneBeep(400, 200);
            delay(200);
            digitalWrite(ledRed, LOW);
            break;
        case FeedbackType::CANCELLED:
            digitalWrite(ledGreen, HIGH);
            delay(80);
            digitalWrite(ledGreen, LOW);
            break;
        default:
            break;
    }
}

void UIFeedback::toneBeep(uint16_t freq, uint16_t duration) {
    if (buzzer == 255) return;
    tone(buzzer, freq, duration);
}