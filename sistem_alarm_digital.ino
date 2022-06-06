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

bool isOnAlarm1 = false;
bool isOnAlarm2 = false;
bool isOnAlarm3 = false;

bool isOnTime = false;
bool isOnTemp = false;

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

uint8_t timeHour;
uint8_t timeMinute;
uint8_t timeSecond;

uint8_t hourAlarm1;
uint8_t minuteAlarm1;

uint8_t hourAlarm2;
uint8_t minuteAlarm2;

uint8_t hourAlarm3;
uint8_t minuteAlarm3;

int durAlarm1Min, durAlarm2Min, durAlarm3Min = 0;
int durAlarm1Sec, durAlarm2Sec, durAlarm3Sec = 0;

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
int countMinute, countSecond = 0;

int setIntensityLed(int light)
{
    int value = 0;
    if (light <= 100)
    {
        value = 0;
    }
    else if (light >= 101 && light <= 200)
    {
        value = 5;
    }
    else if (light >= 201 && light <= 250)
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
    if (isOnTime)
    {
        strcpy(text, times.c_str());

        if (display_effect == 0)
        {
            output.setTextEffect(PA_SCROLL_DOWN, PA_NO_EFFECT);
            display_effect = 1;
        }
        else
        {
            isOnTime = false;
        }
        Serial.println("Jam");
    }
    else if (isOnTemp)
    {
        if (isOnAlarm1 == false || isOnAlarm2 == false || isOnAlarm3 == false)
        {
            temp = rtc.getTemp();
            dtostrf(temp, 0, 0, text);
            strcat(text, " $");

            if (display_effect == 1)
            {
                output.setTextEffect(PA_OPENING, PA_NO_EFFECT);
                output.setPause(5000);
                display_effect = 2;
            }
            else
            {
                isOnTemp = false;
            }
            Serial.println("Temp");
        }
    }
    else if (isOnAlarm1)
    {
        if (t.sec >= 60)
        {
            countMinute++;
        }

        if (countMinute <= durAlarm1Min && t.sec <= durAlarm1Sec)
        {
            strcpy(text, nrp.c_str());

            if (display_effect == 2)
            {
                output.setTextEffect(PA_SCROLL_LEFT, PA_NO_EFFECT);
                display_effect = 3;
            }
        }
        else
        {
            isOnAlarm1 = false;
            isOnTime = true;
            output.displayClear();
        }
        Serial.println("Alarm 1");
    }
    else if (isOnAlarm2)
    {
        strcpy(text, name.c_str());

        if (display_effect == 3)
        {
            output.setTextEffect(PA_SCROLL_LEFT, PA_NO_EFFECT);
            display_effect = 4;
        }
        else
        {
            isOnAlarm2 = false;
            isOnTime = true;
        }
        Serial.println("Alarm 2");
    }
    else if (isOnAlarm3)
    {
        strcpy(text, (nrp + ' ' + name).c_str());

        if (display_effect == 4)
        {
            output.setTextEffect(PA_SCROLL_LEFT, PA_NO_EFFECT);
            display_effect = 0;
        }
        else
        {
            isOnAlarm3 = false;
            isOnTime = true;
        }
        Serial.println("Alarm 3");
    }
}

void setScene()
{
    if (t.sec == 10 || t.sec == 40)
    {
        isOnTemp = true;
    }
    else if (t.hour == hourAlarm1 && t.min == minuteAlarm1)
    {
        isOnAlarm1 = true;
    }
    else if (t.hour == hourAlarm2 && t.min == minuteAlarm2)
    {
        isOnAlarm2 = true;
    }
    else if (t.hour == hourAlarm3 && t.min == minuteAlarm3)
    {
        isOnAlarm3 = true;
    }
    else
    {
        isOnTime = true;
    }
}

void setTime()
{
    timeHour = t.hour;
    timeMinute = t.min;
    temp = (int)rtc.getTemp();
    times = (String)timeHour + ':' + timeMinute;
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
                if (strlen(text) >= 8)
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
                setScene();
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
                            timeHour = inputHourString.toInt();

                            text[0] = timeHour / 10 + 48;
                            text[1] = timeHour % 10 + 48;
                            text[2] = ':';
                            text[3] = '-';
                            text[4] = '-';
                        }
                        else
                        {
                            inputMinuteString += keypressed2;
                            timeMinute = inputMinuteString.toInt();

                            text[0] = timeHour / 10 + 48;
                            text[1] = timeHour % 10 + 48;
                            text[2] = ':';
                            text[3] = timeMinute / 10 + 48;
                            text[4] = timeMinute % 10 + 48;
                        }

                        if (timeHour >= 24)
                        {
                            timeHour = 0;
                        }

                        if (timeMinute >= 60)
                        {
                            timeMinute = 0;
                            timeHour++;
                        }

                        output.displayText(text, textAlign, scrollSpeed, 0, PA_SCROLL_UP, PA_NO_EFFECT);
                    }
                    else if (keypressed2 == '*')
                    {
                        rtc.setTime(timeHour, timeMinute, t.sec);
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
                            timeHour = inputHourString.toInt();

                            text[0] = timeHour / 10 + 48;
                            text[1] = timeHour % 10 + 48;
                            text[2] = ':';
                            text[3] = '-';
                            text[4] = '-';
                        }
                        else
                        {
                            inputMinuteString += keypressed3;
                            timeMinute = inputMinuteString.toInt();

                            text[0] = timeHour / 10 + 48;
                            text[1] = timeHour % 10 + 48;
                            text[2] = ':';
                            text[3] = timeMinute / 10 + 48;
                            text[4] = timeMinute % 10 + 48;
                        }

                        if (timeHour >= 24)
                        {
                            timeHour = 0;
                        }

                        if (timeMinute >= 60)
                        {
                            timeMinute = 0;
                            timeHour++;
                        }

                        output.displayText(text, textAlign, scrollSpeed, 0, PA_SCROLL_UP, PA_NO_EFFECT);
                    }
                    else if (keypressed3 == '#')
                    {
                        timeHour = 0;
                        timeMinute = 0;

                        isSetAlarm = false;
                        setting(isSetAlarm);
                    }
                    else if (keypressed3 == 'A')
                    {
                        hourAlarm1 = timeHour;
                        minuteAlarm1 = timeMinute;

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
                        hourAlarm2 = timeHour;
                        minuteAlarm2 = timeMinute;

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
                        hourAlarm3 = timeHour;
                        minuteAlarm3 = timeMinute;

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
                        hourAlarm1 = 0;
                        minuteAlarm1 = 0;

                        hourAlarm2 = 0;
                        minuteAlarm2 = 0;

                        hourAlarm3 = 0;
                        minuteAlarm3 = 0;

                        isSetDuration = false;
                        setting(isSetDuration);
                    }
                    else if (keypressed4 == 'A')
                    {
                        output.displayScroll("Atur Alarm 1 berhasil", textAlign, PA_SCROLL_LEFT, scrollSpeed);

                        durAlarm1Min = durationMinute;
                        durAlarm1Sec = durationSecond;

                        Serial.println(durAlarm1Min);
                        Serial.println(durAlarm1Sec);

                        isOnAlarm1 = true;
                        isSetDuration = false;
                        inputDurMinuteString = "";
                        inputDurSecondString = "";
                        setting(isSetDuration);
                    }
                    else if (keypressed4 == 'B')
                    {
                        output.displayScroll("Atur Alarm 2 berhasil", textAlign, PA_SCROLL_LEFT, scrollSpeed);

                        durAlarm2Min = durationMinute;
                        durAlarm2Sec = durationSecond;

                        Serial.println(durAlarm2Min);
                        Serial.println(durAlarm2Sec);

                        isSetDuration = false;
                        inputDurMinuteString = "";
                        inputDurSecondString = "";
                        setting(isSetDuration);
                    }
                    else if (keypressed4 == 'C')
                    {
                        output.displayScroll("Atur Alarm 3 berhasil", textAlign, PA_SCROLL_LEFT, scrollSpeed);

                        durAlarm3Min = durationMinute;
                        durAlarm3Sec = durationSecond;

                        Serial.println(durAlarm3Min);
                        Serial.println(durAlarm3Sec);

                        isSetDuration = false;
                        inputDurMinuteString = "";
                        inputDurSecondString = "";
                        setting(isSetDuration);
                    }
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
