#include <FreqMeasure.h>

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
