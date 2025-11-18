#ifndef ENUMS_H
#define ENUMS_H

enum Mode : unsigned short
{
    Auto    = 0x00,
    Manual  = 0x01,
    Duty    = 0x02,
    Prepare = 0x03,
    Work    = 0x04,
};

enum ModeAddress
{
    Address = 0x00
};

enum BlockTableAddress
{
    BoardOperatingMode      = 0x100,
    LaserOperatingMode      = 0x101,
    CaseTemperatureOne      = 0x102,
    CaseTemperatureTwo      = 0x104,
    CoolantTemperatureOne   = 0x106,
    CoolantTemperatureTwo   = 0x108,
    CoolantFlowRateOne      = 0x10A,
    CoolantFlowRateTwo      = 0x10C,
    CoolantFlowRateThree    = 0x10E,
    AirHumidityOne          = 0x110,
    AitTemperatureOne       = 0x112,
    AirHumidityTwo          = 0x114,
    AitTemperatureTwo       = 0x116,
    LaserPower              = 0x118,
};

#endif // ENUMS_H
