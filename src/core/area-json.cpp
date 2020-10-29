/***************************************
** Tsunagari Tile Engine              **
** area-json.cpp                      **
** Copyright 2011-2013 Michael Reiley **
** Copyright 2011-2020 Paul Merrill   **
***************************************/

// **********
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
// **********

#include "core/area-json.h"

#include "core/area.h"
#include "core/character.h"
#include "core/entity.h"
#include "core/images.h"
#include "core/jsons.h"
#include "core/log.h"
#include "core/measure.h"
#include "core/overlay.h"
#include "core/resources.h"
#include "core/tile.h"
#include "core/window.h"
#include "core/world.h"
#include "data/data-world.h"
#include "os/c.h"
#include "util/assert.h"
#include "util/int.h"
#include "util/math2.h"
#include "util/move.h"
#include "util/optional.h"
#include "util/string2.h"
#include "util/unique.h"
#include "util/vector.h"

#define CHECK(x)      \
    if (!(x)) {       \
        return false; \
    }

/* NOTE: In the JSON map format used by Tiled, tileset tiles start counting
         their Y-positions from 0, while layer tiles start counting from 1. I
         can't imagine why the author did this, but we have to take it into
         account.
*/

class AreaJSON : public Area {
 public:
    AreaJSON(Player* player, StringView filename) noexcept;

 private:
    //! Allocate Tile objects for one layer of map.
    void
    allocateMapLayer(TileGrid::LayerType type) noexcept;

    //! Parse an Area file.
    bool
    processDescriptor() noexcept;
    bool
    processMapProperties(JsonValue obj) noexcept;
    bool
    processTileSet(JsonValue obj) noexcept;
    bool
    processTileSetFile(JsonValue obj, StringView source, int firstGid) noexcept;
    bool
    processTileType(JsonValue obj,
                    Animation& graphic,
                    TiledImage img,
                    int id) noexcept;
    bool
    processLayer(JsonValue obj) noexcept;
    bool
    processLayerProperties(JsonValue obj) noexcept;
    bool
    processLayerData(JsonValue arr) noexcept;
    bool
    processObjectGroup(JsonValue obj) noexcept;
    bool
    processObjectGroupProperties(JsonValue obj) noexcept;
    bool
    processObject(JsonValue obj) noexcept;
    bool
    splitTileFlags(StringView strOfFlags, unsigned* flags) noexcept;
    bool
    parseExit(StringView dest,
              Optional<Exit>& exit,
              bool* wwide,
              bool* hwide) noexcept;
    bool
    parseARGB(StringView str,
              unsigned char& a,
              unsigned char& r,
              unsigned char& g,
              unsigned char& b) noexcept;
};


Area*
makeAreaFromJSON(Player* player, StringView filename) noexcept {
    return new AreaJSON(player, filename);
}


AreaJSON::AreaJSON(Player* player, StringView descriptor) noexcept {
    TimeMeasure m(String() << "Constructed " << descriptor << " as area-json");

    dataArea = DataWorld::area(descriptor);
    this->player = player;
    this->descriptor = descriptor;

    // Add TileType #0. Not used, but Tiled's gids start from 1.
    tileGraphics.resize(1);

    ok = processDescriptor();
}

void
AreaJSON::allocateMapLayer(TileGrid::LayerType type) noexcept {
    ivec3 dim = grid.dim;

    // FIXME: Better int overflow check that multiplies x,y,z together.
    assert_(0 <= dim.y);
    assert_(0 <= dim.x);
    assert_(0 <= dim.z);

    grid.layerTypes.push_back(type);

    grid.graphics.resize(grid.graphics.size + dim.x * dim.y);
    grid.dim.z++;
}

bool
AreaJSON::processDescriptor() noexcept {
    Optional<JsonDocument> doc = loadJson(descriptor);
    CHECK(doc);

    JsonValue root = doc->root;

    JsonValue widthValue = root["width"];
    JsonValue heightValue = root["height"];
    JsonValue propertiesValue = root["properties"];
    JsonValue tilesetsValue = root["tilesets"];
    JsonValue layersValue = root["layers"];

    CHECK(widthValue.isNumber());
    CHECK(heightValue.isNumber());
    CHECK(propertiesValue.isObject());
    CHECK(tilesetsValue.isArray());
    CHECK(layersValue.isArray());

    grid.dim.x = widthValue.toInt();
    grid.dim.y = heightValue.toInt();
    grid.dim.z = 0;

    CHECK(processMapProperties(propertiesValue));

    CHECK(tilesetsValue.toNode());

    for (JsonNode& tilesetNode : tilesetsValue) {
        JsonValue tilesetValue = tilesetNode.value;
        CHECK(tilesetValue.isObject());

        CHECK(processTileSet(tilesetValue));
    }

    CHECK(layersValue.toNode());

    for (JsonNode& layerNode : layersValue) {
        JsonValue layerValue = layerNode.value;
        CHECK(layerValue.isObject());

        JsonValue typeValue = layerValue["type"];
        CHECK(typeValue.isString());

        StringView type = typeValue.toString();

        if (type == "tilelayer") {
            CHECK(processLayer(layerValue));
        }
        else if (type == "objectgroup") {
            CHECK(processObjectGroup(layerValue));
        }
        else {
            logErr(descriptor, "Each layer must be a tilelayer or objectlayer");
            return false;
        }
    }

    return true;
}

bool
AreaJSON::processMapProperties(JsonValue obj) noexcept {
    /*
     {
       "name": "Wooded Area"
       "music": "wind.oga",
       "loop": "xy",
       "color_overlay": "255,255,255,127"
     }
    */

    JsonValue nameValue = obj["name"];
    JsonValue musicValue = obj["music"];
    JsonValue loopValue = obj["loop"];
    JsonValue coloroverlayValue = obj["coloroverlay"];

    CHECK(musicValue.isString() || musicValue.isNull());
    CHECK(loopValue.isString() || loopValue.isNull());
    CHECK(coloroverlayValue.isString() || coloroverlayValue.isNull());

    if (!nameValue.isString()) {
        logErr(descriptor, "Area must have \"name\" property");
    }

    name = nameValue.toString();

    if (musicValue.isString()) {
        musicPath = musicValue.toString();
    }
    if (loopValue.isString()) {
        StringView directions = loopValue.toString();
        grid.loopX = directions.find('x') != SV_NOT_FOUND;
        grid.loopY = directions.find('y') != SV_NOT_FOUND;
    }
    if (coloroverlayValue.isString()) {
        unsigned char a, r, g, b;
        CHECK(parseARGB(coloroverlayValue.toString(), a, r, g, b));
        colorOverlayARGB = (uint32_t)(a << 24) + (uint32_t)(r << 16) +
                           (uint32_t)(g << 8) + (uint32_t)b;
    }

    return true;
}

/**
 * dirname
 *
 * Returns the directory component of a path, including trailing slash.  If
 * there is no directory component, return an empty string.
 */
static StringView
dirname(StringView path) noexcept {
    StringPosition slash = path.rfind('/');
    return slash == SV_NOT_FOUND
        ? ""
        : path.substr(0, slash + 1);
}

bool
AreaJSON::processTileSet(JsonValue obj) noexcept {
    /*
     {
       "firstgid": 1,
       "source": "tiles\/forest.png.json"
     }
    */

    JsonValue firstgidValue = obj["firstgid"];
    JsonValue sourceValue = obj["source"];

    CHECK(firstgidValue.isNumber());
    CHECK(sourceValue.isString());

    const unsigned firstGid = firstgidValue.toInt();

    String source = String() << dirname(descriptor) << sourceValue.toString();

    // We don't handle embeded tilesets, only references to an external JSON
    // files.
    Optional<JsonDocument> doc = loadJson(source);
    if (!doc) {
        logErr(descriptor, String() << source << ": failed to load JSON file");
        return false;
    }

    if (!processTileSetFile(doc->root, source, firstGid)) {
        logErr(descriptor,
               String() << source << ": failed to parse JSON tileset file");
        return false;
    }

    return true;
}

bool
AreaJSON::processTileSetFile(JsonValue obj,
                             StringView source,
                             int firstGid) noexcept {
    /*
     {
       "image": "forest.png",
       "imageheight": 304,
       "imagewidth": 272,
       "name": "forest.png",
       "tilecount": 323,
       "tileheight": 16,
       "tileproperties": {
         "29": {
           "frames": "29,58",
           "speed": "0.75"
         }
       },
       "tilewidth": 16
     }
    */

    assert_(firstGid == tileGraphics.size);

    unsigned tilex, tiley;
    unsigned pixelw, pixelh;
    unsigned width, height;

    JsonValue imageNode = obj["image"];
    JsonValue imagewidthNode = obj["imagewidth"];
    JsonValue imageheightNode = obj["imageheight"];
    JsonValue tilewidthNode = obj["tilewidth"];
    JsonValue tileheightNode = obj["tileheight"];
    JsonValue tilespropertiesNode = obj["tileproperties"];

    CHECK(imageNode.isString());
    CHECK(imagewidthNode.isNumber());
    CHECK(imageheightNode.isNumber());
    CHECK(tilewidthNode.isNumber());
    CHECK(tileheightNode.isNumber());
    CHECK(tilespropertiesNode.isObject() || tilespropertiesNode.isNull());

    tilex = tilewidthNode.toInt();
    tiley = tileheightNode.toInt();

    CHECK(tilex > 0 && tiley > 0);
    CHECK(tilex <= 0x7FFF && tiley <= 0x7FFF);  // Reasonable limit?

    if (grid.tileDim && grid.tileDim.x != tilex && grid.tileDim.y != tiley) {
        logErr(descriptor, "Tileset's width/height contradict earlier <layer>");
        return false;
    }
    grid.tileDim = ivec2{static_cast<int>(tilex), static_cast<int>(tiley)};

    pixelw = imagewidthNode.toInt();
    pixelh = imageheightNode.toInt();

    width = pixelw / grid.tileDim.x;
    height = pixelh / grid.tileDim.y;

    String imgSource = String() << dirname(source) << imageNode.toString();
    tileSets[imgSource] = TileSet{firstGid, (size_t)width, (size_t)height};

    // Load tileset image.
    TiledImage images = tilesLoad(imgSource, tilex, tiley);
    if (!TILES_VALID(images)) {
        logErr(descriptor, "Tileset image not found");
        return false;
    }

    int nTiles = images.numTiles;
    tileGraphics.reserve(tileGraphics.size + nTiles);

    // Initialize "vanilla" tile type array.
    for (int i = 0; i < nTiles; i++) {
        Image image = tileAt(images, i);
        tileGraphics.push_back(Animation(image));
    }

    if (!tilespropertiesNode.isObject()) {
        return true;
    }

    // Handle explicitly declared "non-vanilla" types.

    for (JsonNode& tilepropertiesNode : tilespropertiesNode) {
        CHECK(tilepropertiesNode.value.isObject());

        // "id" is 0-based index of a tile in the current
        // tileset, if the tileset were a flat array.
        Optional<unsigned> id_ = parseUInt(tilepropertiesNode.key);
        if (!id_) {
            logErr(descriptor, "Tile type id is invalid");
            return false;
        }
        if (*id_ > INT32_MAX) {
            logErr(descriptor, "Tile type id is invalid");
            return false;
        }
        int id__ = static_cast<int>(*id_);
        if (nTiles <= id__) {
            logErr(descriptor, "Tile type id is invalid");
            return false;
        }

        // "gid" is the global area-wide id of the tile.
        int gid = id__ + firstGid;

        Animation& graphic = tileGraphics[gid];
        if (!processTileType(tilepropertiesNode.value,
                             graphic,
                             images,
                             static_cast<int>(id__))) {
            return false;
        }
    }

    return true;
}

bool
AreaJSON::processTileType(JsonValue obj,
                          Animation& graphic,
                          TiledImage images,
                          int id) noexcept {
    /*
      {
        "frames": "29,58",
        "speed": "0.75"
      }
    */

    // The id has already been handled by processTileSet, so we don't have
    // to worry about it.

    JsonValue framesNode = obj["frames"];
    JsonValue speedNode = obj["speed"];

    CHECK(framesNode.isString() || framesNode.isNull());
    CHECK(speedNode.isString() || speedNode.isNull());

    // If a Tile is animated, it needs both member frames and a speed.
    Vector<Image> framesvec;
    Optional<int> frameLen;

    int nTiles = images.numTiles;

    if (framesNode.isString()) {
        Vector<StringView> frames = splitStr(framesNode.toString(), ",");

        // Make sure the first member is this tile.
        Optional<int> firstFrame = parseInt(frames[0]);
        if (!firstFrame || *firstFrame != id) {
            logErr(descriptor,
                   String() << "first member of tile id " << id
                            << " animation must be itself.");
            return false;
        }

        // Add frames to our animation.
        // We already have one from TileType's constructor.
        for (StringView& frame : frames) {
            Optional<unsigned> idx_ = parseUInt(frame);
            if (!idx_) {
                logErr(descriptor,
                       "couldn't parse frame index for animated tile");
                return false;
            }
            if (*idx_ > INT32_MAX) {
                logErr(descriptor, "frame index out of bounds");
                return false;
            }

            int idx = static_cast<int>(*idx_);

            if (nTiles <= idx) {
                logErr(descriptor,
                       "frame index out of range for animated tile");
                return false;
            }

            framesvec.push_back(tileAt(images, idx));
        }
    }
    if (speedNode.isString()) {
        Optional<float> hertz = parseFloat(speedNode.toString());
        CHECK(hertz);
        frameLen = (int)(1000.0 / *hertz);
    }

    if (framesvec.size || frameLen) {
        if (framesvec.size == 0 || !frameLen) {
            logErr(descriptor,
                   "Tile type must either have both frames and speed or none");
            return false;
        }
        // Add 'now' to Animation constructor??
        time_t now = World::time();
        graphic = Animation(move_(framesvec), *frameLen);
        graphic.restart(now);
    }

    return true;
}

bool
AreaJSON::processLayer(JsonValue obj) noexcept {
    /*
     {
       "data": [9, 9, 9, ..., 3, 9, 9],
       "height": 33,
       "properties": {
         ...
       },
       "width": 34,
     }
    */

    JsonValue widthValue = obj["width"];
    JsonValue heightValue = obj["height"];
    JsonValue propertiesValue = obj["properties"];
    JsonValue dataValue = obj["data"];

    CHECK(widthValue.isNumber());
    CHECK(heightValue.isNumber());
    CHECK(propertiesValue.isObject());
    CHECK(dataValue.isArray());

    const int x = widthValue.toInt();
    const int y = heightValue.toInt();

    if (grid.dim.x != x || grid.dim.y != y) {
        logErr(descriptor, "layer x,y size != map x,y size");
        return false;
    }

    allocateMapLayer(TileGrid::LayerType::TILE_LAYER);

    CHECK(processLayerProperties(propertiesValue));
    CHECK(processLayerData(dataValue));

    return true;
}

bool
AreaJSON::processLayerProperties(JsonValue obj) noexcept {
    /*
     {
       "depth": "-0.5"
     }
    */

    JsonValue depthValue = obj["depth"];

    if (!depthValue.isString() || !parseFloat(depthValue.toString())) {
        logErr(descriptor, "A tilelayer must have the \"depth\" property");
        return false;
    }

    const float depth = *parseFloat(depthValue.toString());

    if (grid.depth2idx.find(depth) != grid.depth2idx.end()) {
        logErr(descriptor, "Layers cannot share a depth");
        return false;
    }

    grid.depth2idx[depth] = grid.dim.z - 1;
    grid.idx2depth.push_back(
            depth);  // Effectively idx2depth[dim.z - 1] = depth;

    return true;
}

bool
AreaJSON::processLayerData(JsonValue arr) noexcept {
    /*
     [9, 9, 9, ..., 3, 9, 9]
    */

    const size_t z = static_cast<size_t>(grid.dim.z) - 1;

    // If we ever allow finding layers out of order.
    // assert_(0 <= z && z < dim.z);

    size_t x = 0, y = 0;

    for (JsonNode& node : arr) {
        CHECK(node.value.isNumber());

        unsigned gid = node.value.toInt();

        if (gid >= tileGraphics.size) {
            logErr(descriptor, "Invalid tile gid");
            return false;
        }

        size_t idx = (z * grid.dim.y + y) * grid.dim.x + x;

        // A gid of zero means there is no tile at this
        // position on this layer.
        grid.graphics[idx] = gid;

        if (++x == static_cast<size_t>(grid.dim.x)) {
            x = 0;
            y++;
        }
    }

    return true;
}

bool
AreaJSON::processObjectGroup(JsonValue obj) noexcept {
    /*
     {
       "name": "Prop(1)",
       "objects": [...],
       "properties": {...}
     }
    */

    JsonValue propertiesValue = obj["properties"];
    JsonValue objectsValue = obj["objects"];

    CHECK(propertiesValue.isObject());
    CHECK(objectsValue.isArray());

    CHECK(processObjectGroupProperties(propertiesValue));

    for (JsonNode& objectNode : objectsValue) {
        CHECK(objectNode.value.isObject());
        CHECK(processObject(objectNode.value));
    }

    return true;
}

bool
AreaJSON::processObjectGroupProperties(JsonValue obj) noexcept {
    /*
     {
       "depth": "0.0"
     }
    */

    JsonValue depthValue = obj["depth"];

    if (!depthValue.isString() || !parseFloat(depthValue.toString())) {
        logErr(descriptor, "An objectlayer must have the \"depth\" property");
        return false;
    }

    const float depth = *parseFloat(depthValue.toString());

    if (grid.depth2idx.contains(depth)) {
        logErr(descriptor, "Layers cannot share a depth");
        return false;
    }

    allocateMapLayer(TileGrid::LayerType::OBJECT_LAYER);
    grid.depth2idx[depth] = grid.dim.z - 1;
    grid.idx2depth.push_back(
            depth);  // Effectively idx2depth[dim.z - 1] = depth;

    return true;
}

bool
AreaJSON::processObject(JsonValue obj) noexcept {
    /*
     {
       "height": 16,
       "properties": {
         "onEnter": "half_speed"
         "onLeave": "normal_speed"
         "onuse": "normal_speed"
         "exit": "grassfield.area,1,1,0"
         "flags": "npc_nowalk"
       },
       "width": 16,
       "x": 256,
       "y": 272
     }
    */

    JsonValue propertiesValue = obj["properties"];
    if (!propertiesValue.isObject()) {
        // Empty tile object. Odd, but acceptable.
        return true;
    }

    JsonValue xValue = obj["x"];
    JsonValue yValue = obj["y"];
    JsonValue widthValue = obj["width"];
    JsonValue heightValue = obj["height"];

    CHECK(xValue.isNumber());
    CHECK(yValue.isNumber());
    CHECK(widthValue.isNumber());
    CHECK(heightValue.isNumber());

    JsonValue flagsValue = propertiesValue["flags"];
    JsonValue onenterValue = propertiesValue["on_enter"];
    JsonValue onleaveValue = propertiesValue["on_leave"];
    JsonValue onuseValue = propertiesValue["on_use"];
    JsonValue exitValue = propertiesValue["exit"];
    JsonValue exitupValue = propertiesValue["exit:up"];
    JsonValue exitdownValue = propertiesValue["exit:down"];
    JsonValue exitleftValue = propertiesValue["exit:left"];
    JsonValue exitrightValue = propertiesValue["exit:right"];
    JsonValue layermodValue = propertiesValue["layermod"];
    JsonValue layermodupValue = propertiesValue["layermod:up"];
    JsonValue layermoddownValue = propertiesValue["layermod:down"];
    JsonValue layermodleftValue = propertiesValue["layermod:left"];
    JsonValue layermodrightValue = propertiesValue["layermod:right"];

    CHECK(flagsValue.isString() || flagsValue.isNull());
    CHECK(onenterValue.isString() || onenterValue.isNull());
    CHECK(onleaveValue.isString() || onleaveValue.isNull());
    CHECK(onuseValue.isString() || onuseValue.isNull());
    CHECK(exitValue.isString() || exitValue.isNull());
    CHECK(exitupValue.isString() || exitupValue.isNull());
    CHECK(exitdownValue.isString() || exitdownValue.isNull());
    CHECK(exitleftValue.isString() || exitleftValue.isNull());
    CHECK(exitrightValue.isString() || exitrightValue.isNull());
    CHECK(layermodValue.isNull() ||
          (layermodValue.isString() && parseFloat(layermodValue.toString())));
    CHECK(layermodValue.isNull() || (layermodupValue.isString() &&
                                     parseFloat(layermodupValue.toString())));
    CHECK(layermodValue.isNull() || (layermoddownValue.isString() &&
                                     parseFloat(layermoddownValue.toString())));
    CHECK(layermodValue.isNull() || (layermodleftValue.isString() &&
                                     parseFloat(layermodleftValue.toString())));
    CHECK(layermodValue.isNull() ||
          (layermodrightValue.isString() &&
           parseFloat(layermodrightValue.toString())));

    const size_t z = static_cast<size_t>(grid.dim.z) - 1;

    // If we ever allow finding layers out of order.
    // assert_(0 <= z && z < dim.z);

    // Gather object properties now. Assign them to tiles later.
    bool wwide[5] = {}, hwide[5] = {};  // Wide exit in width or height.

    DataArea::TileScript enterScript = nullptr, leaveScript = nullptr,
                         useScript = nullptr;
    Optional<Exit> exit[5];
    Optional<float> layermods[5];
    unsigned flags = 0x0;

    if (flagsValue.isString()) {
        CHECK(splitTileFlags(flagsValue.toString(), &flags));
    }

    if (onenterValue.isString()) {
        StringView scriptName = onenterValue.toString();
        enterScript = dataArea->scripts[scriptName];
    }
    if (onleaveValue.isString()) {
        StringView scriptName = onleaveValue.toString();
        leaveScript = dataArea->scripts[scriptName];
    }
    if (onuseValue.isString()) {
        StringView scriptName = onuseValue.toString();
        useScript = dataArea->scripts[scriptName];
    }

    if (exitValue.isString()) {
        StringView value = exitValue.toString();
        CHECK(parseExit(value,
                        exit[EXIT_NORMAL],
                        &wwide[EXIT_NORMAL],
                        &hwide[EXIT_NORMAL]));
        flags |= TILE_NOWALK_NPC;
    }
    if (exitupValue.isString()) {
        StringView value = exitupValue.toString();
        CHECK(parseExit(
                value, exit[EXIT_UP], &wwide[EXIT_UP], &hwide[EXIT_UP]));
    }
    if (exitdownValue.isString()) {
        StringView value = exitdownValue.toString();
        CHECK(parseExit(
                value, exit[EXIT_DOWN], &wwide[EXIT_DOWN], &hwide[EXIT_DOWN]));
    }
    if (exitleftValue.isString()) {
        StringView value = exitleftValue.toString();
        CHECK(parseExit(
                value, exit[EXIT_LEFT], &wwide[EXIT_LEFT], &hwide[EXIT_LEFT]));
    }
    if (exitrightValue.isString()) {
        StringView value = exitrightValue.toString();
        CHECK(parseExit(value,
                        exit[EXIT_RIGHT],
                        &wwide[EXIT_RIGHT],
                        &hwide[EXIT_RIGHT]));
    }

    if (layermodValue.isString()) {
        float mod = parseFloat(layermodValue.toString());
        layermods[EXIT_NORMAL] = mod;
        flags |= TILE_NOWALK_NPC;
    }
    if (layermodupValue.isString()) {
        float mod = parseFloat(layermodupValue.toString());
        layermods[EXIT_UP] = mod;
    }
    if (layermoddownValue.isString()) {
        float mod = parseFloat(layermoddownValue.toString());
        layermods[EXIT_DOWN] = mod;
    }
    if (layermodleftValue.isString()) {
        float mod = parseFloat(layermodleftValue.toString());
        layermods[EXIT_LEFT] = mod;
    }
    if (layermodrightValue.isString()) {
        float mod = parseFloat(layermodrightValue.toString());
        layermods[EXIT_RIGHT] = mod;
    }

    // Apply these properties directly to one or more tiles in a rectangle
    // of the map. We don't keep an intermediary "object" object lying
    // around.

    const int x = xValue.toInt() / grid.tileDim.x;
    const int y = yValue.toInt() / grid.tileDim.y;
    const int w = widthValue.toInt() / grid.tileDim.x;
    const int h = heightValue.toInt() / grid.tileDim.y;

    CHECK(x + w <= grid.dim.x);
    CHECK(y + h <= grid.dim.y);

    // We know which Tiles are being talked about now... yay
    for (int Y = y; Y < y + h; Y++) {
        for (int X = x; X < x + w; X++) {
            icoord tile = {X, Y, static_cast<int>(z)};

            grid.flags[tile] |= flags;
            for (size_t i = 0; i < EXITS_LENGTH; i++) {
                if (exit[i]) {
                    int dx = X - x;
                    int dy = Y - y;
                    if (wwide[i]) {
                        exit[i]->coords.x += dx;
                    }
                    if (hwide[i]) {
                        exit[i]->coords.y += dy;
                    }
                    grid.exits[i][tile] = move_(*exit[i]);
                }
            }
            for (size_t i = 0; i < EXITS_LENGTH; i++) {
                if (layermods[i]) {
                    grid.layermods[i][tile] = *layermods[i];
                }
            }

            if (enterScript) {
                grid.scripts[TileGrid::SCRIPT_TYPE_ENTER][tile] = enterScript;
            }
            if (leaveScript) {
                grid.scripts[TileGrid::SCRIPT_TYPE_LEAVE][tile] = leaveScript;
            }
            if (useScript) {
                grid.scripts[TileGrid::SCRIPT_TYPE_USE][tile] = useScript;
            }
        }
    }

    return true;
}

bool
AreaJSON::splitTileFlags(StringView strOfFlags, unsigned* flags) noexcept {
    for (auto str : splitStr(strOfFlags, ",")) {
        if (str == "nowalk") {
            *flags |= TILE_NOWALK;
        }
        else if (str == "nowalk_player") {
            *flags |= TILE_NOWALK_PLAYER;
        }
        else if (str == "nowalk_npc") {
            *flags |= TILE_NOWALK_NPC;
        }
        else {
            logErr(descriptor, String() << "Invalid tile flag: " << str);
            return false;
        }
    }

    return true;
}

/**
 * Matches regex /^\s*\d+\+?$/
 */
static bool
isIntegerOrPlus(StringView s) noexcept {
    const int space = 0;
    const int digit = 1;
    const int sign = 2;

    int state = space;

    for (char c : s) {
        if (state == space) {
            if (c == ' ') {
                continue;
            }
            else {
                state++;
            }
        }
        if (state == digit) {
            if ('0' <= c && c <= '9') {
                continue;
            }
            else {
                state++;
            }
        }
        if (state == sign) {
            return c == '+';
        }
    }
    return true;
}

bool
AreaJSON::parseExit(StringView dest,
                    Optional<Exit>& exit,
                    bool* wwide,
                    bool* hwide) noexcept {
    /*
      Format: destination area, x, y, z
      E.g.:   "babysfirst.area,1,3,0"
    */

    Vector<StringView> strs = splitStr(dest, ",");

    if (strs.size != 4) {
        logErr(descriptor, "exit: Invalid format");
        return false;
    }

    StringView area = strs[0];
    StringView x = strs[1];
    StringView y = strs[2];
    StringView z = strs[3];

    if (!isIntegerOrPlus(x) || !isIntegerOrPlus(y) || !isIntegerOrPlus(z)) {
        logErr(descriptor, "exit: Invalid format");
        return false;
    }

    if (x.find('+') != SV_NOT_FOUND) {
        x = x.substr(0, x.size - 1);
    }
    if (y.find('+') != SV_NOT_FOUND) {
        y = y.substr(0, y.size - 1);
    }

    Optional<int> x_ = parseInt(x);
    Optional<int> y_ = parseInt(y);
    Optional<float> z_ = parseFloat(z);

    exit = Exit{area, *x_, *y_, *z_};

    *wwide = x.find('+') != SV_NOT_FOUND;
    *hwide = y.find('+') != SV_NOT_FOUND;

    return true;
}

bool
AreaJSON::parseARGB(StringView str,
                    unsigned char& a,
                    unsigned char& r,
                    unsigned char& g,
                    unsigned char& b) noexcept {
    unsigned char* channels[] = {&a, &r, &g, &b};

    Vector<StringView> strs = splitStr(str, ",");

    if (strs.size != 4) {
        logErr(descriptor, "invalid ARGB format");
        return false;
    }

    for (size_t i = 0; i < 4; i++) {
        Optional<int> v = parseInt(strs[i]);
        if (!v) {
            logErr(descriptor, "invalid ARGB format");
            return false;
        }
        int v_ = *v;
        if (!(0 <= v_ && v_ < 256)) {
            logErr(descriptor, "ARGB values must be between 0 and 255");
            return false;
        }
        *channels[i] = (unsigned char)v_;
    }

    return true;
}
