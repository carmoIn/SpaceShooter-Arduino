#ifndef MUSIC_SOUNDS_H

#define MUSIC_SOUNDS_H

#ifndef BUZZER_PIN
    #define BUZZER_PIN      4
#endif

void playLoseSound();
void playIntro();

void playSound(int16_t* melodyNotes, uint8_t tempo);

#endif