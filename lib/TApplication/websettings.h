#pragma once
#include "Objects.h"
#include <WebServer.h>
#include <Preferences.h>
#include <vector>
#include <map>

class ConfigWebServer:public TControl {
public:
    struct FieldDefinition {
        String name;
        String type;
        bool isSection;
        String sectionLabel;
        
        FieldDefinition(String n, String t, bool isSec, String lbl) 
            : name(n), type(t), isSection(isSec), sectionLabel(lbl) {}
    };

private:
    WebServer server;
    Preferences preferences;
    
    std::map<String, String> fieldValues;

    String htmlHeader = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <meta http-equiv="Content-Type" content="text/html; charset=utf-8" />  
    <meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no">
    <style>
        * {
            box-sizing: border-box;
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
        }

        body {
            margin: 0;
            padding: 10px;
            background-color: #f0f2f5;
        }

        .container {
            max-width: 600px;
            margin: 0 auto;
            background: white;
            padding: 10px;
            border-radius: 12px;
            box-shadow: 0 2px 10px rgba(0,0,0,0.1);
        }

        h1 {
            color: #1a73e8;
            text-align: center;
            margin-bottom: 30px;
        }

        .form-group {
            margin-bottom: 10px;
        }

        label {
            display: block;
            margin-bottom: 8px;
            color: #5f6368;
            font-weight: 500;
        }

        input {
            width: 100%;
            padding: 12px;
            border: 1px solid #dadce0;
            border-radius: 6px;
            font-size: 16px;
            transition: border-color 0.3s;
            margin-bottom: 15px;
        }

        input:focus {
            border-color: #1a73e8;
            outline: none;
        }

        button {
            width: 100%;
            background: #1a73e8;
            color: white;
            padding: 14px;
            border: none;
            border-radius: 6px;
            font-size: 16px;
            cursor: pointer;
            transition: background 0.3s;
        }

        .divider {
            margin: 5px 0;
            text-align: center;
            position: relative;
        }

        .divider hr {
            border: 0;
            height: 1px;
            background: #ddd;
            margin: 10px 0;
        }

        .section-title {
            display: inline-block;
            background: white;
            padding: 0 10px;
            color: #1a73e8;
            font-weight: bold;
            position: relative;
            top: -15px;
            font-size: 1.1em;
        }

        @media (max-width: 480px) {
            body {
                padding: 10px;
            }
            
            .container {
                padding: 10px;
            }
            
            .section-title {
                font-size: 0.9em;
                top: -12px;
            }
            
            input {
                padding: 10px;
                font-size: 14px;
            }
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>Теплица</h1>
        <form method="POST">
)rawliteral";

    String htmlFooter = R"rawliteral(
        </form>
    </div>
</body>
</html>
)rawliteral";

    String escapeHTML(const String& str) {
        String escaped = str;
        escaped.replace("&", "&amp;");
        escaped.replace("\"", "&quot;");
        escaped.replace("'", "&#39;");
        escaped.replace("<", "&lt;");
        escaped.replace(">", "&gt;");
        return escaped;
    }

    void generateForm(String& html) {
        for (const auto& field : fields) {
            if (field.isSection) {
                html += "<div class='divider'><hr><span class='section-title'>";
                html += escapeHTML(field.sectionLabel);
                html += "</span></div>";
            } else {
                html += "<label>" + escapeHTML(field.sectionLabel) + "</label>";
                html += "<input type='" + field.type + "'";
                html += " name='" + escapeHTML(field.name) + "'";
                html += " value='" + escapeHTML(fieldValues[field.name]) + "'>";
            }
        }
        html += "<button type='submit'>Save Settings</button>";
    }

    void handleRoot() {
        if (server.method() == HTTP_POST) {
            for (const auto& field : fields) {
                if (!field.isSection) {
                    String value = server.arg(field.name);
                    fieldValues[field.name] = value;
                    preferences.putString(field.name.c_str(), value);
                }
            }
            preferences.end();
            preferences.begin("config", false);
        }

        String html = htmlHeader;
        generateForm(html);
        html += "<label>TEST</label>";
        html += htmlFooter;
        server.send(200, "text/html", "TEST");
    }

public:
    String AdvancedHTML="";
    std::vector<FieldDefinition> fields;
    ConfigWebServer(int port) {
        for (const auto& field : fields) {
            if (!field.isSection) {
                fieldValues[field.name] = "";
            }
        }
        preferences.begin("config", false);
        for (const auto& field : fields) {
            if (!field.isSection) {
                fieldValues[field.name] = preferences.getString(field.name.c_str(), "");
            }
        }

        server.on("/", HTTP_ANY, [this]() { handleRoot(); });
        server.begin(port);
    }


    void Idle() {
        server.handleClient();
    }

    String getValue(const String& fieldName) {
        return fieldValues[fieldName];
    }
};