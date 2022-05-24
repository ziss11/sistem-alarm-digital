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
#define LDR A0

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

uint8_t scrollSpeed = 30;
textEffect_t scrollEffectIn = PA_SCROLL_UP;
textEffect_t scrollEffectOut = PA_SCROLL_UP;
textEffect_t effectPrint = PA_PRINT;
textEffect_t effectNoEffect = PA_NO_EFFECT;
textPosition_t textAlign = PA_CENTER;
uint16_t scrollPause = 3000;

bool mainLoop = true;

bool statusAlarm1 = false;
bool statusAlarm2 = false;
bool statusAlarm3 = false;

bool systemMainLoop = true;
bool systemSettingLoop = false;

int display_scene = 0;
int display_effect = 0;

char text[BUF_SIZE];

uint8_t intensity;

uint8_t hour;
uint8_t minute;
uint8_t second;

String times;
int temp;

int dur1, dur2, dur3 = 0;

String name = " Abdul Azis ";
String nrp = " 07211940007004 ";

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
int i1, i2;

int setIntensityLed(int light)
{
    int value = 0;
    if (light >= 0 && light <= 150)
    {
        value = 1;
    }
    else if (light >= 151 && light <= 250)
    {
        value = 5;
    }
    else
    {
        value = 10;
    }
    return value;
}

void adjustTime(String data)
{
    int _hour = data.substring(0, 2).toInt();
    int _minute = data.substring(3, 5).toInt();
    int _second = data.substring(6, 8).toInt();

    rtc.setTime(_hour, _minute, _second);
}

void displayScene()
{
    switch (display_scene)
    {
    case 0:
        temp = rtc.getTemp();
        dtostrf(temp, 0, 0, text);
        strcat(text, " $");

        if (display_effect == 0)
        {
            output.setTextEffect(PA_SCROLL_DOWN, effectNoEffect);
            output.setPause(3000);
            display_effect = 1;
        }
        break;
    case 1:
        strcpy(text, times.c_str());
        if (display_effect == 1)
        {
            output.setTextEffect(PA_OPENING, effectNoEffect);
            display_effect = 2;
        }
        break;
    case 2:
        strcpy(text, nrp.c_str());
        if (display_effect == 2)
        {
            output.setTextEffect(PA_SCROLL_LEFT, effectNoEffect);
            display_effect = 3;
        }
        break;
    case 3:
        strcpy(text, name.c_str());
        if (display_effect == 3)
        {
            output.setTextEffect(PA_SCROLL_LEFT, effectNoEffect);
            display_effect = 4;
        }
        break;
    case 4:
        String result = nrp + ';' + name;
        strcpy(text, result.c_str());
        if (display_effect == 4)
        {
            output.setTextEffect(PA_SCROLL_LEFT, effectNoEffect);
            display_effect = 0;
        }
        break;
    }
}

void readKeypad()
{
    if (keypressed == 'A')
    {
        systemMainLoop = false;
        systemSettingLoop = true;

        statusAlarm1 = !statusAlarm1;
        if (statusAlarm1)
        {
            output.displayScroll("Alarm 1 aktif", textAlign, PA_SCROLL_LEFT, PA_NO_EFFECT);
            output.setSpeed(40);
            systemMainLoop = true;
            systemSettingLoop = false;
        }
    }
    if (keypressed == 'B')
    {
        systemMainLoop = false;
        systemSettingLoop = true;

        statusAlarm2 = !statusAlarm2;
        if (statusAlarm2)
        {
            output.displayScroll("Alarm 2 aktif", textAlign, PA_SCROLL_LEFT, PA_NO_EFFECT);
            output.setSpeed(40);
            systemMainLoop = true;
            systemSettingLoop = false;
        }
    }
    if (keypressed == 'C')
    {
        systemMainLoop = false;
        systemSettingLoop = true;

        statusAlarm3 = !statusAlarm3;
        if (statusAlarm3)
        {
            output.displayScroll("Alarm 3 aktif", textAlign, PA_SCROLL_LEFT, PA_NO_EFFECT);
            output.setSpeed(40);
            systemMainLoop = true;
            systemSettingLoop = false;
        }
    }
    // if (keypressed == '*')
    // {
    //     char keypressed2 = myKeypad.getKey();
    //     output.displayScroll("Atur durasi alarm", textAlign, PA_SCROLL_LEFT, PA_NO_EFFECT);
    //     output.setSpeed(40);

    //     if (keypressed == 'A')
    //     {
    //         Serial.println("Atur durasi alarm 1");
    //     }
    // }
}

void setTime()
{
    hour = t.hour;
    minute = t.min;
    temp = (int)rtc.getTemp();
    times = (String)hour + ':' + minute;
}

void doTimer()
{
    Time timer = rtc.getTime();

    if (timer.sec == 10 || timer.sec == 40)
    {
        display_scene = 0;
        Serial.println("Suhu");
    }
    else if (timer.sec == 6)
    {
        if (statusAlarm1)
        {
            display_scene = 2;
            Serial.println("Alarm1");
        }
    }
    else if (timer.sec == 16)
    {
        if (statusAlarm2)
        {
            display_scene = 3;
            Serial.println("Alarm2");
        }
    }
    else if (timer.sec == 28)
    {
        if (statusAlarm3)
        {
            display_scene = 4;
            Serial.println("Alarm3");
        }
    }
    else
    {
        display_scene = 1;
        Serial.println("Jam");
    }
}

void setup()
{
    Serial.begin(9600);
    output.begin();
    rtc.begin();
    output.displayClear();

    output.addChar('$', degC);
    output.setIntensity(1);

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

    if (Serial.available() > 0)
    {
        adjustTime(Serial.readString());
    }
    readKeypad();

    if (output.displayAnimate())
    {
        if (systemMainLoop == true && systemSettingLoop == false)
        {
            if (strlen(text) >= 6)
            {
                output.displayText(text, textAlign, scrollSpeed, 0, PA_SCROLL_LEFT, effectNoEffect);
            }
            else
            {
                output.displayText(text, textAlign, scrollSpeed, 0, PA_PRINT, effectNoEffect);
            }

            t = rtc.getTime();

            if (mainLoop == true)
            {
                output.setSpeed(40);
                output.setPause(500);
                output.setTextEffect(0, PA_SCROLL_DOWN, PA_NO_EFFECT);
                mainLoop = false;
            }
            setTime();
            displayScene();
            doTimer();
        }
        if (systemMainLoop == false && systemSettingLoop == true)
        {
            if (mainLoop == false)
            {
                output.setSpeed(40);
                output.setPause(500);
                output.setTextEffect(0, PA_SCROLL_UP, PA_NO_EFFECT);
                mainLoop = true;
            }
        }
    }
}
