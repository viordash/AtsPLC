# Service mode

A separate device mode for maintenance: choosing the work mode, WiFi setup, backing up and
restoring programs, resetting settings.

## Entering service mode

Power on with **UP** held.

Menu navigation - Up/Down, select - Select. The menu scrolls: the screen fits 4 lines, with
a scrollbar on the right. The mode times out after 120 seconds of inactivity, after which
service mode ends and the device continues its normal startup (no reboot).

## Available functions

| Function | Description |
|----------|--------------|
| **Work mode** | Choose the work mode: Stop / Run / Debug |
| **Smart config** | Set up WiFi via the ESP-Touch app |
| **Backup logic** | Save the program to one of 4 slots (ladder_0 - ladder_3) |
| **Restore logic** | Restore the program from a slot |
| **Reset to default** | Reset: settings, program, backups, or all at once |

## Work mode

Switches the controller between Stop, Run, and Debug. The current mode is shown in the
screen title. Select on the chosen mode opens a confirmation screen: **UP** applies the
mode, any other button cancels. The device reboots either way - into the chosen mode or
back into the previous one. See [WORK_MODES.md](WORK_MODES.md) for mode details.

## Backup / Restore

4 independent slots (`ladder_0`-`ladder_3`) are available in the `backups` partition. Backup
saves the current working program into the chosen slot; Restore overwrites the working
program with the slot's contents. Occupied slots are marked `(stored)` in the list;
restoring from an empty slot is treated as an error.

These actions have no separate confirmation - Select on the chosen slot runs them right
away. The result is shown on screen (`Backup completed!` / `Restore error!`, etc.), exit
with Select. Unlike Work mode and Reset, there's no reboot after Backup or Restore.

## Reset

Four independent reset levels - from narrow (only WiFi/SNTP/ADC settings) to full (Factory
reset - program, backups, and settings at once). The level is chosen with the same Up/Down/
Select navigation as picking a service mode function.

| Level | What gets deleted |
|-------|--------------------|
| Settings | device settings |
| Ladder program | the working program |
| Backups | all 4 slots |
| Factory reset | everything above |

Confirming any reset - the **UP** button; any other button cancels.
