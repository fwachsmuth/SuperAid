#include <FreqMeasure.h>

#define isrPin 2

double sum = 0;
int count = 0;

void setup()
{
    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(isrPin, INPUT);
    Serial.begin(115200);
    FreqMeasure.begin();
    Serial.println("Hallo.");

    attachInterrupt(digitalPinToInterrupt(isrPin), dutyCycleISR, CHANGE);
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
}

void dutyCycleISR()
{
    
}
