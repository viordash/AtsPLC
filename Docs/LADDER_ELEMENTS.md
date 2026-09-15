# Ladder Diagram Elements

A program is a list of networks. A network is a chain of elements left to right: from the
incoming rail to the outgoing one. Each element takes the previous element's state and
passes its own state further.

Limits: 1..80 networks per program, 1..5 elements per network, 2 networks visible on screen.

I/O addresses: `DI` (digital input), `AI` (analog input), `O1`, `O2` (outputs), `V1`..`V4`
(variables).

## Inputs

| Element | Description |
|---------|-------------|
| **InputNO** | normally open contact: passes through when the address value is not 0 |
| **InputNC** | normally closed contact: passes through when the address value is 0 |
| **TimerSecs** | delay in seconds, 1..99999 |
| **TimerMSecs** | delay in milliseconds, 50..99950, step 50 |
| **SquareWaveGenerator** | pulse generator, pause and pulse durations are configurable |
| **ComparatorEq / Ne / GE / Gr / LE / Ls** | compares the address value against a setpoint |
| **Indicator** | shows the address value with scaling and a decimal point |

A timer starts counting when the previous element becomes active, and passes the signal
through once the time elapses. Any state change of the previous element resets the count.

## Outputs

| Element | Description |
|---------|-------------|
| **DirectOutput** | writes 255 while the input is active, 0 while passive |
| **SetOutput** | writes 255 while the input is active, latching |
| **ResetOutput** | writes 0 while the input is active |
| **IncOutput** | increases the value by 1, caps at 255 |
| **DecOutput** | decreases the value by 1, floors at 0 |

An output element ends the chain and is drawn at the outgoing rail.

## Flow between networks

| Element | Description |
|---------|-------------|
| **ContinuationOut** | ends a network, passing its state to the next one |
| **ContinuationIn** | starts a network with the state passed from the previous one |

The pair lets you build a chain longer than five elements by splitting it across several
networks.

## Bindings

A binding ties a variable to an external source: while the binding is active, the variable
reflects the source's state.

### WiFiBinding

Scans the air for a network with the given SSID and puts its signal level into the variable,
scaled to 0..255. Zero means the network wasn't found.

### WiFiStaBinding

Connects the device to the saved WiFi station and puts the signal level into the variable.
The web server comes up once connected.

### WiFiApBinding

Brings up an access point with the given SSID and password, and puts the number of
connected clients into the variable. A MAC mask filters who counts as a client: `*` in a
position means "any".

### DateTimeBinding

Puts a part of the current time into the variable: second, minute, hour, day, weekday,
month, or year. The element asks the controller to wake it up with the needed period.

## Settings element (SettingsElement)

Shows and edits one device settings field right in the chain: WiFi station, scanner, access
point, SNTP, ADC settings, current date and time. The value is read from settings on every
render and shown regardless of the chain's state; the station password is masked with
asterisks, and `connect_max_retry_count` set to `-1` is shown as `infinity`.

## Editing modes

Editing is available in Stop and Debug modes, see [WORK_MODES.md](WORK_MODES.md).

### Basic editing

Select on the main screen picks a network, the next Select enters it, then Up/Down switch
between elements and their parameters, Select confirms.

### Advanced modes

A long press of Select on a selected network opens actions for the whole network:

| Action | What it does |
|--------|--------------|
| Move | move the network up/down |
| Copy | duplicate the network |
| Delete | delete the network |
| Disable | turn the network off without deleting it |

A disabled network is drawn with a dashed incoming rail. There's no separate "disabled"
state: the network is switched to the passive state, so its elements get a passive input and
never turn anything on.

## Technical details

### Controller cycle

One cycle: poll inputs, walk all networks (`Ladder::DoAction`), write outputs. The walk
repeats while elements keep changing state, but no longer than two system ticks (20 ms); if
changes haven't settled by then, the rest continues on the next pass.

### Wakeup priorities

Elements ask the controller to wake them up after a given time. A critical-priority request
(timers, generator) fires exactly on time; an idle-priority request (cursor blink, indicator
refresh) can be merged with a nearby one, so the controller isn't woken up more than needed.

### Storage

The program is serialized to a binary form keyed by element type (`TvElementType`) and
written through `redundant_storage` to both partitions at once. The work mode is written to
the file together with the networks.
