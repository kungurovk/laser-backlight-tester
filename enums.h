#ifndef ENUMS_H
#define ENUMS_H

enum Mode : unsigned short
{
    Auto    = 0x00,
    Manual  = 0x01,
    Duty    = 0x02,
    Prepare = 0x03,
    Work    = 0x04
};

enum ModeAddress
{
    Address = 0x00
};

#endif // ENUMS_H
