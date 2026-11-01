#!/usr/bin/env python3
"""Decode a ladder program file (.dat, as downloaded from the web UI).

Usage:
    ladder.py dump program.dat     # readable listing, one network per line
    ladder.py cpp program.dat      # C++ code in the style of Ladder::InitialLoad()

The binary format follows Ladder::Serialize / Network::Serialize and the
Serialize() method of every element in PLC_esp8266/main/LogicProgram.
"""

import argparse
import struct
import sys

IO = ['DI', 'AI', 'O1', 'O2', 'V1', 'V2', 'V3', 'V4']
DATETIME_PARTS = ['second', 'minute', 'hour', 'day', 'weekday', 'month', 'year']
NETWORK_STATES = ['nsPassive', 'nsActive', 'nsStopFromPassive', 'nsStopFromActive']
SETTINGS = [
    't_wifi_station_settings_ssid',
    't_wifi_station_settings_password',
    't_wifi_station_settings_reconnect_delay_ms',
    't_wifi_station_settings_scan_station_rssi_period_ms',
    't_wifi_station_settings_max_rssi',
    't_wifi_station_settings_min_rssi',
    't_wifi_station_settings_min_worktime_ms',
    't_wifi_scanner_settings_per_channel_scan_time_ms',
    't_wifi_scanner_settings_max_rssi',
    't_wifi_scanner_settings_min_rssi',
    't_wifi_access_point_settings_generation_time_ms',
    't_wifi_access_point_settings_ssid_hidden',
    't_current_date',
    't_current_time',
    't_datetime_sntp_server_primary',
    't_datetime_sntp_server_secondary',
    't_datetime_timezone',
    't_adc_scan_period_ms',
]

# TvElementType -> (class name, comparison sign for comparators)
ET_WIRE = 1
ET_INPUTS = {2: 'InputNC', 3: 'InputNO'}
ET_TIMER_SECS = 4
ET_TIMER_MSECS = 5
ET_COMPARATORS = {
    6: ('ComparatorEq', '=='),
    7: ('ComparatorNe', '!='),
    8: ('ComparatorGE', '>='),
    9: ('ComparatorGr', '>'),
    10: ('ComparatorLE', '<='),
    11: ('ComparatorLs', '<'),
}
ET_OUTPUTS = {
    12: 'DirectOutput',
    13: 'SetOutput',
    14: 'ResetOutput',
    15: 'IncOutput',
    16: 'DecOutput',
}
ET_INDICATOR = 17
ET_WIFI = 18
ET_WIFI_STA = 19
ET_WIFI_AP = 20
ET_SETTINGS = 21
ET_DATETIME = 22
ET_SQUARE_WAVE = 23
ET_CONTINUATION_IN = 24
ET_CONTINUATION_OUT = 25

SSID_SIZE = 24 + 1
PASSWORD_SIZE = 16 + 1
MAC_SIZE = 12 + 1


class Reader:
    def __init__(self, data):
        self.data = data
        self.pos = 0

    def read(self, fmt):
        values = struct.unpack_from('<' + fmt, self.data, self.pos)
        self.pos += struct.calcsize('<' + fmt)
        return values if len(values) > 1 else values[0]

    def read_str(self, size):
        raw = self.data[self.pos:self.pos + size]
        if len(raw) != size:
            raise ValueError('unexpected end of data')
        self.pos += size
        return raw.split(b'\0')[0].decode('ascii')


def ReadElement(reader):
    """Returns (text, cpp) for one element; cpp is a list of lines, the last one is the expression."""
    t = reader.read('B')
    if t in ET_INPUTS:
        io = IO[reader.read('B')]
        name = ET_INPUTS[t]
        return f'{name[5:]}[{io}]', [f'new {name}(MapIO::{io})']
    if t == ET_TIMER_SECS:
        secs = reader.read('Q') // 1000000
        return f'Timer[{secs}s]', [f'new TimerSecs({secs})']
    if t == ET_TIMER_MSECS:
        msecs = reader.read('Q') // 1000
        return f'Timer[{msecs}ms]', [f'new TimerMSecs({msecs})']
    if t in ET_COMPARATORS:
        ref, io = reader.read('BB')
        name, sign = ET_COMPARATORS[t]
        return f'{IO[io]}{sign}{ref}', [f'new {name}({ref}, MapIO::{IO[io]})']
    if t in ET_OUTPUTS:
        io = IO[reader.read('B')]
        name = ET_OUTPUTS[t]
        return f'{name[:-6] or "Out"}({io})', [f'new {name}(MapIO::{io})']
    if t == ET_INDICATOR:
        io, low, high, point = reader.read('BffB')
        io = IO[io]
        return f'Indicator[{io} {low:g}..{high:g}]', [
            f'auto indicator = new Indicator(MapIO::{io});',
            f'indicator->SetLowScale({FormatFloat(low)});',
            f'indicator->SetHighScale({FormatFloat(high)});',
            f'indicator->SetDecimalPoint({point});',
            'indicator->UpdateScale();',
            'indicator',
        ]
    if t == ET_WIFI:
        io = IO[reader.read('B')]
        ssid = reader.read_str(SSID_SIZE)
        return f'WiFi[{io} "{ssid}"]', [f'new WiFiBinding(MapIO::{io}, "{ssid}")']
    if t == ET_WIFI_STA:
        io = IO[reader.read('B')]
        return f'WiFiSta[{io}]', [f'new WiFiStaBinding(MapIO::{io})']
    if t == ET_WIFI_AP:
        io = IO[reader.read('B')]
        ssid = reader.read_str(SSID_SIZE)
        password = reader.read_str(PASSWORD_SIZE)
        mac = reader.read_str(MAC_SIZE)
        return f'WiFiAp[{io} "{ssid}"]', [
            f'new WiFiApBinding(MapIO::{io}, "{ssid}", "{password}", "{mac}")'
        ]
    if t == ET_SETTINGS:
        discriminator = SETTINGS[reader.read('I')]
        return f'Settings[{discriminator[2:]}]', [
            f'new SettingsElement(SettingsElement::Discriminator::{discriminator})'
        ]
    if t == ET_DATETIME:
        io, part = reader.read('BI')
        io = IO[io]
        part = DATETIME_PARTS[part]
        return f'DateTime[{part} -> {io}]', [
            f'auto datetime = new DateTimeBinding(MapIO::{io});',
            f'datetime->datetime_part = DatetimePart::t_{part};',
            'datetime',
        ]
    if t == ET_SQUARE_WAVE:
        period0, period1 = reader.read('QQ')
        period0 //= 1000
        period1 //= 1000
        return f'SquareWave[{period0}ms/{period1}ms]', [
            f'new SquareWaveGenerator({period0}, {period1})'
        ]
    if t == ET_CONTINUATION_IN:
        reader.read('B')
        return 'ContIn', ['new ContinuationIn()']
    if t == ET_CONTINUATION_OUT:
        reader.read('B')
        return 'ContOut', ['new ContinuationOut()']
    raise ValueError(f'unknown element type {t} at offset {reader.pos - 1}')


def FormatFloat(value):
    if value == int(value):
        return str(int(value))
    return f'{value:g}f'


def ReadLadder(data):
    reader = Reader(data)
    work_mode = reader.read('B')
    networks = []
    for _ in range(reader.read('H')):
        state = reader.read('B')
        elements = [ReadElement(reader) for _ in range(reader.read('H'))]
        networks.append((NETWORK_STATES[state], elements))
    if reader.pos != len(data):
        raise ValueError(f'{len(data) - reader.pos} extra bytes at the end')
    return work_mode, networks


def Dump(work_mode, networks):
    print(f'work_mode: {"Run" if work_mode else "Stop"}, networks: {len(networks)}')
    for i, (_, elements) in enumerate(networks):
        print(f'{i + 1:2}  ' + ' -- '.join(text for text, _ in elements))


def Cpp(networks):
    lines = ['void Ladder::InitialLoad() {']
    for i, (state, elements) in enumerate(networks):
        lines.append(f'    // {i + 1}: ' + ' -- '.join(text for text, _ in elements))
        lines.append('    {')
        lines.append(f'        auto network = new Network(NetworkState::{state});')
        for _, cpp in elements:
            lines += [f'        {line}' for line in cpp[:-1]]
            lines.append(f'        network->Append({cpp[-1]});')
        lines.append('        Append(network);')
        lines.append('    }')
    lines.append('}')
    print('\n'.join(lines))


def main():
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument('command', choices=['dump', 'cpp'])
    parser.add_argument('file')
    args = parser.parse_args()

    with open(args.file, 'rb') as f:
        work_mode, networks = ReadLadder(f.read())
    if args.command == 'dump':
        Dump(work_mode, networks)
    else:
        Cpp(networks)


if __name__ == '__main__':
    sys.exit(main())
