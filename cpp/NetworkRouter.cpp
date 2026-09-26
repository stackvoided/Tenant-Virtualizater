#include "TenantManager.h"
#include "TenantTypes.h"
#include <cstdint>
#include <cstring>

struct OutgoingPacketDescriptor {
    uint16_t player_id;
    uint32_t data_len;
    uint8_t payload[512];
};

static LockFreeRingBuffer<OutgoingPacketDescriptor, 4096> g_PacketPipeline;

namespace NetworkEngine {

    bool PushOutgoingPacket(uint16_t player_id, const uint8_t* data, uint32_t len) {
        if (!data || len > 512) return false;

        OutgoingPacketDescriptor pkt;
        pkt.player_id = player_id;
        pkt.data_len = len;
        std::memcpy(pkt.payload, data, len);

        return g_PacketPipeline.Push(pkt);
    }

    void ProcessNetworkQueue() {
        OutgoingPacketDescriptor pkt;
        while (g_PacketPipeline.Pop(pkt)) {
            int32_t tenant_id = TenantManager::GetInstance().ResolvePlayerTenant(pkt.player_id);
            if (tenant_id == -1) continue;

            TenantInstance* ctx = TenantManager::GetInstance().GetTenantContext(tenant_id);
            if (ctx && ctx->active.load(std::memory_order_acquire)) {
            }
        }
    }

}
