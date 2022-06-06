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

bool isOnAlarm[4] = {
    false,
    false,
    false,
    false,
};

bool isOnTime = false;
bool isOnTemp = false;

bool systemMainLoop = true;
bool systemSettingLoop = false;

int display_scene = 0;
int display_effect = 0;

String inputHourString;
String inputMinuteString;

String inputDurSecondString;

char text[BUF_SIZE];

char alarmMessage[4][BUF_SIZE] = {
    "Alarm 1 berhasil diatur",
    "Alarm 2 berhasil diatur",
    "Alarm 3 berhasil diatur",
    "Alarm 4 berhasil diatur",
};

uint8_t timeHour;
uint8_t timeMinute;
uint8_t timeSecond;

uint8_t hourAlarm[4];
uint8_t minuteAlarm[4];

int durAlarm[4] = {
    0,
    0,
    0,
    0,
};

String infoAlarm[4];
String infoAllAlarm;

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
            output.setTextEffect(PA_OPENING, PA_NO_EFFECT);
            display_effect = 1;
        }
        else
        {
            isOnTime = false;
        }
    }
    else if (isOnTemp)
    {
        if (isOnAlarm[0] == false || isOnAlarm[1] == false || isOnAlarm[2] == false || isOnAlarm[3] == false)
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
        }
    }
    else if (isOnAlarm[0])
    {
        strcpy(text, nrp.c_str());

        if (display_effect == 2)
        {
            output.setTextEffect(PA_SCROLL_LEFT, PA_NO_EFFECT);
            display_effect = 3;
        }
        else
        {
            isOnAlarm[0] = false;
        }
        Serial.println("Alarm 1");
    }
    else if (isOnAlarm[1])
    {
        strcpy(text, name.c_str());

        if (display_effect == 3)
        {
            output.setTextEffect(PA_SCROLL_LEFT, PA_NO_EFFECT);
            display_effect = 4;
        }
        else
        {
            isOnAlarm[1] = false;
        }
        Serial.println("Alarm 2");
    }
    else if (isOnAlarm[2])
    {
        strcpy(text, (nrp + "; " + name).c_str());

        if (display_effect == 4)
        {
            output.setTextEffect(PA_SCROLL_LEFT, PA_NO_EFFECT);
            display_effect = 5;
        }
        else
        {
            isOnAlarm[2] = false;
        }
        Serial.println("Alarm 3");
    }
    else if (isOnAlarm[3])
    {
        infoAlarm[0] = (String)' ' + hourAlarm[0] + ':' + minuteAlarm[0] + " (" + durAlarm[0] + ");";
        infoAlarm[1] = (String)' ' + hourAlarm[1] + ':' + minuteAlarm[1] + " (" + durAlarm[1] + ");";
        infoAlarm[2] = (String)' ' + hourAlarm[2] + ':' + minuteAlarm[2] + " (" + durAlarm[2] + ");";
        infoAlarm[3] = (String)' ' + hourAlarm[3] + ':' + minuteAlarm[3] + " (" + durAlarm[3] + ");";

        infoAllAlarm = infoAlarm[0] + infoAlarm[1] + infoAlarm[2] + infoAlarm[3] + ' ';

        strcpy(text, infoAllAlarm.c_str());

        if (display_effect == 5)
        {
            output.setTextEffect(PA_SCROLL_LEFT, PA_NO_EFFECT);
            display_effect = 0;
        }
        else
        {
            isOnAlarm[3] = false;
        }
        Serial.println("Alarm 4");
    }
}

void setScene()
{
    if (t.sec == 10 || t.sec == 40)
    {
        isOnTemp = true;
    }
    else if (t.hour == hourAlarm[0] && t.min == minuteAlarm[0] && t.sec <= durAlarm[0])
    {
        isOnAlarm[0] = true;
    }
    else if (t.hour == hourAlarm[1] && t.min == minuteAlarm[1] && t.sec <= durAlarm[1])
    {
        isOnAlarm[1] = true;
    }
    else if (t.hour == hourAlarm[2] && t.min == minuteAlarm[2] && t.sec <= durAlarm[2])
    {
        isOnAlarm[2] = true;
    }
    else if (t.hour == hourAlarm[3] && t.min == minuteAlarm[3] && t.sec <= durAlarm[3])
    {
        isOnAlarm[3] = true;
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
    timeSecond = t.sec;
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
    uint8_t intensity = setIntensityLed(analogRead(LDR));
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
                        inputHourString = "";
                        inputMinuteString = "";

                        isSetAlarm = false;
                        setting(isSetAlarm);
                    }
                    else if (keypressed3 == 'A')
                    {
                        hourAlarm[0] = timeHour;
                        minuteAlarm[0] = timeMinute;

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
                            text[2] = '\0';
                            text[3] = '\0';
                            text[4] = '\0';

                            output.displayText(text, textAlign, scrollSpeed, 0, PA_SCROLL_UP, PA_NO_EFFECT);
                            setting(isSetDuration);
                        }
                    }
                    else if (keypressed3 == 'B')
                    {
                        hourAlarm[1] = timeHour;
                        minuteAlarm[1] = timeMinute;

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
                            text[2] = '\0';
                            text[3] = '\0';
                            text[4] = '\0';

                            output.displayText(text, textAlign, scrollSpeed, 0, PA_SCROLL_UP, PA_NO_EFFECT);
                            setting(isSetDuration);
                        }
                    }
                    else if (keypressed3 == 'C')
                    {
                        hourAlarm[2] = timeHour;
                        minuteAlarm[2] = timeMinute;

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
                            text[2] = '\0';
                            text[3] = '\0';
                            text[4] = '\0';

                            output.displayText(text, textAlign, scrollSpeed, 0, PA_SCROLL_UP, PA_NO_EFFECT);
                            setting(isSetDuration);
                        }
                    }
                    else if (keypressed3 == 'D')
                    {
                        hourAlarm[3] = timeHour;
                        minuteAlarm[3] = timeMinute;

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
                            text[2] = '\0';
                            text[3] = '\0';
                            text[4] = '\0';

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
                        if (inputDurSecondString.length() < 2)
                        {
                            inputDurSecondString += keypressed4;
                            durationSecond = inputDurSecondString.toInt();

                            text[0] = durationSecond / 10 + 48;
                            text[1] = durationSecond % 10 + 48;
                            text[2] = '\0';
                            text[3] = '\0';
                            text[4] = '\0';
                        }

                        output.displayText(text, textAlign, scrollSpeed, 0, PA_SCROLL_UP, PA_NO_EFFECT);
                    }
                    else if (keypressed4 == '#')
                    {
                        inputDurSecondString = "";

                        isSetDuration = false;
                        setting(isSetDuration);
                    }
                    else if (keypressed4 == 'A')
                    {
                        output.displayScroll(alarmMessage[0], textAlign, PA_SCROLL_LEFT, scrollSpeed);

                        durAlarm[0] = durationSecond;

                        isOnAlarm[0] = true;
                        isSetDuration = false;
                        inputDurSecondString = "";
                        setting(isSetDuration);
                    }
                    else if (keypressed4 == 'B')
                    {
                        output.displayScroll(alarmMessage[1], textAlign, PA_SCROLL_LEFT, scrollSpeed);

                        durAlarm[1] = durationSecond;

                        isOnAlarm[1] = true;
                        isSetDuration = false;
                        inputDurSecondString = "";
                        setting(isSetDuration);
                    }
                    else if (keypressed4 == 'C')
                    {
                        output.displayScroll(alarmMessage[2], textAlign, PA_SCROLL_LEFT, scrollSpeed);

                        durAlarm[2] = durationSecond;

                        isOnAlarm[2] = true;
                        isSetDuration = false;
                        inputDurSecondString = "";
                        setting(isSetDuration);
                    }
                    else if (keypressed4 == 'D')
                    {
                        output.displayScroll(alarmMessage[3], textAlign, PA_SCROLL_LEFT, scrollSpeed);

                        durAlarm[3] = durationSecond;

                        isOnAlarm[3] = true;
                        isSetDuration = false;
                        inputDurSecondString = "";
                        setting(isSetDuration);
                    }
                }
            }
        }
    }
    if (keypressed == '*')
    {
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
