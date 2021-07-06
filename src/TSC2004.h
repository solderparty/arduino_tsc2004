#pragma once

#include <Wire.h>

#define TSC2004_DEFAULT_ADDR 0x4b

class TS_Point
{
    public:
        TS_Point(int16_t _x = 0, int16_t _y = 0, int16_t _z = 0)
        {
            x = _x;
            y = _y;
            z = _z;
        }

        bool operator==(TS_Point &other)
        {
            return (x == other.x) && (y == other.y) && (z == other.z);
        }

        int16_t x;
        int16_t y;
        int16_t z;
};

class TSC2004
{
    public:
        TSC2004();

        bool begin(uint8_t addr = TSC2004_DEFAULT_ADDR, TwoWire *wire = &Wire);

        void reset();

        void readData(int16_t *x, int16_t *y, int16_t *z);
        bool touched();
        bool bufferEmpty();
        TS_Point getPoint();

        uint16_t readRegister16(uint8_t reg) const;
        void writeRegister16(uint8_t reg, uint16_t value);
        void writeCmd(uint8_t cmd);

    private:
        TwoWire *m_wire;
        uint8_t m_addr;
};
