#include <common/debug.h>
#include <common/i2c.h>

int smbus_read(uint8_t address, uint8_t command, uint16_t * data) {
    return i2c_get(address, command, (uint8_t *)data, 2);
}

int smbus_write(uint8_t address, uint8_t command, uint16_t data) {
    return i2c_set(address, command, (uint8_t *)&data, 2);
}

int battery_charger_disable(void) {
    int res = 0;

    // Disable charge current
    res = smbus_write(0x09, 0x14, 0);
    if (res < 0) return res;

    // Disable charge voltage
    res = smbus_write(0x09, 0x15, 0);
    if (res < 0) return res;

    return 0;
}

int battery_charger_enable(void) {
    int res = 0;

    res = battery_charger_disable();
    if (res < 0) return res;

    // Set charge current to ~1.5 A
    res = smbus_write(0x09, 0x14, 0x0600);
    if (res < 0) return res;

    // Set charge voltage to ~13 V
    res = smbus_write(0x09, 0x15, 0x3300);
    if (res < 0) return res;

    return 0;
}

uint16_t battery_temp = 0;
uint16_t battery_voltage = 0;
uint16_t battery_current = 0;
uint16_t battery_charge = 0;

void battery_event(void) {
    int res = 0;

    #define command(N, V) { \
        res = smbus_read(0x0B, V, &N); \
        if (res < 0) { \
            N = 0; \
            return; \
        } \
    }

    command(battery_temp, 0x08);
    command(battery_voltage, 0x09);
    command(battery_current, 0x0A);
    command(battery_charge, 0x0D);

    #undef command
}

void battery_debug(void) {
    uint16_t data = 0;
    int res = 0;

    #define command(N, A, V) { \
        DEBUG(#N ": "); \
        res = smbus_read(A, V, &data); \
        if (res < 0) { \
            DEBUG("ERROR %04X\n", -res); \
        } else { \
            DEBUG("%04X\n", data); \
        } \
    }

    DEBUG("Battery:\n");
    command(Temperature, 0x0B, 0x08);
    command(Voltage, 0x0B, 0x09);
    command(Current, 0x0B, 0x0A);
    command(Charge, 0x0B, 0x0D);

    DEBUG("Charger:\n");
    command(ChargeOption0, 0x09, 0x12);
    command(ChargeOption1, 0x09, 0x3B);
    command(ChargeOption2, 0x09, 0x38);
    command(ChargeOption3, 0x09, 0x37);
    command(ChargeCurrent, 0x09, 0x14);
    command(ChargeVoltage, 0x09, 0x15);
    command(DishargeCurrent, 0x09, 0x39);
    command(InputCurrent, 0x09, 0x3F);
    command(ProchotOption0, 0x09, 0x3C);
    command(ProchotOption1, 0x09, 0x3D);
    command(ProchotStatus, 0x09, 0x3A);

    #undef command
}
