#include "tiles/client-conf.h"

#include "os/os.h"
#include "util/compiler.h"
#include "util/json.h"
#include "util/string.h"

extern fvec2 dataWorldViewportResolution;

MoveMode confMoveMode;
ivec2 confWindowSize;
bool confFullscreen;

// Parse and process the client config file, and set configuration defaults for
// missing options.
void
confParse(StringView filename) noexcept {
    String file;

    bool ok = readFile(filename, file);
    if (!ok) {
        logInfo("ClientConf",
                String() << "Missing " << filename << ", using defaults");
        confWindowSize.x = static_cast<I32>(dataWorldViewportResolution.x);
        confWindowSize.y = static_cast<I32>(dataWorldViewportResolution.y);
        confFullscreen = false;
        return;
    }

    JsonDocument doc = JsonDocument(static_cast<String&&>(file));
    if (!doc.ok) {
        logErr("ClientConf", String() << "Could not parse " << filename);
        return;
    }

    JsonValue root = doc.root;

    JsonValue windowValue = root["window"];
    if (windowValue.isObject()) {
        JsonValue widthValue = windowValue["width"];
        if (widthValue.isNumber()) {
            confWindowSize.x = widthValue.toInt();
        }
        JsonValue heightValue = windowValue["height"];
        if (heightValue.isNumber()) {
            confWindowSize.y = heightValue.toInt();
        }
        JsonValue fullscreenValue = windowValue["fullscreen"];
        if (fullscreenValue.isBool()) {
            confFullscreen = fullscreenValue.toBool();
        }
    }
}
