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
    const DigitalInput &Common;

public:
    DeMux_Driver(DigitalOutput &S0,
                 DigitalOutput &S1,
                 DigitalOutput &S2,
                 DigitalOutput &S3,
                 const DigitalInput &Common) : S0{S0}, S1{S1}, S2{S2}, S3{S3}, Common{Common} {};
    ~DeMux_Driver() {};

    PinState getState(uint8_t pinNumber)
    {
        if (pinNumber >= 16)
        {
            return floating;
        }
        S0.setState(pinNumber & 0b0001 ? High : Low);
        S1.setState(pinNumber & 0b0010 ? High : Low);
        S2.setState(pinNumber & 0b0100 ? High : Low);
        S3.setState(pinNumber & 0b1000 ? High : Low);
        // delayMicroseconds(1);
        return Common.getState();
    }
};

class DeMuxedPin : public DigitalInput
{
private:
    DeMux_Driver &demux;
    const uint8_t pinNumber;
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
    const AnalogInput &Common;

public:
    DeMux_Driver_Analog(DigitalOutput &S0,
                        DigitalOutput &S1,
                        DigitalOutput &S2,
                        DigitalOutput &S3,
                        const AnalogInput &Common) : S0{S0}, S1{S1}, S2{S2}, S3{S3}, Common{Common} {};
    ~DeMux_Driver_Analog() {};

    int getState(uint8_t pinNumber)
    {
        if (pinNumber >= 16)
        {
            return floating;
        }
        S0.setState(pinNumber & 0b0001 ? High : Low);
        S1.setState(pinNumber & 0b0010 ? High : Low);
        S2.setState(pinNumber & 0b0100 ? High : Low);
        S3.setState(pinNumber & 0b1000 ? High : Low);
        // delayNanoseconds(1);
        return Common.getState();
    }
};

class DeMuxedPin_Analog : public AnalogInput
{
private:
    DeMux_Driver_Analog &demux;
    const uint8_t pinNumber;
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
    const uint8_t pinNumber;
    const int threshold;
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
        }
        return cachedState > threshold ? High : Low;
    }
};

template <typename T, unsigned int Size>
class CircularBuffer
{
private:
    T _store[Size] = {0};
    unsigned int _position = 0; // _position variable for circular buffer
    unsigned int _count = 0;

public:
    CircularBuffer() {};
    ~CircularBuffer() {};
    void push(T entry);
    T at(unsigned int position);
    T size() const { return Size; };
};

template <typename T, unsigned int Size>
void CircularBuffer<T, Size>::push(T entry)
{
    if (_count < Size)
    {
        _count++;
    }
    _store[_position] = entry;
    _position = (_position + 1) % Size; // Use modulo to handle wrap-around
}

template <typename T, unsigned int Size>
T CircularBuffer<T, Size>::at(unsigned int position)
{
    return _store[(_position + position) % _count]; // Use modulo to handle wrap-around
}

class DeMuxedPin_AC_Analog
{
private:
    DeMux_Driver_Analog &demux;
    const uint8_t pinNumber;
    unsigned long lastRead = millis();
    CircularBuffer<int, 500> buffer;
    int cachedState = 0;

public:
    DeMuxedPin_AC_Analog(DeMux_Driver_Analog &demux, uint8_t pinNumber) : demux{demux}, pinNumber{pinNumber}, buffer() {};
    ~DeMuxedPin_AC_Analog() {};
    int getState()
    {
        // if (millis() - lastRead > 2)
        // {
        const auto newReading = demux.getState(pinNumber);
        buffer.push(newReading);
        auto highestReading = 0;
        for (int i = 0; i < buffer.size(); i++)
        {
            if (buffer.at(i) > highestReading)
            {
                highestReading = buffer.at(i);
            }
        }
        cachedState = highestReading;
        //     lastRead = millis();
        // }
        return cachedState;
    }
};
#endif