# Work modes

The work mode determines three things: whether program networks execute, what happens to
the physical outputs, and whether on-device program editing is allowed.

| Mode | Networks | Physical outputs | Editing |
|------|----------|-------------------|---------|
| **Stop** | not executed | reset to 0 | allowed |
| **Run** | executed | follow program logic | forbidden |
| **Debug** | executed | follow program logic | allowed |

## Stop

All networks switch to the stopped state, elements stop computing, polling inputs has no
effect on the result.

- Output elements (direct assignment, set, increment, decrement) write 0 to their address
  once. A latch set by the Set element is released. The reset element has nothing to write:
  it only ever writes 0, so in Stop it simply stays idle.
- Timers and the pulse generator drop their wakeup requests - the controller stops waking up
  for their periods.
- Bindings to a WiFi network, a WiFi station, an access point, and date/time unbind their
  variables.
- Each network's outgoing rail is drawn dashed. Wires and element icons are drawn as active,
  so the program stays readable.

A variable value changed by increment or decrement elements resets to 0 when switching to
Stop.

## Run

Networks execute, outputs follow program logic. On-device editing is forbidden: pressing
Select on the main screen doesn't open network selection, and a message is logged. Both
rails of each network are drawn solid.

## Debug

Differs from Run only in that on-device program editing is allowed. Networks execute,
outputs are live.

Debug looks the same as Run on screen - the current mode is only shown by the **Work mode**
item in service mode.

The Debug flag doesn't survive a power loss: after power is removed, the device comes up in
Run. A software restart, including after an OTA update, keeps the mode.

## Disabled networks

A single network can be disabled (advanced editing mode, Disable). Switching the work mode
preserves this: a network disabled before switching to Stop comes back disabled after
switching back. A disabled network is drawn with a dashed incoming rail in any mode.

## Storage

- **Stop / Run** is stored in the program file together with network states and survives a
  power loss. The default is Stop.
- **The Debug flag** is stored in RTC memory. It survives a software restart and is lost on
  power loss.

The dangerous combination of "live outputs + editing allowed" can't be represented in
persistent storage: there's nothing to reset - a power loss always drops Debug down to
plain Run.

## Changing the mode

Two ways:

- **From the device.** Power on with **UP** held, pick **Work mode**, choose Stop / Run /
  Debug with Up/Down, Select. Select opens a confirmation screen: **UP** applies the mode,
  any other button cancels. The device reboots either way - into the chosen mode or back
  into the previous one.
- **From the browser.** `POST /workmode` with `{"mode":<0|1|2>}`. Changing the mode is
  forbidden while the program is being edited on the device. See
  [WEB_INTERFACE.md](WEB_INTERFACE.md).

The program file format changed when work modes were added: `LADDER_VERSION` was bumped, so
programs saved by earlier firmware don't load.
