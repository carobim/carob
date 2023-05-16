#include "tiles/area-json.h"

#include "data/data-world.h"
#include "tiles/area.h"
#include "tiles/character.h"
#include "tiles/entity.h"
#include "tiles/images.h"
#include "tiles/jsons.h"
#include "tiles/log.h"
#include "tiles/overlay.h"
#include "tiles/resources.h"
#include "tiles/tile.h"
#include "tiles/world.h"
#include "util/assert.h"
#include "util/compiler.h"
#include "util/int.h"
#include "util/measure.h"
#include "util/string2.h"
#include "util/vector.h"

#define CHECK(x)          \
    do {                  \
        if (!(x)) {       \
            return false; \
        }                 \
    } while (false)

/* NOTE: In the JSON map format used by Tiled, tileset tiles start counting
         their Y-positions from 0, while layer tiles start counting from 1. I
         can't imagine why the author did this, but we have to take it into
         account.
*/

class AreaJSON : public Area {
 public:
    AreaJSON(Player* player, StringView descriptor) noexcept;

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
    processTileSetFile(JsonValue obj, StringView source, U32 firstGid) noexcept;
    bool
    processTileType(JsonValue obj, Animation& graphic, TiledImage images,
                    U32 id) noexcept;
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
    splitTileFlags(StringView strOfFlags, U32* flags) noexcept;
    bool
    parseExit(StringView dest, Exit& exit, bool* wwide, bool* hwide) noexcept;
    bool
    parseARGB(StringView str, U8& a, U8& r, U8& g, U8& b) noexcept;
};

static void
preallocateMapLayers(TileGrid& grid, Size n) noexcept {
    ivec3 dim = grid.dim;
    Size layerSize = dim.x * dim.y;
    grid.graphics.reserve(layerSize * n);
}

Area*
makeAreaFromJSON(Player* player, StringView filename) noexcept {
    return new AreaJSON(player, filename);
}


AreaJSON::AreaJSON(Player* player, StringView descriptor) noexcept {
    TimeMeasure m(String() << "Constructed " << descriptor << " as area-json");

    dataArea = dataWorldArea(descriptor);
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

    grid.layerTypes.push(type);

    grid.graphics.resize(grid.graphics.size + dim.x * dim.y);
    grid.dim.z++;
}

bool
AreaJSON::processDescriptor() noexcept {
    JsonDocument doc = loadJson(descriptor);
    CHECK(doc.ok);

    JsonValue root = doc.root;

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

    for (JsonIterator tilesetNode = begin(tilesetsValue);
         tilesetNode != end(tilesetsValue); ++tilesetNode) {
        JsonValue tilesetValue = tilesetNode->value;
        CHECK(tilesetValue.isObject());

        CHECK(processTileSet(tilesetValue));
    }

    CHECK(layersValue.toNode());

    Size numLayers = 0;
    for (JsonIterator layerNode = begin(layersValue);
         layerNode != end(layersValue); ++layerNode) {
        JsonValue layerValue = layerNode->value;
        CHECK(layerValue.isObject());

        JsonValue typeValue = layerValue["type"];
        CHECK(typeValue.isString());

        StringView type = typeValue.toString();
        if (type == "tilelayer" || type == "objectgroup") {
            numLayers++;
        }
    }

    preallocateMapLayers(grid, numLayers);

    for (JsonIterator layerNode = begin(layersValue);
         layerNode != end(layersValue); ++layerNode) {
        JsonValue layerValue = layerNode->value;
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
        if (musicPath == "") {
            musicPath << StringView("\0", 1);
        }
    }
    if (loopValue.isString()) {
        StringView directions = loopValue.toString();
        grid.loopX = directions.find('x') != SV_NOT_FOUND;
        grid.loopY = directions.find('y') != SV_NOT_FOUND;
    }
    if (coloroverlayValue.isString()) {
        U8 a, r, g, b;
        CHECK(parseARGB(coloroverlayValue.toString(), a, r, g, b));
        colorOverlayARGB =
            (U32)(a << 24) + (U32)(r << 16) + (U32)(g << 8) + (U32)b;
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
    return slash == SV_NOT_FOUND ? "" : path.substr(0, slash + 1);
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

    const U32 firstGid = firstgidValue.toInt();

    String source = String() << dirname(descriptor) << sourceValue.toString();

    // We don't handle embedded tilesets, only references to an external JSON
    // files.
    JsonDocument doc = loadJson(source);
    if (!doc.ok) {
        logErr(descriptor, String() << source << ": failed to load JSON file");
        return false;
    }

    if (!processTileSetFile(doc.root, source, firstGid)) {
        logErr(descriptor,
               String() << source << ": failed to parse JSON tileset file");
        return false;
    }

    return true;
}

bool
AreaJSON::processTileSetFile(JsonValue obj, StringView source,
                             U32 firstGid) noexcept {
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

    U32 tileWidth, tileHeight;
    U32 pixelWidth, pixelHeight;
    U32 numAcross, numHigh;

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

    tileWidth = tilewidthNode.toInt();
    tileHeight = tileheightNode.toInt();

    pixelWidth = imagewidthNode.toInt();
    pixelHeight = imageheightNode.toInt();

    numAcross = pixelWidth / tileWidth;
    numHigh = pixelHeight / tileHeight;

    CHECK(tileWidth > 0 && tileHeight > 0);
    CHECK(tileWidth <= 0x7FFF && tileHeight <= 0x7FFF);  // Reasonable limit?

    if ((grid.tileDim.x || grid.tileDim.y) &&
        static_cast<U32>(grid.tileDim.x) != tileWidth &&
        static_cast<U32>(grid.tileDim.y) != tileHeight) {
        logErr(descriptor, "Tileset's width/height contradict earlier <layer>");
        return false;
    }

    grid.tileDim.x = static_cast<I32>(tileWidth);
    grid.tileDim.y = static_cast<I32>(tileHeight);

    String imgSource = String() << dirname(source) << imageNode.toString();

    TileSet tileSet = {firstGid, numAcross, numHigh};
    tileSets[imgSource] = tileSet;

    // Load tileset image.
    TiledImage images =
        tilesLoad(imgSource, tileWidth, tileHeight, numAcross, numHigh);
    if (!TILES_VALID(images)) {
        logErr(descriptor, "Tileset image not found");
        return false;
    }

    U32 nTiles = images.numTiles;
    tileGraphics.reserve(tileGraphics.size + nTiles);

    // Initialize "vanilla" tile type array.
    for (U32 i = 0; i < nTiles; i++) {
        Image image = tileAt(images, i);
        tileGraphics.push(Animation(image));
    }

    if (!tilespropertiesNode.isObject()) {
        return true;
    }

    // Handle explicitly declared "non-vanilla" types.

    String buf;
    for (JsonIterator tilepropertiesNode = begin(tilespropertiesNode);
         tilepropertiesNode != end(tilespropertiesNode); ++tilepropertiesNode) {
        CHECK(tilepropertiesNode->value.isObject());

        // "id" is 0-based index of a tile in the current
        // tileset, if the tileset were a flat array.
        buf.clear();
        buf = tilepropertiesNode->key;
        U32 id;
        if (!parseUInt(id, buf)) {
            logErr(descriptor, "Tile type id is invalid");
            return false;
        }
        if (id > static_cast<U32>(INT32_MAX)) {
            logErr(descriptor, "Tile type id is invalid");
            return false;
        }
        if (nTiles <= id) {
            logErr(descriptor, "Tile type id is invalid");
            return false;
        }

        // "gid" is the global area-wide id of the tile.
        U32 gid = id + firstGid;

        Animation& graphic = tileGraphics[gid];
        if (!processTileType(tilepropertiesNode->value, graphic, images, id)) {
            return false;
        }
    }

    return true;
}

bool
AreaJSON::processTileType(JsonValue obj, Animation& graphic, TiledImage images,
                          U32 id) noexcept {
    /*
      {
        "frames": "29,58",
        "speed": 0.75
      }
    */

    // The id has already been handled by processTileSet, so we don't have
    // to worry about it.

    JsonValue framesNode = obj["frames"];
    JsonValue speedNode = obj["speed"];

    CHECK(framesNode.isString() && speedNode.isNumber());

    // If a Tile is animated, it needs both member frames and a speed.
    Vector<Image> framesvec;
    I32 frameLen;

    U32 nTiles = images.numTiles;

    Vector<StringView> frames;
    splitStr(frames, framesNode.toString(), ",");
    CHECK(frames.size);

    String buf;

    // Make sure the first member is this tile.
    buf.clear();
    buf = frames[0];
    U32 firstFrame;
    if (!parseUInt(firstFrame, buf)) {
        logErr(descriptor, "couldn't parse frame index for animated tile");
        return false;
    }
    if (firstFrame != id) {
        logErr(descriptor, String() << "first member of tile id " << id
                                    << " animation must be itself.");
        return false;
    }

    // Add frames to our animation.
    // We already have one from TileType's constructor.
    for (StringView* frame = frames.begin(); frame != frames.end(); frame++) {
        buf.clear();
        buf = *frame;

        U32 idx;
        if (!parseUInt(idx, buf)) {
            logErr(descriptor, "couldn't parse frame index for animated tile");
            return false;
        }
        if (idx > static_cast<U32>(INT32_MAX)) {
            logErr(descriptor, "frame index out of bounds");
            return false;
        }

        if (nTiles <= idx) {
            logErr(descriptor, "frame index out of range for animated tile");
            return false;
        }

        framesvec.push(tileAt(images, idx));
    }

    float hertz = static_cast<float>(speedNode.toNumber());
    CHECK(hertz > 0.0f);
    frameLen = static_cast<I32>(1000.0f / hertz);

    // Add 'now' to Animation constructor??
    Time now = worldTime();
    graphic = Animation(static_cast<Vector<Image>&&>(framesvec), frameLen);
    graphic.restart(now);

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
    CHECK(propertiesValue.isObject() || propertiesValue.isNull());
    CHECK(dataValue.isArray());

    const I32 x = widthValue.toInt();
    const I32 y = heightValue.toInt();

    if (grid.dim.x != x || grid.dim.y != y) {
        logErr(descriptor, "layer x,y size != map x,y size");
        return false;
    }

    allocateMapLayer(TileGrid::TILE_LAYER);

    if (propertiesValue.isObject()) {
        CHECK(processLayerProperties(propertiesValue));
    }
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

    if (!depthValue.isString()) {
        logErr(descriptor, "A tilelayer must have the \"depth\" property");
        return false;
    }

    String buf = depthValue.toString();
    float depth;
    if (!parseFloat(depth, buf)) {
        logErr(descriptor, "A tilelayer must have the \"depth\" property");
        return false;
    }

    if (grid.depth2idx.contains(depth)) {
        logErr(descriptor, "Layers cannot share a depth");
        return false;
    }

    grid.depth2idx[depth] = grid.dim.z - 1;

    // Effectively idx2depth[dim.z - 1] = depth;
    grid.idx2depth.push(depth);

    return true;
}

bool
AreaJSON::processLayerData(JsonValue arr) noexcept {
    /*
     [9, 9, 9, ..., 3, 9, 9]
    */

    const Size z = static_cast<Size>(grid.dim.z) - 1;

    // If we ever allow finding layers out of order.
    // assert_(0 <= z && z < dim.z);

    Size x = 0, y = 0;

    for (JsonIterator node = begin(arr); node != end(arr); ++node) {
        CHECK(node->value.isNumber());

        U32 gid = node->value.toInt();

        if (gid >= tileGraphics.size) {
            logErr(descriptor, "Invalid tile gid");
            return false;
        }

        Size idx = (z * grid.dim.y + y) * grid.dim.x + x;

        // A gid of zero means there is no tile at this
        // position on this layer.
        grid.graphics[idx] = gid;

        if (++x == static_cast<Size>(grid.dim.x)) {
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

    CHECK(propertiesValue.isObject() || propertiesValue.isNull());
    CHECK(objectsValue.isArray());

    if (propertiesValue.isObject()) {
        CHECK(processObjectGroupProperties(propertiesValue));
    }

    for (JsonIterator objectNode = begin(objectsValue);
         objectNode != end(objectsValue); ++objectNode) {
        CHECK(objectNode->value.isObject());
        CHECK(processObject(objectNode->value));
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

    if (!depthValue.isString()) {
        logErr(descriptor, "An objectlayer must have the \"depth\" property");
        return false;
    }

    String buf = depthValue.toString();
    float depth;
    if (!parseFloat(depth, buf)) {
        logErr(descriptor, "An objectlayer must have the \"depth\" property");
        return false;
    }

    if (grid.depth2idx.contains(depth)) {
        logErr(descriptor, "Layers cannot share a depth");
        return false;
    }

    allocateMapLayer(TileGrid::OBJECT_LAYER);
    grid.depth2idx[depth] = grid.dim.z - 1;

    // Effectively idx2depth[dim.z - 1] = depth;
    grid.idx2depth.push(depth);

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

    String buf;
    float f;

    if (layermodValue.isString()) {
        buf.clear();
        buf = layermodValue.toString();
        CHECK(parseFloat(f, buf));
    }
    else {
        CHECK(layermodValue.isNull());
    }

    if (layermodupValue.isString()) {
        buf.clear();
        buf = layermodupValue.toString();
        CHECK(parseFloat(f, buf));
    }
    else {
        CHECK(layermodupValue.isNull());
    }

    if (layermoddownValue.isString()) {
        buf.clear();
        buf = layermoddownValue.toString();
        CHECK(parseFloat(f, buf));
    }
    else {
        CHECK(layermoddownValue.isNull());
    }

    if (layermodleftValue.isString()) {
        buf.clear();
        buf = layermodleftValue.toString();
        CHECK(parseFloat(f, buf));
    }
    else {
        CHECK(layermodleftValue.isNull());
    }

    if (layermodrightValue.isString()) {
        buf.clear();
        buf = layermodrightValue.toString();
        CHECK(parseFloat(f, buf));
    }
    else {
        CHECK(layermodrightValue.isNull());
    }

    const Size z = static_cast<Size>(grid.dim.z) - 1;

    // If we ever allow finding layers out of order.
    // assert_(0 <= z && z < dim.z);

    // Gather object properties now. Assign them to tiles later.
    bool wwide[5] = {}, hwide[5] = {};  // Wide exit in width or height.

    void (*enterScript)(DataArea*, Entity*, ivec3) noexcept = 0;
    void (*leaveScript)(DataArea*, Entity*, ivec3) noexcept = 0;
    void (*useScript)(DataArea*, Entity*, ivec3) noexcept = 0;

    bool haveExit[5] = {};
    Exit exit[5];
    bool haveLayermod[5] = {};
    float layermod[5];
    U32 flags = 0x0;

    if (flagsValue.isString()) {
        CHECK(splitTileFlags(flagsValue.toString(), &flags));
    }

    // For some reason, Visual Studio 2015 and up widen a void (*) noexcept to
    // a void (*). This latter type does not have a noexcept qualifier. Wonder
    // why do they do this...
    //
    // The solution suggested by the compiler is to use a reinterpret_cast to
    // bring back the noexcept.
    if (onenterValue.isString()) {
        StringView scriptName = onenterValue.toString();
        enterScript =
            reinterpret_cast<void (*)(DataArea*, Entity*, ivec3) noexcept>(
                *dataArea->scripts.find(hash_(scriptName)));
    }
    if (onleaveValue.isString()) {
        StringView scriptName = onleaveValue.toString();
        leaveScript =
            reinterpret_cast<void (*)(DataArea*, Entity*, ivec3) noexcept>(
                *dataArea->scripts.find(hash_(scriptName)));
    }
    if (onuseValue.isString()) {
        StringView scriptName = onuseValue.toString();
        useScript =
            reinterpret_cast<void (*)(DataArea*, Entity*, ivec3) noexcept>(
                *dataArea->scripts.find(hash_(scriptName)));
    }

    if (exitValue.isString()) {
        haveExit[EXIT_NORMAL] = true;
        StringView value = exitValue.toString();
        CHECK(parseExit(value, exit[EXIT_NORMAL], &wwide[EXIT_NORMAL],
                        &hwide[EXIT_NORMAL]));
        flags |= TILE_NOWALK_NPC;
    }
    if (exitupValue.isString()) {
        haveExit[EXIT_UP] = true;
        StringView value = exitupValue.toString();
        CHECK(
            parseExit(value, exit[EXIT_UP], &wwide[EXIT_UP], &hwide[EXIT_UP]));
    }
    if (exitdownValue.isString()) {
        haveExit[EXIT_DOWN] = true;
        StringView value = exitdownValue.toString();
        CHECK(parseExit(value, exit[EXIT_DOWN], &wwide[EXIT_DOWN],
                        &hwide[EXIT_DOWN]));
    }
    if (exitleftValue.isString()) {
        haveExit[EXIT_LEFT] = true;
        StringView value = exitleftValue.toString();
        CHECK(parseExit(value, exit[EXIT_LEFT], &wwide[EXIT_LEFT],
                        &hwide[EXIT_LEFT]));
    }
    if (exitrightValue.isString()) {
        haveExit[EXIT_RIGHT] = true;
        StringView value = exitrightValue.toString();
        CHECK(parseExit(value, exit[EXIT_RIGHT], &wwide[EXIT_RIGHT],
                        &hwide[EXIT_RIGHT]));
    }

    if (layermodValue.isString()) {
        haveLayermod[EXIT_NORMAL] = true;
        buf.clear();
        buf = layermodValue.toString();
        CHECK(parseFloat(layermod[EXIT_NORMAL], buf));
        flags |= TILE_NOWALK_NPC;
    }
    if (layermodupValue.isString()) {
        haveLayermod[EXIT_UP] = true;
        buf.clear();
        buf = layermodupValue.toString();
        CHECK(parseFloat(layermod[EXIT_UP], buf));
    }
    if (layermoddownValue.isString()) {
        haveLayermod[EXIT_DOWN] = true;
        buf.clear();
        buf = layermoddownValue.toString();
        CHECK(parseFloat(layermod[EXIT_DOWN], buf));
    }
    if (layermodleftValue.isString()) {
        haveLayermod[EXIT_LEFT] = true;
        buf.clear();
        buf = layermodleftValue.toString();
        CHECK(parseFloat(layermod[EXIT_LEFT], buf));
    }
    if (layermodrightValue.isString()) {
        haveLayermod[EXIT_RIGHT] = true;
        buf.clear();
        buf = layermodrightValue.toString();
        CHECK(parseFloat(layermod[EXIT_RIGHT], buf));
    }

    // Apply these properties directly to one or more tiles in a rectangle
    // of the map. We don't keep an intermediary "object" object lying
    // around.

    const I32 x = xValue.toInt() / grid.tileDim.x;
    const I32 y = yValue.toInt() / grid.tileDim.y;
    const I32 w = widthValue.toInt() / grid.tileDim.x;
    const I32 h = heightValue.toInt() / grid.tileDim.y;

    CHECK(x + w <= grid.dim.x);
    CHECK(y + h <= grid.dim.y);

    // We know which Tiles are being talked about now... yay
    for (I32 Y = y; Y < y + h; Y++) {
        for (I32 X = x; X < x + w; X++) {
            ivec3 tile = {X, Y, static_cast<I32>(z)};

            grid.flags[tile] |= flags;
            for (Size i = 0; i < EXITS_LENGTH; i++) {
                if (haveExit[i]) {
                    I32 dx = X - x;
                    I32 dy = Y - y;
                    if (wwide[i]) {
                        exit[i].coords.x += dx;
                    }
                    if (hwide[i]) {
                        exit[i].coords.y += dy;
                    }
                    grid.exits[i][tile] = static_cast<Exit&&>(exit[i]);
                }
            }
            for (Size i = 0; i < EXITS_LENGTH; i++) {
                if (haveLayermod[i]) {
                    grid.layermods[i][tile] = layermod[i];
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
AreaJSON::splitTileFlags(StringView strOfFlags, U32* flags) noexcept {
    Vector<StringView> flagStrs;
    splitStr(flagStrs, strOfFlags, ",");

    for (StringView* flagStr = flagStrs.begin(); flagStr != flagStrs.end();
         flagStr++) {
        if (*flagStr == "nowalk") {
            *flags |= TILE_NOWALK;
        }
        else if (*flagStr == "nowalk_player") {
            *flags |= TILE_NOWALK_PLAYER;
        }
        else if (*flagStr == "nowalk_npc") {
            *flags |= TILE_NOWALK_NPC;
        }
        else {
            logErr(descriptor, String() << "Invalid tile flag: " << *flagStr);
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
    const I32 space = 0;
    const I32 digit = 1;
    const I32 sign = 2;

    I32 state = space;

    for (const char* c = s.begin(); c != s.end(); c++) {
        if (state == space) {
            if (*c == ' ') {
                continue;
            }
            else {
                state++;
            }
        }
        if (state == digit) {
            if ('0' <= *c && *c <= '9') {
                continue;
            }
            else {
                state++;
            }
        }
        if (state == sign) {
            return *c == '+';
        }
    }
    return true;
}

bool
AreaJSON::parseExit(StringView dest, Exit& exit, bool* wwide,
                    bool* hwide) noexcept {
    /*
      Format: destination area, x, y, z
      E.g.:   "babysfirst.area,1,3,0"
    */

    Vector<StringView> strs;
    splitStr(strs, dest, ",");

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

    String buf;

    I32 x_;
    I32 y_;
    float z_;

    buf.clear();
    buf = x;
    CHECK(parseInt(x_, buf));

    buf.clear();
    buf = y;
    CHECK(parseInt(y_, buf));

    buf.clear();
    buf = z;
    CHECK(parseFloat(z_, buf));

    exit.area = area;
    exit.coords.x = x_;
    exit.coords.y = y_;
    exit.coords.z = z_;

    *wwide = x.find('+') != SV_NOT_FOUND;
    *hwide = y.find('+') != SV_NOT_FOUND;

    return true;
}

bool
AreaJSON::parseARGB(StringView str, U8& a, U8& r, U8& g, U8& b) noexcept {
    U8* channels[] = {&a, &r, &g, &b};

    Vector<StringView> strs;
    splitStr(strs, str, ",");

    if (strs.size != 4) {
        logErr(descriptor, "invalid ARGB format");
        return false;
    }

    String buf;
    for (Size i = 0; i < 4; i++) {
        buf.clear();
        buf = strs[i];
        I32 v;
        if (!parseInt(v, buf)) {
            logErr(descriptor, "invalid ARGB format");
            return false;
        }
        if (!(0 <= v && v < 256)) {
            logErr(descriptor, "ARGB values must be between 0 and 255");
            return false;
        }
        *channels[i] = static_cast<U8>(v);
    }

    return true;
}
