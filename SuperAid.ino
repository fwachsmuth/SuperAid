#include <FreqMeasure.h>

#define isr_pin 2

double sum = 0;
int count = 0;
volatile bool shutter_just_changed = false;
bool new_shutter_cycle = true;

int32_t shutter_cycle_millis_start = 0;
int32_t shutter_cycle_millis_closed = 0;
int32_t shutter_cycle_millis_end = 0;
int32_t millisNow = 0;
int32_t millis_now;

void duty_cycle_ISR()
{
    shutter_just_changed = true;
}


void setup()
{
    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(isr_pin, INPUT);
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
        sum = sum + FreqMeasure.read();
        count = count + 1;
        if (count > 30)
        {
            float frequency = FreqMeasure.countToFrequency(sum / count);
            Serial.print(frequency, 3);
            Serial.println(" fps");
            sum = 0;
            count = 0;
        }
    }

    if (shutter_just_changed) 
    {
        if (digitalRead(isr_pin) == LOW)
        {   
            /* shutter is now open */
            if (new_shutter_cycle) 
            {
                millis_now = millis();
                shutter_cycle_millis_start = millis();
                new_shutter_cycle = false;
            }
            else
            {
                shutter_cycle_millis_end = millis();
                new_shutter_cycle = true;
            }
            shutter_just_changed = false;
        }
        else if (digitalRead(isr_pin) == HIGH)
        { 
            /* shutter is now closed */
            shutter_cycle_millis_closed = millis();
            shutter_just_changed = false;

            Serial.println(shutter_cycle_millis_start - millis_now);
            Serial.println(shutter_cycle_millis_closed - millis_now);
            Serial.println(shutter_cycle_millis_end - millis_now);
            Serial.println("----------------");
        }
    }
}

