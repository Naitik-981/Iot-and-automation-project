#ifndef DHT_MODULE_H
#define DHT_MODULE_H

#include <DHT.h>

class DHTSensor {
private:
    DHT dht;
    uint8_t pin;

public:
    DHTSensor(uint8_t pin, uint8_t type) : dht(pin, type), pin(pin) {}

    void begin() {
        dht.begin();
    }

    float getHumidity() {
        return dht.readHumidity();
    }

    float getTemperature() {
        return dht.readTemperature();
    }

    bool isDataValid(float humidity, float temperature) {
        return !isnan(humidity) && !isnan(temperature);
    }
};

#endif
