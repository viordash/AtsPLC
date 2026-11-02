#!/usr/bin/env python3
"""Show a ladder program file (.dat, as the web UI downloads it) as a diagram or as C++ code.

Usage:
    ladder.py ascii program.dat    # ladder diagram drawn with text characters
    ladder.py cpp program.dat      # C++ code in the style of Ladder::InitialLoad()

The binary format follows Ladder::Serialize / Network::Serialize and the
Serialize() method of every element in PLC_esp8266/main/LogicProgram.
"""

import argparse
import re
import struct
import sys

IO = ['DI', 'AI', 'O1', 'O2', 'V1', 'V2', 'V3', 'V4']
DATETIME_PARTS = ['second', 'minute', 'hour', 'day', 'weekday', 'month', 'year']
NETWORK_STATES = ['nsPassive', 'nsActive', 'nsStopFromPassive', 'nsStopFromActive']
WORK_MODES = ['Stop', 'Run']
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

# TvElementType values
ET_INPUTS = {2: 'NC', 3: 'NO'}
ET_INPUT_CLASSES = {2: 'InputNC', 3: 'InputNO'}
ET_TIMER_SECS = 4
ET_TIMER_MSECS = 5
ET_COMPARATORS = {6: '==', 7: '!=', 8: '>=', 9: '>', 10: '<=', 11: '<'}
ET_COMPARATOR_CLASSES = {
    6: 'ComparatorEq',
    7: 'ComparatorNe',
    8: 'ComparatorGE',
    9: 'ComparatorGr',
    10: 'ComparatorLE',
    11: 'ComparatorLs',
}
ET_OUTPUTS = {12: 'Out', 13: 'Set', 14: 'Reset', 15: 'Inc', 16: 'Dec'}
ET_OUTPUT_CLASSES = {
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


def FormatFloat(value):
    """The shortest text that gives the same float32 value back."""
    for precision in range(1, 10):
        text = f'{value:.{precision}g}'
        if struct.pack('<f', float(text)) == struct.pack('<f', value):
            return text
    return repr(value)


def CppFloat(value):
    text = FormatFloat(value)
    return text if re.fullmatch(r'-?\d+', text) else text + 'f'


# An element is a tuple: (type, fields...). The functions below read it from
# binary and convert it to the ASCII diagram and C++.

def ReadElement(reader):
    t = reader.read('B')
    if t in ET_INPUTS or t in ET_OUTPUTS or t == ET_WIFI_STA:
        return (t, reader.read('B'))
    if t in (ET_TIMER_SECS, ET_TIMER_MSECS):
        return (t, reader.read('Q'))
    if t in ET_COMPARATORS:
        ref, io = reader.read('BB')
        return (t, io, ref)
    if t == ET_INDICATOR:
        return (t, *reader.read('BffB'))
    if t == ET_WIFI:
        return (t, reader.read('B'), reader.read_str(SSID_SIZE))
    if t == ET_WIFI_AP:
        io = reader.read('B')
        return (t, io, reader.read_str(SSID_SIZE), reader.read_str(PASSWORD_SIZE),
                reader.read_str(MAC_SIZE))
    if t == ET_SETTINGS:
        return (t, reader.read('I'))
    if t == ET_DATETIME:
        return (t, *reader.read('BI'))
    if t == ET_SQUARE_WAVE:
        return (t, *reader.read('QQ'))
    if t in (ET_CONTINUATION_IN, ET_CONTINUATION_OUT):
        reader.read('B')
        return (t,)
    raise ValueError(f'unknown element type {t} at offset {reader.pos - 1}')


def ElementCpp(e):
    """Returns a list of C++ lines; the last one is the expression to append."""
    t = e[0]
    if t in ET_INPUTS:
        return [f'new {ET_INPUT_CLASSES[t]}(MapIO::{IO[e[1]]})']
    if t == ET_TIMER_SECS:
        return [f'new TimerSecs({e[1] // 1000000})']
    if t == ET_TIMER_MSECS:
        return [f'new TimerMSecs({e[1] // 1000})']
    if t in ET_COMPARATORS:
        return [f'new {ET_COMPARATOR_CLASSES[t]}({e[2]}, MapIO::{IO[e[1]]})']
    if t in ET_OUTPUTS:
        return [f'new {ET_OUTPUT_CLASSES[t]}(MapIO::{IO[e[1]]})']
    if t == ET_INDICATOR:
        return [
            f'auto indicator = new Indicator(MapIO::{IO[e[1]]});',
            f'indicator->SetLowScale({CppFloat(e[2])});',
            f'indicator->SetHighScale({CppFloat(e[3])});',
            f'indicator->SetDecimalPoint({e[4]});',
            'indicator->UpdateScale();',
            'indicator',
        ]
    if t == ET_WIFI:
        return [f'new WiFiBinding(MapIO::{IO[e[1]]}, "{e[2]}")']
    if t == ET_WIFI_STA:
        return [f'new WiFiStaBinding(MapIO::{IO[e[1]]})']
    if t == ET_WIFI_AP:
        return [f'new WiFiApBinding(MapIO::{IO[e[1]]}, "{e[2]}", "{e[3]}", "{e[4]}")']
    if t == ET_SETTINGS:
        return [f'new SettingsElement(SettingsElement::Discriminator::{SETTINGS[e[1]]})']
    if t == ET_DATETIME:
        return [
            f'auto datetime = new DateTimeBinding(MapIO::{IO[e[1]]});',
            f'datetime->datetime_part = DatetimePart::t_{DATETIME_PARTS[e[2]]};',
            'datetime',
        ]
    if t == ET_SQUARE_WAVE:
        return [f'new SquareWaveGenerator({e[1] // 1000}, {e[2] // 1000})']
    if t == ET_CONTINUATION_IN:
        return ['new ContinuationIn()']
    if t == ET_CONTINUATION_OUT:
        return ['new ContinuationOut()']
    raise ValueError(f'unknown element type {t}')


ASCII_OUTPUTS = {12: '( )', 13: '(S)', 14: '(R)', 15: '(+)', 16: '(-)'}


def ElementAscii(e):
    """Returns (label, symbol): the label is drawn above the symbol."""
    t = e[0]
    if t in ET_INPUTS:
        return IO[e[1]], ']/[' if ET_INPUTS[t] == 'NC' else '] ['
    if t == ET_TIMER_SECS:
        return '', f'[TON {e[1] // 1000000}s]'
    if t == ET_TIMER_MSECS:
        return '', f'[TON {e[1] // 1000}ms]'
    if t in ET_COMPARATORS:
        return '', f'[{IO[e[1]]} {ET_COMPARATORS[t]} {e[2]}]'
    if t in ET_OUTPUTS:
        return IO[e[1]], ASCII_OUTPUTS[t]
    if t == ET_SQUARE_WAVE:
        return '', f'[SQW off {e[1] // 1000}ms on {e[2] // 1000}ms]'
    if t in (ET_CONTINUATION_IN, ET_CONTINUATION_OUT):
        return '', '>>'
    if t == ET_INDICATOR:
        point = f' dp={e[4]}' if e[4] else ''
        return '', f'[Indicator {IO[e[1]]} {FormatFloat(e[2])}..{FormatFloat(e[3])}{point}]'
    if t == ET_WIFI:
        return '', f'[WiFi {IO[e[1]]} "{e[2]}"]'
    if t == ET_WIFI_STA:
        return '', f'[WiFiSta {IO[e[1]]}]'
    if t == ET_WIFI_AP:
        return '', f'[WiFiAp {IO[e[1]]} "{e[2]}" "{e[3]}" "{e[4]}"]'
    if t == ET_SETTINGS:
        return '', f'[Settings {SETTINGS[e[1]][2:]}]'
    if t == ET_DATETIME:
        return '', f'[DateTime {DATETIME_PARTS[e[2]]} -> {IO[e[1]]}]'
    raise ValueError(f'unknown element type {t}')


def DefaultState(work_mode, disabled):
    if WORK_MODES[work_mode] == 'Run':
        return NETWORK_STATES.index('nsPassive' if disabled else 'nsActive')
    return NETWORK_STATES.index('nsStopFromPassive' if disabled else 'nsStopFromActive')


def ReadLadder(data):
    reader = Reader(data)
    work_mode = reader.read('B')
    networks = []
    for _ in range(reader.read('H')):
        state = reader.read('B')
        elements = [ReadElement(reader) for _ in range(reader.read('H'))]
        networks.append((state, elements))
    if reader.pos != len(data):
        raise ValueError(f'{len(data) - reader.pos} extra bytes at the end')
    return work_mode, networks


def AsciiLines(work_mode, networks, first_number):
    """Returns the lines of each network; all networks get the same width."""
    rows = []
    for i, (state, elements) in enumerate(networks):
        parts = [ElementAscii(e) for e in elements]
        starts_with_continuation = elements[0][0] == ET_CONTINUATION_OUT
        if starts_with_continuation:
            parts = parts[1:]
        split = next((n for n, e in enumerate(elements[len(elements) - len(parts):])
                      if e[0] in ET_OUTPUTS or e[0] == ET_CONTINUATION_IN), len(parts))
        disabled = '  (disabled)' if state == DefaultState(work_mode, True) else ''
        rows.append((first_number + i, starts_with_continuation, parts[:split], parts[split:],
                     disabled))

    def Cells(parts):
        labels, symbols = [], []
        for label, symbol in parts:
            width = max(len(label), len(symbol))
            labels.append(label.center(width))
            symbols.append(symbol.center(width, '-'))
        return '  '.join(labels), '--'.join(symbols)

    drawn = []
    for number, continuation, inputs, outputs, disabled in rows:
        left = '>>' if continuation else '||'
        in_labels, in_symbols = Cells(inputs)
        out_labels, out_symbols = Cells(outputs)
        in_labels = ('  ' + in_labels) if inputs else ''
        in_symbols = ('--' + in_symbols) if inputs else ''
        ends_with_continuation = outputs and outputs[-1][1] == '>>'
        right = '' if ends_with_continuation else '--||'
        drawn.append((number, left, in_labels, in_symbols, out_labels, out_symbols, right,
                      disabled))

    width = max(len(d[3]) + len(d[5]) + len(d[6]) for d in drawn) + 2
    result = []
    for number, left, in_labels, in_symbols, out_labels, out_symbols, right, disabled in drawn:
        fill = width - len(in_symbols) - len(out_symbols) - len(right)
        lines = []
        label_row = f'{"":2}  {"":2}{in_labels.ljust(len(in_symbols) + fill)}{out_labels}'
        if label_row.strip():
            lines.append(label_row.rstrip())
        lines.append(f'{number:2}  {left}{in_symbols}{"-" * fill}{out_symbols}{right}{disabled}')
        result.append(lines)
    return result


def Ascii(work_mode, networks):
    for lines in AsciiLines(work_mode, networks, 1):
        print('\n'.join(lines))
        print()


def Cpp(work_mode, networks):
    lines = ['void Ladder::InitialLoad() {']
    for i, (state, elements) in enumerate(networks):
        for diagram_line in AsciiLines(work_mode, [(state, elements)], i + 1)[0]:
            lines.append(f'    // {diagram_line}')
        lines.append('    {')
        lines.append(f'        auto network = new Network(NetworkState::{NETWORK_STATES[state]});')
        for e in elements:
            cpp = ElementCpp(e)
            lines += [f'        {line}' for line in cpp[:-1]]
            lines.append(f'        network->Append({cpp[-1]});')
        lines.append('        Append(network);')
        lines.append('    }')
    lines.append('}')
    print('\n'.join(lines))


def main():
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument('command', choices=['ascii', 'cpp'])
    parser.add_argument('file')
    args = parser.parse_args()

    with open(args.file, 'rb') as f:
        work_mode, networks = ReadLadder(f.read())
    if args.command == 'ascii':
        Ascii(work_mode, networks)
    else:
        Cpp(work_mode, networks)


if __name__ == '__main__':
    sys.exit(main())
