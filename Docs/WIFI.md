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

While a request of one type is running, a request of another type interrupts it: the
running task periodically checks its break bit and, once set, finishes and frees the radio.

## Station (`WiFiService_Station.cpp`)

Connects to the network saved in settings. Retry count and delay between retries come from
settings; `-1` means unlimited retries.

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

The access point's lifetime is limited by `generation_time_ms`; `ssid_hidden` hides the SSID
from the air.

## WiFi setup

Station credentials are set either via SmartConfig in service mode, see
[SERVICE_MODE.md](SERVICE_MODE.md), or via the settings element in the program, see
[LADDER_ELEMENTS.md](LADDER_ELEMENTS.md).
