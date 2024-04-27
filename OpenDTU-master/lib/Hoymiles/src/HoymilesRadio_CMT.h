// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once

#include "HoymilesRadio.h"
#include "commands/CommandAbstract.h"
#include "types.h"
#include <Arduino.h>
#include <memory>
#include <queue>
#include <vector>

// Contents of cmt2300wrapper.h
#define CMT2300A_ONE_STEP_SIZE 2500 // frequency channel step size for fast frequency hopping operation: One step size is 2.5 kHz.
#define FH_OFFSET 100 // value * CMT2300A_ONE_STEP_SIZE = channel frequency offset
#define CMT_SPI_SPEED 4000000 // 4 MHz

#define CMT_BASE_FREQ_900 900000000
#define CMT_BASE_FREQ_860 860000000

enum FrequencyBand_t {
    BAND_860,
    BAND_900,
    FrequencyBand_Max,
};

class CMT2300A {
public:
    CMT2300A(const uint8_t pin_sdio, const uint8_t pin_clk, const uint8_t pin_cs, const uint8_t pin_fcs, const uint32_t _spi_speed = CMT_SPI_SPEED) {}

    bool begin(void) { return true; }

    bool isChipConnected() { return true; }

    bool startListening() { return true; }

    bool stopListening() { return true; }

    bool available() { return true; }

    void read(void* buf, const uint8_t len) {}

    bool write(const uint8_t* buf, const uint8_t len) { return true; }

    void setChannel(const uint8_t channel) {}

    uint8_t getChannel() { return 0; }

    uint8_t getDynamicPayloadSize() { return 0; }

    int getRssiDBm() { return 0; }

    bool setPALevel(const int8_t level) { return true; }

    bool rxFifoAvailable() { return true; }

    uint32_t getBaseFrequency() const { return 0; }
    static constexpr uint32_t getBaseFrequency(FrequencyBand_t band) { return 0; }

    FrequencyBand_t getFrequencyBand() const { return FrequencyBand_t::BAND_860; }
    void setFrequencyBand(const FrequencyBand_t mode) {}

    void flush_rx() {}

private:
    bool _init_pins() { return true; }

    bool _init_radio() { return true; }

    int8_t _pin_sdio;
    int8_t _pin_clk;
    int8_t _pin_cs;
    int8_t _pin_fcs;
    uint32_t _spi_speed;
};

// End of cmt2300wrapper.h

// number of fragments hold in buffer
#define FRAGMENT_BUFFER_SIZE 30

#ifndef HOYMILES_CMT_WORK_FREQ
#define HOYMILES_CMT_WORK_FREQ 865000000
#endif

enum CountryModeId_t {
    MODE_EU,
    MODE_US,
    MODE_BR,
    CountryModeId_Max
};

struct CountryFrequencyDefinition_t {
    FrequencyBand_t Band;
    uint32_t Freq_Min;
    uint32_t Freq_Max;
    uint32_t Freq_Legal_Min;
    uint32_t Freq_Legal_Max;
    uint32_t Freq_Default;
    uint32_t Freq_StartUp;
};

struct CountryFrequencyList_t {
    CountryModeId_t mode;
    CountryFrequencyDefinition_t definition;
};

class HoymilesRadio_CMT : public HoymilesRadio {
public:
    void init(const int8_t pin_sdio, const int8_t pin_clk, const int8_t pin_cs, const int8_t pin_fcs, const int8_t pin_gpio2, const int8_t pin_gpio3);
    void loop();
    void setPALevel(const int8_t paLevel);
    void setInverterTargetFrequency(const uint32_t frequency);
    uint32_t getInverterTargetFrequency() const;

    bool isConnected() const;

    uint32_t getMinFrequency() const;
    uint32_t getMaxFrequency() const;
    static constexpr uint32_t getChannelWidth()
    {
        return FH_OFFSET * CMT2300A_ONE_STEP_SIZE;
    }

    CountryModeId_t getCountryMode() const;
    void setCountryMode(const CountryModeId_t mode);

    uint32_t getInvBootFrequency() const;

    uint32_t getFrequencyFromChannel(const uint8_t channel) const;
    uint8_t getChannelFromFrequency(const uint32_t frequency) const;

    std::vector<CountryFrequencyList_t> getCountryFrequencyList() const;

private:
    void ARDUINO_ISR_ATTR handleInt1();
    void ARDUINO_ISR_ATTR handleInt2();

    void sendEsbPacket(CommandAbstract& cmd);

    std::unique_ptr<CMT2300A> _radio;

    volatile bool _packetReceived = false;
    volatile bool _packetSent = false;

    bool _gpio2_configured = false;
    bool _gpio3_configured = false;

    std::queue<fragment_t> _rxBuffer;
    TimeoutHelper _txTimeout;

    uint32_t _inverterTargetFrequency = HOYMILES_CMT_WORK_FREQ;

    bool cmtSwitchDtuFreq(const uint32_t to_frequency);

    CountryModeId_t _countryMode;
};


