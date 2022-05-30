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
textPosition_t textAlign = PA_CENTER;
uint16_t scrollPause = 3000;

bool isSetTime = false;
bool isSetAlarm = false;
bool isSetDuration = false;

bool mainLoop = true;

bool isAlarm1 = false;
bool isAlarm2 = false;
bool isAlarm3 = false;

bool systemMainLoop = true;
bool systemSettingLoop = false;

int display_scene = 0;
int display_effect = 0;

String inputHourString;
String inputMinuteString;

String inputDurMinuteString;
String inputDurSecondString;

char text[BUF_SIZE];

char alarm1Message[BUF_SIZE] = "Alarm 1 berhasil diatur";
char alarm2Message[BUF_SIZE] = "Alarm 2 berhasil diatur";
char alarm3Message[BUF_SIZE] = "Alarm 3 berhasil diatur";

uint8_t intensity;

uint8_t hour;
uint8_t minute;
uint8_t second;

uint8_t hourAlarm1;
uint8_t minuteAlarm1;

uint8_t hourAlarm2;
uint8_t minuteAlarm2;

uint8_t hourAlarm3;
uint8_t minuteAlarm3;

int durLimit = 0;
int durAlarm1, durAlarm2, durAlarm3 = 0;

int durationMinute = 0;
int durationSecond = 0;

String times;
int temp;

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
    if (t.sec == 10 || t.sec == 40)
    {
        temp = rtc.getTemp();
        dtostrf(temp, 0, 0, text);
        strcat(text, " $");

        if (display_effect == 1)
        {
            output.setTextEffect(PA_OPENING, PA_NO_EFFECT);
            display_effect = 2;
        }
    }
    else if (t.hour == hourAlarm1 && t.min == minuteAlarm1)
    {
        strcpy(text, nrp.c_str());
        if (display_effect == 2)
        {
            output.setTextEffect(PA_SCROLL_LEFT, PA_NO_EFFECT);
            display_effect = 3;
        }
        Serial.println("Alarm 1");
    }
    else if (t.hour == hourAlarm2 && t.min == minuteAlarm2)
    {
        strcpy(text, name.c_str());

        if (display_effect == 3)
        {
            output.setTextEffect(PA_SCROLL_LEFT, PA_NO_EFFECT);
            display_effect = 4;
        }
        Serial.println("Alarm 2");
    }
    else if (t.hour == hourAlarm3 && t.min == minuteAlarm3)
    {
        strcpy(text, (nrp + ' ' + name).c_str());

        if (display_effect == 4)
        {
            output.setTextEffect(PA_SCROLL_LEFT, PA_NO_EFFECT);
            display_effect = 0;
        }
        Serial.println("Alarm 3");
    }
    else
    {
        strcpy(text, times.c_str());

        if (display_effect == 0)
        {
            output.setTextEffect(PA_SCROLL_DOWN, PA_NO_EFFECT);
            display_effect = 1;
        }
        Serial.println("Jam");
    }
}

void setTime()
{
    hour = t.hour;
    minute = t.min;
    temp = (int)rtc.getTemp();
    times = (String)hour + ':' + minute;
}

void setting(bool b)
{
    if (b)
    {
        systemMainLoop = false;
        systemSettingLoop = true;
    }
    else
    {
        systemMainLoop = true;
        systemSettingLoop = false;
    }
}

void textTransition()
{
    while (1)
    {
        if (output.displayAnimate())
        {
            output.displayReset();
            break;
        }
    }
}

void playAnimate()
{
    output.displayAnimate();
    keypressed = myKeypad.getKey();
    intensity = setIntensityLed(analogRead(LDR));
    output.setIntensity(intensity);

    if (Serial.available() > 0)
    {
        adjustTime(Serial.readString());
    }
    if (keypressed == NO_KEY)
    {
        if (output.displayAnimate())
        {
            if (systemMainLoop == true && systemSettingLoop == false)
            {
                if (strlen(text) >= 6)
                {
                    output.displayText(text, textAlign, scrollSpeed, 0, PA_SCROLL_LEFT, PA_NO_EFFECT);
                }
                else
                {
                    output.displayText(text, textAlign, scrollSpeed, 0, PA_PRINT, PA_NO_EFFECT);
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

                if (isSetTime)
                {
                    char keypressed2 = myKeypad.waitForKey();

                    if (keypressed2 >= '0' && keypressed2 <= '9')
                    {
                        Serial.println(keypressed2);
                        if (inputHourString.length() < 2)
                        {
                            inputHourString += keypressed2;
                            hour = inputHourString.toInt();

                            text[0] = hour / 10 + 48;
                            text[1] = hour % 10 + 48;
                            text[2] = ':';
                            text[3] = '-';
                            text[4] = '-';
                        }
                        else
                        {
                            inputMinuteString += keypressed2;
                            minute = inputMinuteString.toInt();

                            text[0] = hour / 10 + 48;
                            text[1] = hour % 10 + 48;
                            text[2] = ':';
                            text[3] = minute / 10 + 48;
                            text[4] = minute % 10 + 48;
                        }

                        if (hour >= 24)
                        {
                            hour = 0;
                        }

                        if (minute >= 60)
                        {
                            minute = 0;
                            hour++;
                        }

                        output.displayText(text, textAlign, scrollSpeed, 0, PA_SCROLL_UP, PA_NO_EFFECT);
                    }
                    else if (keypressed2 == '*')
                    {
                        rtc.setTime(hour, minute, second);
                        isSetTime = false;
                        inputHourString = "";
                        inputMinuteString = "";
                        setting(isSetTime);
                    }
                }

                if (isSetAlarm)
                {
                    char keypressed3 = myKeypad.waitForKey();

                    if (keypressed3 >= '0' && keypressed3 <= '9')
                    {
                        if (inputHourString.length() < 2)
                        {
                            inputHourString += keypressed3;
                            hour = inputHourString.toInt();

                            text[0] = hour / 10 + 48;
                            text[1] = hour % 10 + 48;
                            text[2] = ':';
                            text[3] = '-';
                            text[4] = '-';
                        }
                        else
                        {
                            inputMinuteString += keypressed3;
                            minute = inputMinuteString.toInt();

                            text[0] = hour / 10 + 48;
                            text[1] = hour % 10 + 48;
                            text[2] = ':';
                            text[3] = minute / 10 + 48;
                            text[4] = minute % 10 + 48;
                        }

                        if (hour >= 24)
                        {
                            hour = 0;
                        }

                        if (minute >= 60)
                        {
                            minute = 0;
                            hour++;
                        }

                        output.displayText(text, textAlign, scrollSpeed, 0, PA_SCROLL_UP, PA_NO_EFFECT);
                    }
                    else if (keypressed3 == '#')
                    {
                        hour = NULL;
                        minute = NULL;

                        isSetAlarm = false;
                        setting(isSetAlarm);
                    }
                    else if (keypressed3 == 'A')
                    {
                        hourAlarm1 = hour;
                        minuteAlarm1 = minute;

                        isSetAlarm = false;
                        inputHourString = "";
                        inputMinuteString = "";

                        isSetDuration = !isSetDuration;

                        if (isSetDuration)
                        {
                            output.displayScroll("Atur Durasi", textAlign, PA_SCROLL_LEFT, scrollSpeed);
                            textTransition();

                            text[0] = '-';
                            text[1] = '-';
                            text[2] = ':';
                            text[3] = '-';
                            text[4] = '-';

                            output.displayText(text, textAlign, scrollSpeed, 0, PA_SCROLL_UP, PA_NO_EFFECT);
                            setting(isSetDuration);
                        }
                    }
                    else if (keypressed3 == 'B')
                    {
                        hourAlarm2 = hour;
                        minuteAlarm2 = minute;

                        isSetAlarm = false;
                        inputHourString = "";
                        inputMinuteString = "";

                        isSetDuration = !isSetDuration;

                        if (isSetDuration)
                        {
                            output.displayScroll("Atur Durasi", textAlign, PA_SCROLL_LEFT, scrollSpeed);
                            textTransition();

                            text[0] = '-';
                            text[1] = '-';
                            text[2] = ':';
                            text[3] = '-';
                            text[4] = '-';

                            output.displayText(text, textAlign, scrollSpeed, 0, PA_SCROLL_UP, PA_NO_EFFECT);
                            setting(isSetDuration);
                        }
                    }
                    else if (keypressed3 == 'C')
                    {
                        hourAlarm3 = hour;
                        minuteAlarm3 = minute;

                        isSetAlarm = false;
                        inputHourString = "";
                        inputMinuteString = "";

                        isSetDuration = !isSetDuration;

                        if (isSetDuration)
                        {
                            output.displayScroll("Atur Durasi", textAlign, PA_SCROLL_LEFT, scrollSpeed);
                            textTransition();

                            text[0] = '-';
                            text[1] = '-';
                            text[2] = ':';
                            text[3] = '-';
                            text[4] = '-';

                            output.displayText(text, textAlign, scrollSpeed, 0, PA_SCROLL_UP, PA_NO_EFFECT);
                            setting(isSetDuration);
                        }
                    }
                }

                if (isSetDuration)
                {
                    char keypressed4 = myKeypad.getKey();

                    if (keypressed4 >= '0' && keypressed4 <= '9')
                    {
                        if (inputDurMinuteString.length() < 2)
                        {
                            inputDurMinuteString += keypressed4;
                            durationMinute = inputDurMinuteString.toInt();

                            text[0] = durationMinute / 10 + 48;
                            text[1] = durationMinute % 10 + 48;
                            text[2] = ':';
                            text[3] = '-';
                            text[4] = '-';
                        }
                        else
                        {
                            inputDurSecondString += keypressed4;
                            durationSecond = inputDurSecondString.toInt();

                            text[0] = durationMinute / 10 + 48;
                            text[1] = durationMinute % 10 + 48;
                            text[2] = ':';
                            text[3] = durationSecond / 10 + 48;
                            text[4] = durationSecond % 10 + 48;
                        }

                        output.displayText(text, textAlign, scrollSpeed, 0, PA_SCROLL_UP, PA_NO_EFFECT);
                    }
                    else if (keypressed4 == '#')
                    {
                        hourAlarm1 = NULL;
                        minuteAlarm1 = NULL;

                        hourAlarm2 = NULL;
                        minuteAlarm2 = NULL;

                        hourAlarm3 = NULL;
                        minuteAlarm3 = NULL;

                        isSetDuration = false;
                        setting(isSetDuration);
                    }
                    else if (keypressed4 == 'A')
                    {
                        output.displayScroll("Atur Alarm 1 berhasil", textAlign, PA_SCROLL_LEFT, scrollSpeed);

                        int result = (durationMinute * 60) + durationSecond;

                        Serial.println(result);

                        isSetDuration = false;
                        inputDurMinuteString = "";
                        inputDurSecondString = "";
                        setting(isSetDuration);
                    }
                    // else if (keypressed4 == 'B')
                    // {
                    //     output.displayScroll("Atur Alarm 2 berhasil", textAlign, PA_SCROLL_LEFT, scrollSpeed);
                    //     int result = (durationMinute * 60) + durationSecond;
                    //     durAlarm2 = result;

                    //     Serial.println(durAlarm2);

                    //     isSetDuration = false;
                    //     inputDurMinuteString = "";
                    //     inputDurSecondString = "";
                    //     setting(isSetDuration);
                    // }
                    // else if (keypressed4 == 'C')
                    // {
                    //     output.displayScroll("Atur Alarm 3 berhasil", textAlign, PA_SCROLL_LEFT, scrollSpeed);
                    //     int result = (durationMinute * 60) + durationSecond;
                    //     durAlarm3 = result;

                    //     Serial.println(durAlarm3);

                    //     isSetDuration = false;
                    //     inputDurMinuteString = "";
                    //     inputDurSecondString = "";
                    //     setting(isSetDuration);
                    // }
                }
            }
        }
    }
    if (keypressed == '*')
    {
        Serial.println(keypressed);

        isSetTime = !isSetTime;
        if (isSetTime)
        {
            output.displayScroll("Atur Waktu", textAlign, PA_SCROLL_LEFT, scrollSpeed);
            textTransition();

            text[0] = '-';
            text[1] = '-';
            text[2] = ':';
            text[3] = '-';
            text[4] = '-';

            output.displayText(text, textAlign, scrollSpeed, 0, PA_SCROLL_UP, PA_NO_EFFECT);
            setting(isSetTime);
        }
        else
        {
            strcpy(text, times.c_str());
            output.displayText(text, textAlign, scrollSpeed, scrollPause, PA_SCROLL_UP, PA_NO_EFFECT);
            setting(isSetTime);
        }
    }
    else if (keypressed == '#')
    {
        Serial.println(keypressed);

        isSetAlarm = !isSetAlarm;
        if (isSetAlarm)
        {
            output.displayScroll("Atur Alarm", textAlign, PA_SCROLL_LEFT, scrollSpeed);
            textTransition();

            text[0] = '-';
            text[1] = '-';
            text[2] = ':';
            text[3] = '-';
            text[4] = '-';

            output.displayText(text, textAlign, scrollSpeed, 0, PA_SCROLL_UP, PA_NO_EFFECT);
            setting(isSetAlarm);
        }
        else
        {
            strcpy(text, times.c_str());
            output.displayText(text, textAlign, scrollSpeed, scrollPause, PA_SCROLL_UP, PA_NO_EFFECT);
            setting(isSetAlarm);
        }
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

    inputHourString.reserve(2);
    inputMinuteString.reserve(2);

    inputDurMinuteString.reserve(2);
    inputDurSecondString.reserve(2);

    for (byte i = 0; i < 10; i++)
    {
        text[i] = '\0';
    }
}

void loop()
{
    playAnimate();
}
