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

enum SensorsTableAddress
{
    BoardOperatingMode      = 0x100,
    LaserOperatingMode      = 0x101,
    CaseTemperature_1       = 0x102,
    CaseTemperature_2       = 0x104,
    CoolantTemperature_1    = 0x106,
    CoolantTemperature_2    = 0x108,
    CoolantFlowRate_1       = 0x10A,
    CoolantFlowRate_2       = 0x10C,
    CoolantFlowRate_3       = 0x10E,
    AirHumidity_1           = 0x110,
    AitTemperature_1        = 0x112,
    AirHumidity_2           = 0x114,
    AitTemperature_2        = 0x116,
    LaserPower              = 0x118,
};

enum BlockTableAddress
{
    LaserControlBoardStatus         = 0x11e,
    PowerSupplyControlStatus        = 0x120,
    PowerSupplyQuantumtronsStatus1  = 0x122,
    PowerSupplyQuantumtronsStatus2  = 0x123,
    PowerSupplyQuantumtronsStatus3  = 0x124,
    PowerSupplyQuantumtronsStatus4  = 0x125,
    PowerSupplyQuantumtronsStatus5  = 0x126,
    PowerSupplyQuantumtronsStatus6  = 0x127,
    PowerSupplyQuantumtronsStatus7  = 0x128,
    PowerSupplyQuantumtronsStatus8  = 0x129,
    PowerSupplyQuantumtronsStatus9  = 0x12a,
    PowerSupplyQuantumtronsStatus10 = 0x12b,
};

#endif // ENUMS_H
