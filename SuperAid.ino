#include <FreqMeasure.h>
#include <Arduino.h>    // Why?
#include <U8g2lib.h>
#include <Wire.h>

U8G2_SSD1306_128X64_NONAME_1_HW_I2C u8g2(U8G2_R0, /* reset=*/U8X8_PIN_NONE);


#define isr_pin 2
#define light_meter A0


double sum = 0;
int count = 0;
volatile bool shutter_just_changed = false;

unsigned long shutter_cycle_micros_start = 0;
unsigned long shutter_cycle_micros_closed = 0;

uint32_t light_level_sum = 0;
uint8_t light_level_count = 0;

void duty_cycle_ISR()
{
    shutter_just_changed = true;
}

void setup()
{
    // u8g2.begin();
    
    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(isr_pin, INPUT);
    pinMode(light_meter, INPUT);
    Serial.begin(115200);
    FreqMeasure.begin();
    Serial.println("Hallo.");

    attachInterrupt(digitalPinToInterrupt(isr_pin), duty_cycle_ISR, CHANGE);
}

void loop()
{
    if (FreqMeasure.available())
    {
        // average several reading together
        sum += FreqMeasure.read();
        count++;
        if (count > 30)
        {
            float frequency = FreqMeasure.countToFrequency(sum / count);
            Serial.print(frequency, 3);
            Serial.println(" fps");
            sum = 0;
            count = 0;
        }
    }

    unsigned long micros_now = micros();
    bool shutter_open = digitalRead(isr_pin) == LOW;
    int light_level = analogRead(light_meter);
    if (shutter_just_changed)
    {
        shutter_just_changed = false;
        if (shutter_open)
        {
            // shutter is now open
            unsigned long low_micros = shutter_cycle_micros_closed - shutter_cycle_micros_start;
            unsigned long cycle_micros = micros_now - shutter_cycle_micros_start;

            Serial.print("shutter speed: 1 / ");
            Serial.println(1000000 / low_micros);
            Serial.print("shutter angle: ");
            Serial.print(360 * low_micros / cycle_micros);
            Serial.println("°");
            Serial.println("----------------");

            // do
            // {
            //     u8g2.setFont(u8g2_font_ncenB08_tr);
            //     u8g2.setCursor(0, 15);
            //     u8g2.println("Shutter Speed:");
            //     u8g2.setCursor(0, 30);
            //     u8g2.print("1/");
            //     u8g2.print(1000000 / low_micros);
            //     u8g2.println("  ");
            // } while (u8g2.nextPage());

            shutter_cycle_micros_start = micros_now;
        }
        else
        {
            // shutter is now closed
            shutter_cycle_micros_closed = micros_now;
        }
    }

    if (shutter_open)
    {
        light_level_sum += light_level;
        light_level_count++;

        if (light_level_count == 0)
        {
            // light_level_count (8-bit) overflowed
            Serial.print("light level: ");
            Serial.println(light_level_sum >> 8);
            light_level_sum = 0;
        }
    }
}
