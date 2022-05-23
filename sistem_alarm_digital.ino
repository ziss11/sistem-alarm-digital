#include <MD_MAX72xx_lib.h>
#include <MD_MAX72xx.h>
#include <MD_Parola_lib.h>
#include <MD_Parola.h>
#include <DS3231.h>
#include <Keypad.h>

#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
#define MAX_DEVICES 4
#define CLK_PIN 13
#define DATA_PIN 11
#define CS_PIN 10
#define LDR A1

#define NUM_ROWS 4
#define NUM_COLS 4

#define BUF_SIZE 75
#define DELAY 100

char keymap[NUM_ROWS][NUM_COLS] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'},
};

byte rowPins[NUM_ROWS] = {9, 8, 7, 6};
byte colPins[NUM_COLS] = {5, 4, 3, 2};

MD_Parola output = MD_Parola(HARDWARE_TYPE, DATA_PIN, CLK_PIN, CS_PIN, MAX_DEVICES);
Keypad myKeypad = Keypad(makeKeymap(keymap), rowPins, colPins, NUM_ROWS, NUM_COLS);

DS3231 rtc(SDA, SCL);
Time t;

uint8_t scrollSpeed = 25;
textEffect_t scrollEffectIn = PA_SCROLL_UP;
textEffect_t scrollEffectOut = PA_SCROLL_UP;
textEffect_t effectPrint = PA_PRINT;
textEffect_t effectNoEffect = PA_NO_EFFECT;
textPosition_t textAlign = PA_CENTER;
uint16_t scrollPause = 3000;

bool mainLoop = true;
bool timeSync = true;

bool systemMainLoop = true;
bool systemSettingsLoop = false;

int display_scene = 0;
int display_effect = 0;

char text[BUF_SIZE];

uint8_t intensity;

uint8_t timeHour;
uint8_t timeMinute;
uint8_t timeSecond;

String times;
uint8_t hour, minute, second;
int temp;

String name = "Azis";
String nrp = "7004";

uint8_t degC[] = {6, 3, 3, 56, 68, 68, 68};
String daysOfWeek[7] = {
    "Min"
    "Sen",
    "Sel",
    "Rab",
    "Kam",
    "Jum",
    "Sab",
};

char keypressed;
int pressedCount = 0;
char c1, c2;

byte setIntensityLed(int light)
{
    byte _value = 0;
    if (light >= 0 && light <= 127)
    {
        _value = 15;
    }
    else if (light >= 128 && light <= 319)
    {
        _value = 10;
    }
    else if (light >= 320 && light <= 512)
    {
        _value = 5;
    };
    return _value;
}

int formatYear(int longYear)
{
    return longYear % 100;
}

void setTimeAndTemp()
{
    hour = t.hour;
    minute = t.min;
    second = t.sec;
    temp = (int)rtc.getTemp();

    times = (String)hour + ':' + minute;
}

void setTimer()
{
    if (second == 10)
    {
    }
}

void setup()
{
    Serial.begin(9600);
    output.begin();
    rtc.begin();

    output.addChar('$', degC);
    output.setIntensity(1);

    pinMode(LDR, INPUT_PULLUP);

    for (byte i = 0; i < 10; i++)
    {
        text[i] = '\0';
    }
}

void loop()
{
    keypressed = myKeypad.getKey();
    intensity = setIntensityLed(analogRead(LDR));
    output.setIntensity(intensity);

    while (keypressed == NO_KEY)
    {
        if (output.displayAnimate())
        {
            output.displayText(text, textAlign, scrollSpeed, scrollPause, effectPrint, effectNoEffect);

            t = rtc.getTime();
            if (systemMainLoop == true && systemSettingsLoop == false)
            {
                if (mainLoop == true)
                {
                    output.setSpeed(scrollSpeed);
                    output.setPause(scrollPause);
                    output.setTextEffect(0, PA_SCROLL_DOWN, PA_NO_EFFECT);
                    mainLoop = false;
                }
                setTimeAndTemp();

                switch (display_scene)
                {
                case 0:
                    strcpy(text, times.c_str());
                    if (display_scene == 0)
                    {
                        output.setTextEffect(PA_OPENING, PA_NO_EFFECT);
                        display_effect = 1;
                    }
                    break;
                case 1:
                    dtostrf(temp, 0, 0, text);
                    strcat(text, " $");
                    if (display_scene == 0)
                    {
                        output.setTextEffect(PA_RANDOM, PA_NO_EFFECT);
                        display_effect = 0;
                    }
                    break;
                }

                if (second >= 0 && second <= 30)
                {
                    display_scene = 0;
                }
                else
                {
                    display_scene = 1;
                }
            }
            if (systemMainLoop == false && systemSettingsLoop == true)
            {
                if (mainLoop == false)
                {
                    output.setSpeed(scrollSpeed);
                    output.setPause(scrollPause);
                    output.setTextEffect(0, PA_SCROLL_UP, PA_NO_EFFECT);
                    mainLoop = true;
                }
                char keypressed2 = myKeypad.waitForKey();
            }
        }
    }

    if (keypressed == '*')
    {
        Serial.println(keypressed);
        systemMainLoop = false;
        systemSettingsLoop = true;

        text[0] = timeHour / 10 + 48;
        text[1] = timeHour % 10 + 48;
        text[2] = ':';
        text[3] = '-';
        text[4] = '-';

        char keypressed2 = myKeypad.waitForKey();
        if (keypressed2 != NO_KEY && keypressed2 != '*' && keypressed2 != '#' && keypressed2 != 'A' && keypressed2 != 'B' && keypressed2 != 'C' && keypressed2 != 'D')
        {
            c1 = keypressed2;
            text[0] = keypressed2;
        }
    }
}
