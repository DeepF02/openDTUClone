// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2023 - 2024 Thomas Basler and others
 */
#include "GridProfileParser.h"
#include "../Hoymiles.h"
#include <cstring>
#include <array>
#include <vector>
#include <utility>
#include <stdexcept>
#include <iostream>  // For standard output

// Defining a basic string type using templates
template <typename CharT>
class basic_string {
    const CharT* data_;
    std::size_t size_;

public:
    constexpr basic_string(const CharT* data, std::size_t size) : data_(data), size_(size) {}
    constexpr basic_string(const CharT* data) : data_(data), size_(std::strlen(data)) {}
    constexpr const CharT* data() const { return data_; }
    constexpr std::size_t size() const { return size_; }
    constexpr const CharT& operator[](std::size_t i) const { return data_[i]; }
    friend bool operator==(const basic_string& a, const basic_string& b) {
        return a.size_ == b.size_ && std::memcmp(a.data_, b.data_, a.size_ * sizeof(CharT)) == 0;
    }
    friend bool operator<(const basic_string& a, const basic_string& b) {
        auto min_len = a.size_ < b.size_ ? a.size_ : b.size_;
        int cmp = std::memcmp(a.data_, b.data_, min_len);
        return cmp == 0 ? a.size_ < b.size_ : cmp < 0;
    }
};

using string = basic_string<char>;

// Defining a simple map type using templates
template<typename Key, typename Value, std::size_t N>
class map {
    std::array<std::pair<Key, Value>, N> items_;

public:
    constexpr map(std::initializer_list<std::pair<Key, Value>> items) : items_() {
        std::copy(items.begin(), items.end(), items_.begin());
    }

    constexpr Value at(const Key& key) const {
        for (const auto& item : items_) {
            if (item.first == key) return item.second;
        }
        throw std::out_of_range("Key not found");
    }
};

// Profiles and their types definitions
struct ProfileType_t {
    uint8_t lIdx;
    uint8_t hIdx;
    string Name;
};

constexpr std::array<const ProfileType_t, 10> profileTypes = { {
    { 0x02, 0x00, "US - NA_IEEE1547_240V" },
    { 0x03, 0x00, "DE - DE_VDE4105_2018" },
    { 0x03, 0x01, "XX - unknown" },
    { 0x0a, 0x00, "XX - EN 50549-1:2019" },
    { 0x0c, 0x00, "AT - AT_TOR_Erzeuger_default" },
    { 0x0d, 0x04, "FR -" },
    { 0x10, 0x00, "ES - ES_RD1699" },
    { 0x12, 0x00, "PL - EU_EN50438" },
    { 0x29, 0x00, "NL - NL_NEN-EN50549-1_2019" },
    { 0x37, 0x00, "CH - CH_NA EEA-NE7-CH2020" }
} };

// Defining a map for profile sections
constexpr map<uint8_t, string, 12> profileSection = {
    { 0x00, "Voltage (H/LVRT)" },
    { 0x10, "Frequency (H/LFRT)" },
    { 0x20, "Island Detection (ID)" },
    { 0x30, "Reconnection (RT)" },
    { 0x40, "Ramp Rates (RR)" },
    { 0x50, "Frequency Watt (FW)" },
    { 0x60, "Volt Watt (VW)" },
    { 0x70, "Active Power Control (APC)" },
    { 0x80, "Volt Var (VV)" },
    { 0x90, "Specified Power Factor (SPF)" },
    { 0xA0, "Reactive Power Control (RPC)" },
    { 0xB0, "Watt Power Factor (WPF)" }
};

struct GridProfileItemDefinition_t {
    string Name;
    string Unit;
    uint8_t Divider;
};

constexpr GridProfileItemDefinition_t make_value(string Name, string Unit, uint8_t divisor) {
    return { Name, Unit, divisor };
}

constexpr map<uint8_t, GridProfileItemDefinition_t, 0x42> itemDefinitions = {
    { 0x01, make_value("Nominale Voltage (NV)", "V", 10) },
    { 0x02, make_value("Low Voltage 1 (LV1)", "V", 10) },
    { 0x03, make_value("LV1 Maximum Trip Time (MTT)", "s", 10) },
    { 0x04, make_value("High Voltage 1 (HV1)", "V", 10) },
    { 0x05, make_value("HV1 Maximum Trip Time (MTT)", "s", 10) },
    { 0x06, make_value("Low Voltage 2 (LV2)", "V", 10) },
    { 0x07, make_value("LV2 Maximum Trip Time (MTT)", "s", 100) },
    { 0x08, make_value("High Voltage 2 (HV2)", "V", 10) },
    { 0x09, make_value("HV2 Maximum Trip Time (MTT)", "s", 100) },
    { 0x0A, make_value("10mins Average High Voltage (AHV)", "V", 10) },
    { 0x0B, make_value("High Voltage 3 (HV3)", "V", 10) },
    { 0x0C, make_value("HV3 Maximum Trip Time (MTT)", "s", 100) },
    { 0x0D, make_value("Nominal Frequency", "Hz", 100) },
    { 0x0E, make_value("Low Frequency 1 (LF1)", "Hz", 100) },
    { 0x0F, make_value("LF1 Maximum Trip Time (MTT)", "s", 10) },
    { 0x10, make_value("High Frequency 1 (HF1)", "Hz", 100) },
    { 0x11, make_value("HF1 Maximum Trip time (MTT)", "s", 10) },
    { 0x12, make_value("Low Frequency 2 (LF2)", "Hz", 100) },
    { 0x13, make_value("LF2 Maximum Trip Time (MTT)", "s", 10) },
    { 0x14, make_value("High Frequency 2 (HF2)", "Hz", 100) },
    { 0x15, make_value("HF2 Maximum Trip time (MTT)", "s", 10) },
    { 0x16, make_value("ID Function Activated", "bool", 1) },
    { 0x17, make_value("Reconnect Time (RT)", "s", 10) },
    { 0x18, make_value("Reconnect High Voltage (RHV)", "V", 10) },
    { 0x19, make_value("Reconnect Low Voltage (RLV)", "V", 10) },
    { 0x1A, make_value("Reconnect High Frequency (RHF)", "Hz", 100) },
    { 0x1B, make_value("Reconnect Low Frequency (RLF)", "Hz", 100) },
    { 0x1C, make_value("Normal Ramp up Rate (RUR_NM)", "% Rated/s", 100) },
    { 0x1D, make_value("Soft Start Ramp up Rate (RUR_SS)", "% Rated/s", 100) },
    { 0x1E, make_value("FW Function Activated", "bool", 1) },
    { 0x1F, make_value("Start of Frequency Watt Droop (Fstart)", "Hz", 100) },
    { 0x20, make_value("FW Droop Slope (Kpower_Freq)", "%Pn/Hz", 10) },
    { 0x21, make_value("Recovery Ramp Rate (RRR)", "%Pn/s", 100) },
    { 0x22, make_value("Recovery High Frequency (RVHF)", "Hz", 100) },
    { 0x23, make_value("Recovery Low Frequency (RVLF)", "Hz", 100) },
    { 0x24, make_value("VW Function Activated", "bool", 1) },
    { 0x25, make_value("Start of Voltage Watt Droop (Vstart)", "V", 10) },
    { 0x26, make_value("End of Voltage Watt Droop (Vend)", "V", 10) },
    { 0x27, make_value("Droop Slope (Kpower_Volt)", "%Pn/V", 100) },
    { 0x28, make_value("APC Function Activated", "bool", 1) },
    { 0x29, make_value("Power Ramp Rate (PRR)", "%Pn/s", 100) },
    { 0x2A, make_value("VV Function Activated", "bool", 1) },
    { 0x2B, make_value("Voltage Set Point V1", "V", 10) },
    { 0x2C, make_value("Reactive Set Point Q1", "%Pn", 10) },
    { 0x2D, make_value("Voltage Set Point V2", "V", 10) },
    { 0x2E, make_value("Voltage Set Point V3", "V", 10) },
    { 0x2F, make_value("Voltage Set Point V4", "V", 10) },
    { 0x30, make_value("Reactive Set Point Q4", "%Pn", 10) },
    { 0x31, make_value("VV Setting Time (Tr)", "s", 10) },
    { 0x32, make_value("SPF Function Activated", "bool", 1) },
    { 0x33, make_value("Power Factor (PF)", "", 100) },
    { 0x34, make_value("RPC Function Activated", "bool", 1) },
    { 0x35, make_value("Reactive Power (VAR)", "%Sn", 1) },
    { 0x36, make_value("WPF Function Activated", "bool", 1) },
    { 0x37, make_value("Start of Power of WPF (Pstart)", "%Pn", 10) },
    { 0x38, make_value("Power Factor at Rated Power (PFRP)", "", 100) },
    { 0x39, make_value("Low Voltage 3 (LV3)", "V", 10) },
    { 0x3A, make_value("LV3 Maximum Trip Time (MTT)", "s", 100) },
    { 0x3B, make_value("Momentary Cessation Low Voltage", "V", 10) },
    { 0x3C, make_value("Momentary Cessation High Voltage", "V", 10) },
    { 0x3D, make_value("FW Settling Time (Tr)", "s", 10) },
    { 0x3E, make_value("LF2 Maximum Trip Time (MTT)", "s", 100) },
    { 0x3F, make_value("HF2 Maximum Trip Time (MTT)", "s", 100) },
    { 0x40, make_value("Short Interruption Reconnect Time (SRT)", "s", 10) },
    { 0x41, make_value("Short Interruption Time (SIT)", "s", 10) },
    { 0xFF, make_value("Unknown Value", "", 1) }
};


class GridProfileParser {
public:
    void parse() {
        // Example of parsing logic
        // Let's say we're simply displaying some values
        try {
            auto profileName = profileSection.at(0x00);
            std::cout << "Profile Name: " << profileName.data() << std::endl;
        } catch (const std::out_of_range& e) {
            std::cout << "Profile not found." << std::endl;
        }
    }
};

int main() {
    GridProfileParser parser;
    parser.parse();
    return 0;
}
