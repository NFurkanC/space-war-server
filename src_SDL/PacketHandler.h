#pragma once
#include "Packets.h"
#include <vector>
#include <map>
#include <functional>
#include <memory>
#include <cstdint>

class PacketHandler {
public:
    using PacketCallback = std::function<void(const void*)>;

    PacketHandler();
    ~PacketHandler();
    void onPacket(PacketType type, PacketCallback callback);
    void handleIncomingData(const std::vector<uint8_t>& buffer);
    std::vector<uint8_t> encodeAction(const ActionPacket& payload);

private:
    std::map<PacketType, std::vector<PacketCallback>> listeners;
    std::vector<void*> allocatedMemory;

    template<typename T>
    T* allocate() {
        T* ptr = new T();
        allocatedMemory.push_back(ptr);
        return ptr;
    }

    struct DecodeRes {
        void* ptr = nullptr;
        uint32_t newOffset = 0;
        PacketType type = PacketType::UNKNOWN;
    };

    DecodeRes decodePacketInternal(const std::vector<uint8_t>& buffer, uint32_t offset);
    DecodeRes decodeVector2Packet(const std::vector<uint8_t>& buffer, uint32_t offset);
    DecodeRes decodeLinePacket(const std::vector<uint8_t>& buffer, uint32_t offset);
    DecodeRes decodeBreakableLinePacket(const std::vector<uint8_t>& buffer, uint32_t offset);
    DecodeRes decodePolygonPacket(const std::vector<uint8_t>& buffer, uint32_t offset);
    DecodeRes decodeUniformListPacket(const std::vector<uint8_t>& buffer, uint32_t offset, uint32_t bodySize);
    DecodeRes decodeListPacket(const std::vector<uint8_t>& buffer, uint32_t offset, uint32_t bodySize);
    DecodeRes decodeStringPacket(const std::vector<uint8_t>& buffer, uint32_t offset, uint32_t bodySize);
    DecodeRes decodeNotificationPacket(const std::vector<uint8_t>& buffer, uint32_t offset);
    DecodeRes decodePlayerStatePacket(const std::vector<uint8_t>& buffer, uint32_t offset);
    DecodeRes decodeWorldStatePacket(const std::vector<uint8_t>& buffer, uint32_t offset);
};
