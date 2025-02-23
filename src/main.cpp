#include <Arduino.h>
#include <Adafruit_SCD30.h>

constexpr uint16_t ALTITUDE_OFFSET = 315; // meters above sea level
constexpr uint16_t TEMPERATURE_OFFSET = 400; // 4.00 degree Celsius

Adafruit_SCD30 scd30;

// forward refs
void print_configuration();
bool calibrate(uint16_t, uint16_t);


void setup()
{
    Serial.begin(115200);
    while (!Serial) delay(100); // will pause until serial console opens

    // Try to initialize sensor
    if (!scd30.begin())
    {
        Serial.println(F("Failed to find SCD30 chip"));
        while (true) { delay(1000); }
    }

#if RUN_SENSOR_CALIBRATION == 1
    if (!calibrate(ALTITUDE_OFFSET, TEMPERATURE_OFFSET))
    {
        while (true) { delay(1000); }
    }
#endif

    print_configuration();
}


void loop()
{
    if (scd30.dataReady())
    {
        if (!scd30.read())
        {
            Serial.println(F("Error reading sensor data"));
            return;
        }

        Serial.print(F("Temperature: "));
        Serial.print(scd30.temperature);
        Serial.println(F(" degrees C"));

        Serial.print(F("Relative Humidity: "));
        Serial.print(scd30.relative_humidity);
        Serial.println(F(" %"));

        Serial.print(F("CO2: "));
        Serial.print(scd30.CO2, 3);
        Serial.println(F(" ppm"));
        Serial.println();
    }

    delay(2000);
}

void print_configuration()
{
    Serial.println(F("Sensor Configuration"));
    Serial.println(F("===================="));

    Serial.print(F("Measurement interval: "));
    Serial.print(scd30.getMeasurementInterval());
    Serial.println(F(" seconds"));

    Serial.print(F("Ambient pressure offset: "));
    Serial.print(scd30.getAmbientPressureOffset());
    Serial.println(F(" mBar"));

    Serial.print(F("Altitude offset: "));
    Serial.print(scd30.getAltitudeOffset());
    Serial.println(F(" meters"));

    Serial.print(F("Temperature offset: "));
    Serial.print(static_cast<float>(scd30.getTemperatureOffset()) / 100.0);
    Serial.println(F(" degrees C"));

    Serial.print(F("Forced Recalibration reference: "));
    Serial.print(scd30.getForcedCalibrationReference());
    Serial.println(F(" ppm"));

    if (scd30.selfCalibrationEnabled())
    {
        Serial.println(F("Self calibration enabled"));
    }
    else
    {
        Serial.println(F("Self calibration disabled"));
    }
    Serial.println();
}


bool calibrate(const uint16_t altitude_offset, const uint16_t temperature_offset)
{
    Serial.println(F("Calibration started"));

    /* Set an altitude offset in meters above sea level.
     * Offset value stored in non-volatile memory of SCD30.
     * Setting an altitude offset will override any pressure offset.
     */
    if (!scd30.setAltitudeOffset(altitude_offset))
    {
        Serial.println(F("Failed to set altitude offset"));
        return false;
    }

    /* Set a temperature offset in hundredths of a degree celcius.
     * Offset value stored in non-volatile memory of SCD30.
     */
    if (!scd30.setTemperatureOffset(temperature_offset))
    {
        Serial.println(F("Failed to set temperature offset"));
        return false;
    }

    /* Enable or disable automatic self calibration (ASC).
     * Parameter stored in non-volatile memory of SCD30.
     * Enabling self calibration will override any previously set
     * forced calibration value.
     * ASC needs continuous operation with at least 1 hour
     * 400ppm CO2 concentration daily.
     */
    if (!scd30.selfCalibrationEnabled(false))
    {
        Serial.println(F("Failed to enable or disable self calibration"));
        return false;
    }

    /* Force the sensor to recalibrate with the given reference value
     * from 400-2000 ppm. Writing a recalibration reference will overwrite
     * any previous self calibration values.
     * Reference value stored in non-volatile memory of SCD30.
     */
    if (!scd30.forceRecalibrationWithReference(400))
    {
        Serial.println(F("Failed to force recalibration with reference"));
        return false;
    }

    Serial.println(F("Calibration successful"));
    return true;
}
