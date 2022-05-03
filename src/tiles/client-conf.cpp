#include "tiles/client-conf.h"

#include "config.h"
#include "os/os.h"
#include "util/compiler.h"
#include "util/json.h"
#include "util/string.h"

LogVerbosity confVerbosity = VERBOSE;
MoveMode confMoveMode;
ivec2 confWindowSize = {640, 480};
bool confFullscreen = false;

// Parse and process the client config file, and set configuration defaults for
// missing options.
void
confParse(StringView filename) noexcept {
    String file;

    bool ok = readFile(filename, file);
    if (!ok) {
        logErr(filename, String() << "Could not find " << filename);
        return;
    }

    JsonDocument doc = JsonDocument(static_cast<String&&>(file));
    if (!doc.ok) {
        logErr(filename, String() << "Could not parse " << filename);
        return;
    }

    JsonValue root = doc.root;

    JsonValue engineValue = root["engine"];
    if (engineValue.isObject()) {
        JsonValue verbosityValue = engineValue["verbosity"];
        if (verbosityValue.isString()) {
            StringView verbosity = verbosityValue.toString();
            if (verbosity == "quiet") {
                confVerbosity = QUIET;
            }
            else if (verbosity == "normal") {
                confVerbosity = NORMAL;
            }
            else if (verbosity == "verbose") {
                confVerbosity = VERBOSE;
            }
            else {
                logErr(filename,
                       "Unknown value for \"engine.verbosity\", using default");
            }
        }
    }

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
