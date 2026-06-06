#!/usr/bin/env python3
"""Configure OpenHAB MQTT items for Teplica greenhouse controller."""

import base64
import json
import urllib.error
import urllib.parse
import urllib.request

OH_BASE = "http://192.168.0.51:8080/rest"
AUTH = "Basic " + base64.b64encode(b"admin:admin").decode()
BRIDGE_UID = "mqtt:broker:9d15be98b2"
THING_UID = "mqtt:topic:9d15be98b2:teplicawin"
THING_LABEL = "Теплица — окна и дверь"
GROUP = "Teplica"
TOPIC = "teplica"

MOTORS = [
    ("0", "Окно ближнее"),
    ("1", "Окно дальнее"),
    ("2", "Дверь дальняя"),
]


def request(method, path, data=None):
    url = OH_BASE + path
    body = None
    headers = {"Authorization": AUTH, "Accept": "application/json"}
    if data is not None:
        body = json.dumps(data, ensure_ascii=False).encode("utf-8")
        headers["Content-Type"] = "application/json"
    req = urllib.request.Request(url, data=body, headers=headers, method=method)
    try:
        with urllib.request.urlopen(req) as resp:
            if resp.length and resp.length > 0:
                return json.loads(resp.read().decode("utf-8"))
            return None
    except urllib.error.HTTPError as err:
        detail = err.read().decode("utf-8", errors="replace")
        raise RuntimeError(f"{method} {path} -> {err.code}: {detail}") from err


def relates_to_for_item(name):
    if name.endswith("_Temp") or name.endswith("_ThOpen") or name.endswith("_ThClose"):
        return "Property_Temperature"
    if name.endswith("_State") or name.endswith("_Open") or name.endswith("_Close"):
        return "Property_Opening"
    if name.endswith("_AutoOpen") or name.endswith("_AutoClose") or name.endswith("_Paused"):
        return "Property_Control"
    if name.endswith("_RestoreMin") or name.endswith("_RestoreLeft"):
        return "Property_Time"
    if name.endswith("_Rssi"):
        return "Property_Signal_Strength"
    return "Property_Control"


def put_semantics(name):
    config = {"hasLocation": GROUP}
    relates_to = relates_to_for_item(name)
    if relates_to:
        config["relatesTo"] = relates_to

    request(
        "PUT",
        f"/items/{urllib.parse.quote(name)}/metadata/semantics",
        {"value": "Point", "config": config},
    )


def put_item(name, item_type, label, category="", tags=None):
    request(
        "PUT",
        f"/items/{urllib.parse.quote(name)}",
        {
            "type": item_type,
            "name": name,
            "label": label,
            "category": category,
            "groupNames": [GROUP],
            "tags": tags or ["Point"],
        },
    )
    put_semantics(name)


def put_link(item_name, channel_uid):
    request(
        "PUT",
        f"/links/{urllib.parse.quote(item_name)}/{urllib.parse.quote(channel_uid, safe='')}",
        {"itemName": item_name, "channelUID": channel_uid},
    )


def channel(uid_suffix, channel_id, channel_type, item_type, label, config):
    return {
        "uid": f"{THING_UID}:{uid_suffix}",
        "id": channel_id,
        "channelTypeUID": channel_type,
        "itemType": item_type,
        "kind": "STATE",
        "label": label,
        "defaultTags": [],
        "properties": {},
        "configuration": config,
    }


def build_channels():
    channels = [
        channel(
            "temperature",
            "temperature",
            "mqtt:number",
            "Number:Temperature",
            "Температура",
            {"stateTopic": f"{TOPIC}/temperature"},
        ),
        channel(
            "rssi",
            "rssi",
            "mqtt:number",
            "Number",
            "WiFi RSSI",
            {"stateTopic": f"{TOPIC}/rssi"},
        ),
    ]

    for motor_id, motor_label in MOTORS:
        p = f"motor/{motor_id}"
        prefix = f"m{motor_id}"

        channels.extend(
            [
                channel(
                    f"{prefix}_state",
                    f"{prefix}_state",
                    "mqtt:switch",
                    "Switch",
                    f"{motor_label} — открыто",
                    {
                        "stateTopic": f"{TOPIC}/{p}/state",
                        "on": "open",
                        "off": "closed",
                    },
                ),
                channel(
                    f"{prefix}_open",
                    f"{prefix}_open",
                    "mqtt:switch",
                    "Switch",
                    f"{motor_label} — открыть",
                    {
                        "commandTopic": f"{TOPIC}/{p}/command/open",
                        "on": "1",
                        "off": "0",
                    },
                ),
                channel(
                    f"{prefix}_close",
                    f"{prefix}_close",
                    "mqtt:switch",
                    "Switch",
                    f"{motor_label} — закрыть",
                    {
                        "commandTopic": f"{TOPIC}/{p}/command/close",
                        "on": "1",
                        "off": "0",
                    },
                ),
                channel(
                    f"{prefix}_auto_open",
                    f"{prefix}_auto_open",
                    "mqtt:switch",
                    "Switch",
                    f"{motor_label} — автооткрытие",
                    {
                        "stateTopic": f"{TOPIC}/{p}/auto_open",
                        "commandTopic": f"{TOPIC}/{p}/set/auto_open",
                        "on": "1",
                        "off": "0",
                    },
                ),
                channel(
                    f"{prefix}_auto_close",
                    f"{prefix}_auto_close",
                    "mqtt:switch",
                    "Switch",
                    f"{motor_label} — автозакрытие",
                    {
                        "stateTopic": f"{TOPIC}/{p}/auto_close",
                        "commandTopic": f"{TOPIC}/{p}/set/auto_close",
                        "on": "1",
                        "off": "0",
                    },
                ),
                channel(
                    f"{prefix}_th_open",
                    f"{prefix}_th_open",
                    "mqtt:number",
                    "Number:Temperature",
                    f"{motor_label} — порог открытия",
                    {
                        "stateTopic": f"{TOPIC}/{p}/threshold_open",
                        "commandTopic": f"{TOPIC}/{p}/set/threshold_open",
                        "min": "0",
                        "max": "50",
                    },
                ),
                channel(
                    f"{prefix}_th_close",
                    f"{prefix}_th_close",
                    "mqtt:number",
                    "Number:Temperature",
                    f"{motor_label} — порог закрытия",
                    {
                        "stateTopic": f"{TOPIC}/{p}/threshold_close",
                        "commandTopic": f"{TOPIC}/{p}/set/threshold_close",
                        "min": "0",
                        "max": "50",
                    },
                ),
                channel(
                    f"{prefix}_restore_min",
                    f"{prefix}_restore_min",
                    "mqtt:number",
                    "Number",
                    f"{motor_label} — восст. авто, мин",
                    {
                        "stateTopic": f"{TOPIC}/{p}/auto_restore_min",
                        "commandTopic": f"{TOPIC}/{p}/set/auto_restore_min",
                        "min": "0",
                        "max": "1440",
                    },
                ),
                channel(
                    f"{prefix}_paused",
                    f"{prefix}_paused",
                    "mqtt:switch",
                    "Switch",
                    f"{motor_label} — пауза авто",
                    {
                        "stateTopic": f"{TOPIC}/{p}/auto_paused",
                        "on": "1",
                        "off": "0",
                    },
                ),
                channel(
                    f"{prefix}_restore_left",
                    f"{prefix}_restore_left",
                    "mqtt:string",
                    "String",
                    f"{motor_label} — осталось, мин",
                    {"stateTopic": f"{TOPIC}/{p}/auto_restore_left"},
                ),
            ]
        )

    return channels


def build_items():
    items = [
        ("TeplicaWin_Temp", "Number:Temperature", "Температура", "temperature"),
        ("TeplicaWin_Rssi", "Number", "WiFi RSSI", "temperature"),
    ]

    for motor_id, motor_label in MOTORS:
        prefix = f"m{motor_id}"
        items.extend(
            [
                (f"TeplicaWin_{prefix}_State", "Switch", f"{motor_label} — открыто", "window"),
                (f"TeplicaWin_{prefix}_Open", "Switch", f"{motor_label} — открыть", "window"),
                (f"TeplicaWin_{prefix}_Close", "Switch", f"{motor_label} — закрыть", "window"),
                (f"TeplicaWin_{prefix}_AutoOpen", "Switch", f"{motor_label} — автооткрытие", "settings"),
                (f"TeplicaWin_{prefix}_AutoClose", "Switch", f"{motor_label} — автозакрытие", "settings"),
                (f"TeplicaWin_{prefix}_ThOpen", "Number:Temperature", f"{motor_label} — порог открытия", "temperature"),
                (f"TeplicaWin_{prefix}_ThClose", "Number:Temperature", f"{motor_label} — порог закрытия", "temperature"),
                (f"TeplicaWin_{prefix}_RestoreMin", "Number", f"{motor_label} — восст. авто, мин", "time"),
                (f"TeplicaWin_{prefix}_Paused", "Switch", f"{motor_label} — пауза авто", "settings"),
                (f"TeplicaWin_{prefix}_RestoreLeft", "String", f"{motor_label} — осталось, мин", "time"),
            ]
        )

    return items


def build_links():
    links = [
        ("TeplicaWin_Temp", f"{THING_UID}:temperature"),
        ("TeplicaWin_Rssi", f"{THING_UID}:rssi"),
    ]

    for motor_id, _ in MOTORS:
        prefix = f"m{motor_id}"
        links.extend(
            [
                (f"TeplicaWin_{prefix}_State", f"{THING_UID}:{prefix}_state"),
                (f"TeplicaWin_{prefix}_Open", f"{THING_UID}:{prefix}_open"),
                (f"TeplicaWin_{prefix}_Close", f"{THING_UID}:{prefix}_close"),
                (f"TeplicaWin_{prefix}_AutoOpen", f"{THING_UID}:{prefix}_auto_open"),
                (f"TeplicaWin_{prefix}_AutoClose", f"{THING_UID}:{prefix}_auto_close"),
                (f"TeplicaWin_{prefix}_ThOpen", f"{THING_UID}:{prefix}_th_open"),
                (f"TeplicaWin_{prefix}_ThClose", f"{THING_UID}:{prefix}_th_close"),
                (f"TeplicaWin_{prefix}_RestoreMin", f"{THING_UID}:{prefix}_restore_min"),
                (f"TeplicaWin_{prefix}_Paused", f"{THING_UID}:{prefix}_paused"),
                (f"TeplicaWin_{prefix}_RestoreLeft", f"{THING_UID}:{prefix}_restore_left"),
            ]
        )

    return links


def main():
    channels = build_channels()
    thing_body = {
        "UID": THING_UID,
        "thingTypeUID": "mqtt:topic",
        "label": THING_LABEL,
        "bridgeUID": BRIDGE_UID,
        "channels": channels,
    }

    try:
        request("PUT", f"/things/{urllib.parse.quote(THING_UID, safe='')}", thing_body)
    except RuntimeError as err:
        if "404" not in str(err):
            raise
        request("POST", "/things", thing_body)

    print(f"Thing created: {THING_UID} ({len(channels)} channels)")

    for name, item_type, label, category in build_items():
        put_item(name, item_type, label, category)
        print(f"Item: {name}")

    for item_name, channel_uid in build_links():
        put_link(item_name, channel_uid)
        print(f"Link: {item_name} <- {channel_uid}")

    print("Done. Items are in group 'Teplica' (Теплица), semantics: Point.")


def set_semantics_only():
    for name, _, _, _ in build_items():
        put_semantics(name)
        print(f"Semantics: {name} -> Point")


if __name__ == "__main__":
    import sys

    if len(sys.argv) > 1 and sys.argv[1] == "--semantics-only":
        set_semantics_only()
    else:
        main()


