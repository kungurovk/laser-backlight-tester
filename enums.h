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
    Address = 0x00,
};

enum States : unsigned short
{
    On  = 0x01,
    Off = 0x00,
};

enum SensorsTableAddress
{
    BoardOperatingMode      = 0x100,
    LaserOperatingMode      = 0x101,
    CaseTemperature_1       = 0x102,
    CaseTemperature_2       = 0x104,
    CoolantTemperature_1    = 0x106,
    CoolantTemperature_2    = 0x108,
    CoolantFlowRate_1       = 0x10a,
    CoolantFlowRate_2       = 0x10c,
    CoolantFlowRate_3       = 0x10e,
    AirHumidity_1           = 0x110,
    AitTemperature_1        = 0x112,
    AirHumidity_2           = 0x114,
    AitTemperature_2        = 0x116,
    LaserPower              = 0x118,
    CrystalTemperature_1    = 0x11a,
    CrystalTemperature_2    = 0x11c,
    LaserWorkTime           = 0x12c,
};

enum BlockTableAddress
{
    LaserControlBoardStatus          = 0x11e,
    PowerSupplyControlStatus         = 0x120,
    PowerSupplyQuantumtronsStatus_1  = 0x122,
    PowerSupplyQuantumtronsStatus_2  = 0x123,
    PowerSupplyQuantumtronsStatus_3  = 0x124,
    PowerSupplyQuantumtronsStatus_4  = 0x125,
    PowerSupplyQuantumtronsStatus_5  = 0x126,
    PowerSupplyQuantumtronsStatus_6  = 0x127,
    PowerSupplyQuantumtronsStatus_7  = 0x128,
    PowerSupplyQuantumtronsStatus_8  = 0x129,
    PowerSupplyQuantumtronsStatus_9  = 0x12a,
    PowerSupplyQuantumtronsStatus_10 = 0x12b,
    AddressTillOfEndBlocks           = 0x12c
};

enum ValuesTableAddress
{
    CaseTemperatureMinValue_1       = 0x200,
    CaseTemperatureMaxValue_1       = 0x202,
    CaseTemperatureMinValue_2       = 0x204,
    CaseTemperatureMaxValue_2       = 0x206,
    CoolantTemperatureMinValue_1    = 0x208,
    CoolantTemperatureMaxValue_1    = 0x20a,
    CoolantTemperatureMinValue_2    = 0x20c,
    CoolantTemperatureMaxValue_2    = 0x20e,
    FlowRateMinValue_1              = 0x210,
    FlowRateMaxValue_1              = 0x212,
    FlowRateMinValue_2              = 0x214,
    FlowRateMaxValue_2              = 0x216,
    FlowRateMinValue_3              = 0x218,
    FlowRateMaxValue_3              = 0x21a,
    AirHumidityMinValue_1           = 0x21c,
    AirHumidityMaxValue_1           = 0x21e,
    AirTemperatureMinValue_1        = 0x220,
    AirTemperatureMaxValue_1        = 0x222,
    AirHumidityMinValue_2           = 0x224,
    AirHumidityMaxValue_2           = 0x226,
    AirTemperatureMinValue_2        = 0x228,
    AirTemperatureMaxValue_2        = 0x22a,
    PowerLaserMinValue              = 0x22c,
    PowerLaserMaxValue              = 0x22e,
    CrystalTemperatureTarget_1      = 0x230,
    CrystalTemperatureTarget_2      = 0x232,
    KP_PID_LBO                      = 0x234,
    KI_PID_LBO                      = 0x236,
    KD_PID_LBO                      = 0x238,
    AddressTillOfEndValues          = 0x240
};

enum GeneratorSetterAddress
{
    TermoStableOnOff            = 0x500,
    ImpulseOnOff                = 0x501,
    DiodTemperature             = 0x502,
    CrystalTemperature          = 0x504,
    AddressTillOfEndGenerator   = 0x506
};

enum PowerSupplyQuantumtronsStatusBits
{
    PowerSupply             = 0,
    FrequencyModeControl    = 1,
    Synchronization         = 2,
    PowerSupplyReadySignal  = 5,
};

enum LaserControlBoardStatusBits
{
    HeaterStatus = 0,
    TempOfCase_1BelowMinLimit,
    TempOfCase_1AboveMaxLimit,
    TempOfCase_2BelowMinLimit,
    TempOfCase_2AboveMaxLimit,
    TempOfCool_1BelowMinLimit,
    TempOfCool_1AboveMaxLimit,
    TempOfCool_2BelowMinLimit,
    TempOfCool_2AboveMaxLimit,
    CoolFlowRate_1BelowMinLimit,
    CoolFlowRate_1AboveMaxLimit,
    CoolFlowRate_2BelowMinLimit,
    CoolFlowRate_2AboveMaxLimit,
    CoolFlowRate_3BelowMinLimit,
    CoolFlowRate_3AboveMaxLimit,
    AirHumidity_1BelowMinLimit,
    AirHumidity_1AboveMaxLimit,
    AirTemperature_1BelowMinLimit,
    AirTemperature_1AboveMaxLimit,
    AirHumidity_2BelowMinLimit,
    AirHumidity_2AboveMaxLimit,
    AirTemperature_2BelowMinLimit,
    AirTemperature_2AboveMaxLimit,
    PowerLaser_2BelowMinLimit,
    PowerLaser_2AboveMaxLimit,
    LaserWorkMode_1Bit,
    LaserWorkMode_2Bit,
    SignalIsGood,
    SignalIsReady,
    StateOfMasterOscillatorPumpSyncPulse,
    StateOfAmplifierPumpSyncPulse,
    StateOfRadiationGenerationSyncPulse,
};

enum GeneratorSetterStatusBits
{
    ThermalStabilizationEnabled = 0,
    LaserDiodeTemperatureNormal,
    LaserDiodeOverheated,
    DoublerCrystalTemperatureNormal,
    DoublerCrystalOverheated,
    TwelveVPowerSourceEnabled,
    LaserModuleOperational,
    LaserDiodeVoltageRegulatorEnabled,
    LaserDiodeCurrentPulsesEnabled,
    CoolerOverheated,
    PowerTransistorOverheated,
    LaserDiodeOvercurrent,
    PowerTransistorOvervoltage,
    AcoustoOpticShutterDriverFailure,
    ExternalLock,
    SystemConfigurationLoaded,
    DriverConfigurationLoaded,
    TimingConfigurationLoaded,
    DiodeThermalStabilizationConfigurationLoaded,
    CrystalThermalStabilizationConfigurationLoaded,
};

#endif // ENUMS_H
