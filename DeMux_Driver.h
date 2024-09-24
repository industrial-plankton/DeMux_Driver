#ifndef _Demux_Driver_h_included
#define _Demux_Driver_h_included

#include <DigitalInput.h>
#include <AnalogInput.h>
#include <DigitalOutput.h>
#include <stdint.h>

// 74HC4067BQ,118 https://assets.nexperia.com/documents/data-sheet/74HC_HCT4067.pdf
class DeMux_Driver
{
private:
    DigitalOutput &S0;
    DigitalOutput &S1;
    DigitalOutput &S2;
    DigitalOutput &S3;
    DigitalInput &Common;

public:
    DeMux_Driver(DigitalOutput &S0,
                 DigitalOutput &S1,
                 DigitalOutput &S2,
                 DigitalOutput &S3,
                 DigitalInput &Common) : S0{S0}, S1{S1}, S2{S2}, S3{S3}, Common{Common} {};
    ~DeMux_Driver() {};

    PinState getState(uint8_t pinNumber)
    {
        if (pinNumber >= 16)
        {
            return floating;
        }
        S0.setState(static_cast<PinState>(static_cast<bool>(pinNumber & 0b0001))); // Cast to bool prevent setting to floating
        S1.setState(static_cast<PinState>(static_cast<bool>(pinNumber & 0b0010)));
        S2.setState(static_cast<PinState>(static_cast<bool>(pinNumber & 0b0100)));
        S3.setState(static_cast<PinState>(static_cast<bool>(pinNumber & 0b1000)));
        // May need a delay here
        delayMicroseconds(1);
        return Common.getState();
    }
};

class DeMuxedPin : public DigitalInput
{
private:
    DeMux_Driver &demux;
    uint8_t pinNumber;
    unsigned long lastRead = millis();
    PinState cachedState = Low;

public:
    DeMuxedPin(DeMux_Driver &demux, uint8_t pinNumber) : demux{demux}, pinNumber{pinNumber} {};
    ~DeMuxedPin() {};
    PinState getState() const
    {
        if (millis() - lastRead > 40)
        {
            const_cast<DeMuxedPin *>(this)->cachedState = demux.getState(pinNumber);
            const_cast<DeMuxedPin *>(this)->lastRead = millis();
        }
        return cachedState;
    }
};

////////////////////////////////////////
// Analog
////////////////////////////////////////

class DeMux_Driver_Analog
{
private:
    DigitalOutput &S0;
    DigitalOutput &S1;
    DigitalOutput &S2;
    DigitalOutput &S3;
    AnalogInput &Common;

public:
    DeMux_Driver_Analog(DigitalOutput &S0,
                        DigitalOutput &S1,
                        DigitalOutput &S2,
                        DigitalOutput &S3,
                        AnalogInput &Common) : S0{S0}, S1{S1}, S2{S2}, S3{S3}, Common{Common} {};
    ~DeMux_Driver_Analog() {};

    int getState(uint8_t pinNumber)
    {
        if (pinNumber >= 16)
        {
            return floating;
        }
        S0.setState(static_cast<PinState>(static_cast<bool>(pinNumber & 0b0001))); // Cast to bool prevent setting to floating
        S1.setState(static_cast<PinState>(static_cast<bool>(pinNumber & 0b0010)));
        S2.setState(static_cast<PinState>(static_cast<bool>(pinNumber & 0b0100)));
        S3.setState(static_cast<PinState>(static_cast<bool>(pinNumber & 0b1000)));
        // May need a delay here
        delayMicroseconds(1);
        return Common.getState();
    }
};

class DeMuxedPin_Analog : public AnalogInput
{
private:
    DeMux_Driver_Analog &demux;
    uint8_t pinNumber;
    unsigned long lastRead = millis();
    int cachedState = 0;

public:
    DeMuxedPin_Analog(DeMux_Driver_Analog &demux, uint8_t pinNumber) : demux{demux}, pinNumber{pinNumber} {};
    ~DeMuxedPin_Analog() {};
    int getState() const
    {
        if (millis() - lastRead > 40)
        {
            const_cast<DeMuxedPin_Analog *>(this)->cachedState = demux.getState(pinNumber);
            const_cast<DeMuxedPin_Analog *>(this)->lastRead = millis();
        }
        return cachedState;
    }
};

class DeMuxedPin_DigFromAnalog : public DigitalInput
{
private:
    DeMux_Driver_Analog &demux;
    uint8_t pinNumber;
    int threshold = 1;
    unsigned long lastRead = millis();
    int cachedState = 0;

public:
    DeMuxedPin_DigFromAnalog(DeMux_Driver_Analog &demux, uint8_t pinNumber, int threshold) : demux{demux}, pinNumber{pinNumber}, threshold{threshold} {};
    ~DeMuxedPin_DigFromAnalog() {};
    PinState getState() const
    {
        if (millis() - lastRead > 40)
        {
            const_cast<DeMuxedPin_DigFromAnalog *>(this)->cachedState = demux.getState(pinNumber);
            const_cast<DeMuxedPin_DigFromAnalog *>(this)->lastRead = millis();
            return static_cast<PinState>(cachedState > threshold); // static_cast<PinState>(demux.getState(pinNumber) > threshold);
        }
    }
};
#endif