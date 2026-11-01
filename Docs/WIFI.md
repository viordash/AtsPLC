# WiFi

One radio, three modes: station, scanner, access point. Only one runs at a time.
`WiFiService` keeps a request queue and switches modes.

Requests come from program elements: `WiFiStaBinding` asks for the station, `WiFiBinding`
for scanning, `WiFiApBinding` for the access point.

## Request queue (`WiFiRequests.cpp`)

### Request

| Type | Payload |
|------|---------|
| `wqi_Station` | none |
| `wqi_Scanner` | ssid |
| `wqi_AccessPoint` | ssid, password, MAC mask |

### Queuing a request

A request is queued only if it's not already there. Comparison is by type and by the
payload fields' pointers: two access-point requests with the same ssid but a different
password or MAC mask are considered different.

Pointers, not strings: the ssid and password live in the program element itself, for as
long as the element exists.

### Storage

The queue is a fixed-size array (`WiFi_RequestsLimit`, 5) with a counter. The limit is
picked from the model: at most one request per variable plus the station. On overflow the
request is dropped and logged - the number of requests is set by the user's program, so it
must never crash because of this.

Service order is FIFO.

### Preemption

The station and the access point give the radio to other requests: they periodically check
`HasAnother` and, once another request is queued, finish and free the radio. The scanner
doesn't check the queue: one scan pass ends when the ssid is found, after
14 x `per_channel_scan_time_ms`, or on `SCAN_BREAK_BIT`.

### Cancellation

`CancelScan`, `CancelAccessPoint` and `DisconnectFromStation` drop the matching request
from the queue and raise the break bit of their type (`SCAN_BREAK_BIT`, `AP_BREAK_BIT`,
`STA_BREAK_BIT`). The bit interrupts the running task of that type at once, whatever ssid
it serves.

The bits live in the `WiFiService` event group. Every wait of the WiFi task reads and
clears all bits, so a bit raised while another mode runs is consumed by that mode. A new
request also drops a pending break bit of its own type (`ClearBreakFlags`), so a late
cancellation does not stop it.

## Station (`WiFiService_Station.cpp`)

The station is the default mode: when the queue is empty, the WiFi task runs the station
without a request. Scanner and access point requests take the radio for a while and give
it back. `DisconnectFromStation` frees the radio rather than turning the station off: with
an empty queue the station comes up again right away. Only an empty ssid in settings keeps
the station off.

Connects to the network saved in settings. Without a saved ssid the task leaves the radio
alone and waits for a new request or for a WiFi settings change made by the settings
element. On a connection failure the task ends and frees the radio; the next attempt comes
with a `WiFiStaBinding` request or when the queue is empty again. Before ending, the task
waits `reconnect_delay_ms` - only when no other request is queued, and minus the time
already spent waiting for the disconnection.

After four failures in a row the signal level drops to 0 and the program is woken up once;
a successful connection resets the counter.

The web server comes up once connected and goes down on disconnect.

Signal level is polled at a period from settings and scaled to 0..255 using the
`min_rssi` / `max_rssi` bounds.

The minimum uptime (`min_worktime_ms`) keeps a freshly established connection from being
torn down right away: a request of another type will wait.

## Scanner (`WiFiService_Scanner.cpp`)

Passive scan across all 14 channels, time per channel comes from settings. Found networks
go into the `ScannedSsid` array (capped at `WiFi_SsidLimit`, 4 - one per variable).

## Access point (`WiFiService_AccessPoint.cpp`)

Brings up an AP with the given SSID and password. Without a password the AP is open and
doesn't accept connections - this mode is used as a beacon.

Clients are filtered by MAC mask: a connection that doesn't match the mask is disconnected.
Accepted clients go into `ApClients` - up to `WiFi_Hotspot_Max_Clients` (4) per SSID.

The access point runs until it is cancelled. Every `generation_time_ms` it checks the queue
and frees the radio only when another request is waiting; with an empty queue it keeps
running. `ssid_hidden` hides the SSID from the air.

## WiFi setup

Station credentials are set either via SmartConfig in service mode, see
[SERVICE_MODE.md](SERVICE_MODE.md), or via the settings element in the program, see
[LADDER_ELEMENTS.md](LADDER_ELEMENTS.md).
