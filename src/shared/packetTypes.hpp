#include <cstdint>

enum class PacketType : uint8_t // needs to be in byte format so enumerate as such 
{
    INPUT, // Client -> Server
    GAMESTATE, // Server -> Client
};

struct PacketHeader
{
    PacketType type;
};