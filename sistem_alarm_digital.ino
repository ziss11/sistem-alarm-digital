#include <MD_MAX72xx_lib.h>
#include <MD_MAX72xx.h>
#include <MD_Parola_lib.h>
#include <MD_Parola.h>
#include <DS3231.h>

#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
#define MAX_DEVICES 4
#define CLK_PIN 13
#define DATA_PIN 11
#define CS_PIN 10
#define LDR A1

#define BUF_SIZE 75
#define DELAY 100

MD_Parola output = MD_Parola(HARDWARE_TYPE, DATA_PIN, CLK_PIN, CS_PIN, MAX_DEVICES);
DS3231 rtc(SDA, SCL);
Time t;

uint8_t scrollSpeed = 25;
textEffect_t scrollEffectIn = PA_SCROLL_UP;
textEffect_t scrollEffectOut = PA_SCROLL_UP;
textEffect_t effectPrint = PA_PRINT;
textEffect_t effectNoEffect = PA_NO_EFFECT;
textPosition_t textAlign = PA_CENTER;
uint16_t scrollPause = 3000;

bool systemMainLoop = true;
bool systemSettingsLoop = false;

bool mainLoop = true;
bool timeSync = true;

int display_scene = 0;
int display_effect = 0;

char text[BUF_SIZE];

uint8_t intensity;

uint8_t timeHour;
uint8_t timeMinute;
uint8_t timeSecond;
uint8_t timeDay;
uint8_t timeMonth;
uint8_t timeDate;
word timeYear;

String day, dates, times;
uint8_t dow, hour, minute, second, date, month, year;
int temp;

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

void adjustClock(String data)
{
    byte _day = data.substring(0, 2).toInt();
    byte _month = data.substring(3, 5).toInt();
    byte _year = data.substring(6, 10).toInt();
    byte _hour = data.substring(11, 13).toInt();
    byte _minute = data.substring(14, 16).toInt();
    byte _second = data.substring(17, 19).toInt();

    rtc.setTime(_hour, _minute, _second);
    rtc.setDate(_day, _month, _year);
    Serial.println("Berhasil");
}

void setTimeAndTemp()
{
    dow = t.dow;
    day = daysOfWeek[dow];
    hour = t.hour;
    minute = t.min;
    second = t.sec;
    date = t.date;
    month = t.mon;
    year = t.year;
    temp = (int)rtc.getTemp();

    dates = (String)date + '.' + month + '.' + year;
    times = (String)hour + ':' + minute;
}

void setTextDisplay()
{
    setTimeAndTemp();

    switch (display_scene)
    {
    case 0:
        strcpy(text, times.c_str());
        if (display_effect == 0)
        {
            output.setTextEffect(PA_OPENING, PA_NO_EFFECT);
            display_effect = 1;
        }
        break;
    case 1:
        strcpy(text, rtc.getDOWStr());
        if (display_effect == 1)
        {
            output.setTextEffect(PA_RANDOM, PA_NO_EFFECT);
            display_effect = 2;
        }
        break;
    case 2:
        strcpy(text, dates.c_str());
        if (display_effect == 2)
        {
            output.setTextEffect(PA_SCROLL_UP, PA_NO_EFFECT);
            display_effect = 3;
        }
        break;
    case 3:
        dtostrf(temp, 0, 0, text);
        strcat(text, " $");
        if (display_effect == 3)
        {
            output.setTextEffect(PA_SCROLL_DOWN, PA_NO_EFFECT);
            display_effect = 0;
        }
        break;
    }
}

void display_scene_setting()
{
    if (t.sec >= 0 && t.sec <= 30)
    {
        display_scene = 0;
    }
    else if (t.sec >= 31 && t.sec <= 40)
    {
        display_scene = 1;
    }
    else if (t.sec >= 41 && t.sec <= 50)
    {
        display_scene = 2;
    }
    else if (t.sec >= 51 && t.sec <= 60)
    {
        display_scene = 3;
    }
}

void setup()
{
    Serial.begin(9600);
    Serial.println("Masukkan waktu");
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
    intensity = setIntensityLed(analogRead(LDR));
    output.setIntensity(intensity);
    if (Serial.available() > 0)
    {
        adjustClock(Serial.readString());
    }
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
            setTextDisplay();
            display_scene_setting();
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
            if (timeSync == true)
            {
                timeHour = hour;
                timeMinute = minute;
                timeSecond = second;
                timeDay = dow;
                timeMonth = month;
                timeDate = date;
                timeYear = year;
                timeSync = false;
            }
        }
    }
}
