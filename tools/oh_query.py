import json
import urllib.request
import base64
from urllib.parse import quote

AUTH = "Basic " + base64.b64encode(b"admin:admin").decode()
BASE = "http://192.168.0.51:8080/rest"

def get(path):
    req = urllib.request.Request(BASE + path)
    req.add_header("Authorization", AUTH)
    with urllib.request.urlopen(req) as r:
        return json.load(r)

for uid in ["mqtt:broker:9d15be98b2", "mqtt:topic:9d15be98b2:teplicaesp"]:
    t = get("/things/" + quote(uid, safe=""))
    print("===", uid, "===")
    print("label:", t.get("label"))
    print("config:", json.dumps(t.get("configuration", {}), indent=2, ensure_ascii=False))
    for ch in t.get("channels", []):
        linked = ch.get("linkedItems", [])
        print(" channel:", ch["id"], ch.get("itemType"), ch.get("label"), ch.get("configuration", {}), "->", linked)
