#include <Arduino.h>

#include "TSC2004.h"

#define _MAX_12BIT      0x0fff
#define _RESISTOR_VAL   280

// Control Byte 0
#define _REG_READ       (0x01)
#define _REG_PND0       (0x02)
#define _REG_X          (0x0 << 3)
#define _REG_Y          (0x1 << 3)
#define _REG_Z1         (0x2 << 3)
#define _REG_Z2         (0x3 << 3)
#define _REG_AUX        (0x4 << 3)
#define _REG_TEMP1      (0x5 << 3)
#define _REG_TEMP2      (0x6 << 3)
#define _REG_STATUS     (0x7 << 3)
#define _REG_AUX_HIGH   (0x8 << 3)
#define _REG_AUX_LOW    (0x9 << 3)
#define _REG_TEMP_HIGH  (0xA << 3)
#define _REG_TEMP_LOW   (0xB << 3)
#define _REG_CFR0       (0xC << 3)
#define _REG_CFR1       (0xD << 3)
#define _REG_CFR2       (0xE << 3)
#define _REG_CONV_FUNC  (0xF << 3)

// Control Byte 1
#define _CMD            (1 << 7)
#define _CMD_NORMAL     (0x00)
#define _CMD_STOP       (1 << 0)
#define _CMD_RESET      (1 << 1)
#define _CMD_12BIT      (1 << 2)

// Config Register 0
#define CFR0_PRECHARGE_20US     (0x00 << 5)
#define CFR0_PRECHARGE_84US     (0x01 << 5)
#define CFR0_PRECHARGE_276US    (0x02 << 5)
#define CFR0_PRECHARGE_340US    (0x03 << 5)
#define CFR0_PRECHARGE_1_044MS  (0x04 << 5)
#define CFR0_PRECHARGE_1_108MS  (0x05 << 5)
#define CFR0_PRECHARGE_1_300MS  (0x06 << 5)
#define CFR0_PRECHARGE_1_364MS  (0x07 << 5)

#define CFR0_STABTIME_0US       (0x00 << 8)
#define CFR0_STABTIME_100US     (0x01 << 8)
#define CFR0_STABTIME_500US     (0x02 << 8)
#define CFR0_STABTIME_1MS       (0x03 << 8)
#define CFR0_STABTIME_5MS       (0x04 << 8)
#define CFR0_STABTIME_10MS      (0x05 << 8)
#define CFR0_STABTIME_50MS      (0x06 << 8)
#define CFR0_STABTIME_100MS     (0x07 << 8)

#define CFR0_CLOCK_4MHZ         (0x00 << 11)
#define CFR0_CLOCK_2MHZ         (0x01 << 11)
#define CFR0_CLOCK_1MHZ         (0x02 << 11)

#define CFR0_12BIT              (1 << 13)
#define CFR0_STATUS             (1 << 14)
#define CFR0_PENMODE            (1 << 15)

// Config Register 1
#define CFR1_BATCHDELAY_0MS     (0x00 << 0)
#define CFR1_BATCHDELAY_1MS     (0x01 << 0)
#define CFR1_BATCHDELAY_2MS     (0x02 << 0)
#define CFR1_BATCHDELAY_4MS     (0x03 << 0)
#define CFR1_BATCHDELAY_10MS    (0x04 << 0)
#define CFR1_BATCHDELAY_20MS    (0x05 << 0)
#define CFR1_BATCHDELAY_40MS    (0x06 << 0)
#define CFR1_BATCHDELAY_100MS   (0x07 << 0)

// Config Register 2
#define CFR2_MAVE_Z             (1 << 2)
#define CFR2_MAVE_Y             (1 << 3)
#define CFR2_MAVE_X             (1 << 4)
#define CFR2_AVG_7              (0x01 << 11)
#define CFR2_MEDIUM_15          (0x03 << 12)

#define STATUS_DAV_X            0x8000
#define STATUS_DAV_Y            0x4000
#define STATUS_DAV_Z1           0x2000
#define STATUS_DAV_Z2           0x1000
#define STATUS_DAV_MASK         (STATUS_DAV_X | STATUS_DAV_Y | STATUS_DAV_Z1 | STATUS_DAV_Z2)

TSC2004::TSC2004()
    : m_wire(nullptr)
{

}

bool TSC2004::begin(uint8_t addr, TwoWire *wire)
{
    m_addr = addr;
    m_wire = wire;

    m_wire->begin();

    reset();
    delay(10);

    const uint16_t cfr0 = (CFR0_STABTIME_1MS | CFR0_CLOCK_1MHZ | CFR0_12BIT | CFR0_PRECHARGE_276US | CFR0_PENMODE);
    writeRegister16(_REG_CFR0, cfr0);

    writeRegister16(_REG_CFR1, CFR1_BATCHDELAY_4MS);

    const uint16_t cfr2 = (CFR2_MAVE_Z | CFR2_MAVE_Y | CFR2_MAVE_X | CFR2_AVG_7 | CFR2_MEDIUM_15);
    writeRegister16(_REG_CFR2, cfr2);

    writeCmd(_CMD_NORMAL);

    return true;
}

void TSC2004::reset()
{
    writeCmd(_CMD_RESET);
}

void TSC2004::readData(int16_t *x, int16_t *y, int16_t *z)
{
    while ((readRegister16(_REG_STATUS) & STATUS_DAV_MASK) == 0) {
        continue;
    }

    *x = readRegister16(_REG_X);
    *y = readRegister16(_REG_Y);

    const uint16_t z1 = readRegister16(_REG_Z1);
    const uint16_t z2 = readRegister16(_REG_Z2);

    if ((*x > _MAX_12BIT) or (*y > _MAX_12BIT) or (z1 == 0) or (z2 > _MAX_12BIT) or (z1 >= z2)) {
        *x = 0;
        *y = 0;
        *z = 0;
        return;
    }

    *z = *x * (z2 - z1) / z1;
    *z *= _RESISTOR_VAL / 4096;
}

bool TSC2004::touched()
{
    return readRegister16(_REG_CFR0) & CFR0_PENMODE;
}

bool TSC2004::bufferEmpty()
{
    return !touched();
}

TS_Point TSC2004::getPoint()
{
    int16_t x, y, z;

    readData(&x, &y, &z);

    return TS_Point(x, y, z);
}

uint16_t TSC2004::readRegister16(uint8_t reg) const
{
    m_wire->beginTransmission(m_addr);
    m_wire->write(reg | _REG_READ);
    m_wire->endTransmission();

    m_wire->requestFrom(m_addr, 2);
    if (m_wire->available() < 2) {
        return 0;
    }

    const uint8_t high = m_wire->read();
    const uint8_t low = m_wire->read();

    return (high << 8) | low;
}

void TSC2004::writeRegister16(uint8_t reg, uint16_t value)
{
    uint8_t data[3];
    data[0] = reg | _REG_PND0;
    data[1] = (value >> 8) & 0xFF;
    data[2] = (value >> 0) & 0xFF;

    m_wire->beginTransmission(m_addr);
    m_wire->write(data, sizeof(uint8_t) * 3);
    m_wire->endTransmission();
}

void TSC2004::writeCmd(uint8_t cmd)
{
    m_wire->beginTransmission(m_addr);
    m_wire->write(_CMD | _CMD_12BIT | cmd);
    m_wire->endTransmission();
}
