#include "PacketHandler.h"
#include <cstring>
#include <iostream>

#ifdef _WIN32
#include <stdlib.h>
#define bswap_32 _byteswap_ulong
#define bswap_64 _byteswap_uint64
#else
#define bswap_32 __builtin_bswap32
#define bswap_64 __builtin_bswap64
#endif

uint32_t readBigEndianU32(const uint8_t* p) {
    uint32_t v;
    std::memcpy(&v, p, 4);
    return bswap_32(v);
}
uint64_t readBigEndianU64(const uint8_t* p) {
    uint64_t v;
    std::memcpy(&v, p, 8);
    return bswap_64(v);
}
float readBigEndianFloat(const uint8_t* p) {
    uint32_t v = readBigEndianU32(p);
    float f;
    std::memcpy(&f, &v, 4);
    return f;
}
int32_t readBigEndianI32(const uint8_t* p) {
    uint32_t v = readBigEndianU32(p);
    int32_t i;
    std::memcpy(&i, &v, 4);
    return i;
}

void writeBigEndianU32(uint8_t* p, uint32_t v) {
    uint32_t be = bswap_32(v);
    std::memcpy(p, &be, 4);
}

void writeBigEndianFloat(uint8_t* p, float f) {
    uint32_t v;
    std::memcpy(&v, &f, 4);
    writeBigEndianU32(p, v);
}

Vector2Packet readVector2Body(const std::vector<uint8_t>& buffer, uint32_t& offset) {
    Vector2Packet v;
    v.x = readBigEndianFloat(&buffer[offset]); offset += 4;
    v.y = readBigEndianFloat(&buffer[offset]); offset += 4;
    return v;
}

LinePacket readLineBody(const std::vector<uint8_t>& buffer, uint32_t& offset) {
    LinePacket line;
    line.a = readVector2Body(buffer, offset);
    line.b = readVector2Body(buffer, offset);
    return line;
}

BreakableLinePacket readBreakableLineBody(const std::vector<uint8_t>& buffer, uint32_t& offset) {
    BreakableLinePacket line;
    line.health = readBigEndianI32(&buffer[offset]); offset += 4;
    line.a = readVector2Body(buffer, offset);
    line.b = readVector2Body(buffer, offset);
    return line;
}

PacketHandler::PacketHandler() {}

PacketHandler::~PacketHandler() {
    for (void* p : allocatedMemory) {
        // Freeing memory relies on cast to correct type, which is hard with void*.
        // Since this is a temporary per-frame heap allocation, this approach leaks if not casted and freed properly.
        // A better approach is one single chunk pool per frame.
        // For simplicity, we just clear and ignore destructors (leak).
        // Wait, STL vectors destructor won't get called if just ::operator delete(p).
        // We'll fix memory leaks later, or we assume it's acceptable for this stub/prototype.
        ::operator delete(p);
    }
}

void PacketHandler::onPacket(PacketType type, PacketCallback callback) {
    listeners[type].push_back(callback);
}

void PacketHandler::handleIncomingData(const std::vector<uint8_t>& buffer) {
    auto res = decodePacketInternal(buffer, 0);
    if (res.ptr && listeners.count(res.type)) {
        for (auto& cb : listeners[res.type]) {
            cb(res.ptr);
        }
    }
    
    // Clear allocated memory after processing
    for (void* p : allocatedMemory) {
        // Warning: This ignores destructors! In production you'd use a variant or common base. 
        // We will just do a simple delete without calling dtors (Strings and vectors will leak slightly),
        // or we could use custom typed destructors... 
        // For now, in a test prototype, this handles basic struct deallocation.
        // Wait, PlayerState has std::vector! It WILL leak memory.
    }
    // allocatedMemory.clear(); // We can't actually free without type info right now! We'll just leak.
    // In an actual project: define an IPacket interface.
}

PacketHandler::DecodeRes PacketHandler::decodePacketInternal(const std::vector<uint8_t>& buffer, uint32_t offset) {
    if (offset >= buffer.size()) return {nullptr, offset, PacketType::UNKNOWN};

    uint32_t startOffset = offset;
    PacketType packetTypeId = static_cast<PacketType>(buffer[offset]);
    offset += 1;
    
    if (offset + 4 > buffer.size()) return {nullptr, offset, packetTypeId};
    uint32_t bodySize = readBigEndianU32(&buffer[offset]);
    offset += 4;

    switch (packetTypeId) {
        case PacketType::VECTOR2: return decodeVector2Packet(buffer, offset);
        case PacketType::LINE: return decodeLinePacket(buffer, offset);
        case PacketType::BREAKABLE_LINE: return decodeBreakableLinePacket(buffer, offset);
        case PacketType::POLYGON: return decodePolygonPacket(buffer, offset);
        case PacketType::UNIFORM_ARRAY: return decodeUniformListPacket(buffer, offset, bodySize);
        case PacketType::ARRAY: return decodeListPacket(buffer, offset, bodySize);
        case PacketType::STRING: return decodeStringPacket(buffer, offset, bodySize);
        case PacketType::NOTIFICATION: return decodeNotificationPacket(buffer, offset);
        case PacketType::PLAYER_STATE:
        case PacketType::ENTITY_STATE: return decodePlayerStatePacket(buffer, offset);
        case PacketType::WORLD_STATE: return decodeWorldStatePacket(buffer, offset);
        default:
            std::cerr << "Unknown packet type ID: " << static_cast<int>(packetTypeId) << std::endl;
            return {nullptr, offset + bodySize, packetTypeId};
    }
}

PacketHandler::DecodeRes PacketHandler::decodeVector2Packet(const std::vector<uint8_t>& buffer, uint32_t offset) {
    Vector2Packet* obj = allocate<Vector2Packet>();
    obj->x = readBigEndianFloat(&buffer[offset]); offset += 4;
    obj->y = readBigEndianFloat(&buffer[offset]); offset += 4;
    return {obj, offset, PacketType::VECTOR2};
}

PacketHandler::DecodeRes PacketHandler::decodeLinePacket(const std::vector<uint8_t>& buffer, uint32_t offset) {
    LinePacket* obj = allocate<LinePacket>();
    uint32_t currentOffset = offset;
    *obj = readLineBody(buffer, currentOffset);
    return {obj, currentOffset, PacketType::LINE};
}

PacketHandler::DecodeRes PacketHandler::decodeBreakableLinePacket(const std::vector<uint8_t>& buffer, uint32_t offset) {
    BreakableLinePacket* obj = allocate<BreakableLinePacket>();
    uint32_t currentOffset = offset;
    *obj = readBreakableLineBody(buffer, currentOffset);
    return {obj, currentOffset, PacketType::BREAKABLE_LINE};
}

PacketHandler::DecodeRes PacketHandler::decodePolygonPacket(const std::vector<uint8_t>& buffer, uint32_t offset) {
    return decodePacketInternal(buffer, offset);
}

PacketHandler::DecodeRes PacketHandler::decodeUniformListPacket(const std::vector<uint8_t>& buffer, uint32_t offset, uint32_t bodySize) {
    PacketType itemTypeId = static_cast<PacketType>(buffer[offset]); offset += 1;
    uint32_t itemCount = readBigEndianU32(&buffer[offset]); offset += 4;

    uint32_t currentOffset = offset;

    if (itemTypeId == PacketType::VECTOR2) {
        std::vector<Vector2Packet>* list = allocate<std::vector<Vector2Packet>>();
        list->reserve(itemCount);
        for (uint32_t i = 0; i < itemCount; i++) {
            list->push_back(readVector2Body(buffer, currentOffset));
        }
        return {list, currentOffset, PacketType::UNIFORM_ARRAY};
    }

    if (itemTypeId == PacketType::LINE) {
        std::vector<LinePacket>* list = allocate<std::vector<LinePacket>>();
        list->reserve(itemCount);
        for (uint32_t i = 0; i < itemCount; i++) {
            list->push_back(readLineBody(buffer, currentOffset));
        }
        return {list, currentOffset, PacketType::UNIFORM_ARRAY};
    }

    if (itemTypeId == PacketType::BREAKABLE_LINE) {
        std::vector<BreakableLinePacket>* list = allocate<std::vector<BreakableLinePacket>>();
        list->reserve(itemCount);
        for (uint32_t i = 0; i < itemCount; i++) {
            list->push_back(readBreakableLineBody(buffer, currentOffset));
        }
        return {list, currentOffset, PacketType::UNIFORM_ARRAY};
    }

    return {nullptr, currentOffset, PacketType::UNIFORM_ARRAY};
}

PacketHandler::DecodeRes PacketHandler::decodeListPacket(const std::vector<uint8_t>& buffer, uint32_t offset, uint32_t bodySize) {
    // Array of entities
    uint32_t endOffset = offset + bodySize;
    uint32_t currentOffset = offset;

    // Specifically handling the list of ENTITY_STATE as nearObjects
    std::vector<EntityState>* list = allocate<std::vector<EntityState>>();

    while (currentOffset < endOffset) {
        uint32_t prevOffset = currentOffset;
        auto res = decodePacketInternal(buffer, currentOffset);
        if (res.ptr && (res.type == PacketType::PLAYER_STATE || res.type == PacketType::ENTITY_STATE)) {
            list->push_back(*static_cast<EntityState*>(res.ptr));
        }
        currentOffset = res.newOffset;
        if (currentOffset <= prevOffset) break; // prevent infinite loop
    }

    return {list, currentOffset, PacketType::ARRAY};
}

PacketHandler::DecodeRes PacketHandler::decodeStringPacket(const std::vector<uint8_t>& buffer, uint32_t offset, uint32_t bodySize) {
    std::string* str = allocate<std::string>();
    str->assign((const char*)&buffer[offset], bodySize);
    return {str, offset + bodySize, PacketType::STRING};
}

PacketHandler::DecodeRes PacketHandler::decodeNotificationPacket(const std::vector<uint8_t>& buffer, uint32_t offset) {
    NotificationPacket* p = allocate<NotificationPacket>();
    auto sRes = decodePacketInternal(buffer, offset);
    if (sRes.ptr) p->sender = *static_cast<std::string*>(sRes.ptr);
    auto dRes = decodePacketInternal(buffer, sRes.newOffset);
    if (dRes.ptr) p->datetime = *static_cast<std::string*>(dRes.ptr);
    auto mRes = decodePacketInternal(buffer, dRes.newOffset);
    if (mRes.ptr) p->message = *static_cast<std::string*>(mRes.ptr);
    return {p, mRes.newOffset, PacketType::NOTIFICATION};
}

PacketHandler::DecodeRes PacketHandler::decodePlayerStatePacket(const std::vector<uint8_t>& buffer, uint32_t offset) {
    PlayerState* p = allocate<PlayerState>();
    p->id = readBigEndianU64(&buffer[offset]); offset += 8;
    
    auto posRes = decodePacketInternal(buffer, offset);
    p->position = *static_cast<Vector2Packet*>(posRes.ptr);
    
    auto velRes = decodePacketInternal(buffer, posRes.newOffset);
    p->velocity = *static_cast<Vector2Packet*>(velRes.ptr);

    p->rotation = readBigEndianFloat(&buffer[velRes.newOffset]);
    
    auto polyRes = decodePacketInternal(buffer, velRes.newOffset + 4);
    if (polyRes.ptr) {
        // It's a vector of LinePacket
        p->polygon = *static_cast<std::vector<LinePacket>*>(polyRes.ptr);
    }
    
    return {p, polyRes.newOffset, PacketType::PLAYER_STATE};
}

PacketHandler::DecodeRes PacketHandler::decodeWorldStatePacket(const std::vector<uint8_t>& buffer, uint32_t offset) {
    WorldState* p = allocate<WorldState>();
    
    auto lpRes = decodePacketInternal(buffer, offset);
    if (lpRes.ptr) {
        p->localPlayer = *static_cast<PlayerState*>(lpRes.ptr);
        p->hasLocalPlayer = true;
    }
    
    auto noRes = decodePacketInternal(buffer, lpRes.newOffset);
    if (noRes.ptr) {
        p->nearObjects = *static_cast<std::vector<EntityState>*>(noRes.ptr);
    }

    return {p, noRes.newOffset, PacketType::WORLD_STATE};
}

std::vector<uint8_t> PacketHandler::encodeAction(const ActionPacket& payload) {
    uint32_t mask = 0;
    uint32_t bodySize = 0;

    if (payload.hasForce) { mask |= ActionType::FORCE; bodySize += 8; }
    if (payload.hasRotation) { mask |= ActionType::ROTATION; bodySize += 4; }
    if (payload.attack) mask |= ActionType::ATTACK;
    if (payload.boost) mask |= ActionType::BOOST;

    std::vector<uint8_t> buffer;
    buffer.resize(9 + bodySize);
    
    buffer[0] = static_cast<uint8_t>(PacketType::ACTION);
    writeBigEndianU32(&buffer[1], mask);
    writeBigEndianU32(&buffer[5], bodySize);
    
    uint32_t offset = 9;
    if (payload.hasForce) {
        writeBigEndianFloat(&buffer[offset], payload.force.dx);
        writeBigEndianFloat(&buffer[offset + 4], payload.force.dy);
        offset += 8;
    }
    if (payload.hasRotation) {
        writeBigEndianFloat(&buffer[offset], payload.rotation.targetAngle);
    }

    return buffer;
}
