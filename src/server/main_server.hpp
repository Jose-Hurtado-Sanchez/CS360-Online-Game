#include <iostream>
#include <enet/enet.h>
#include "../shared/packets.hpp"

void onConnect(ENetPeer* peer)
    {
        printf("Do something onConnect");

    }

void onRecieve(ENetPeer* peer, ENetPacket* packet)
    {
        printf("Do something onRecieve with packet");

        PacketHeader * header = (PacketHeader*)packet->data; // gets what type of packet

        switch(header->type)
        {
            case PacketType :: INPUT:
                InputPacket * input = (InputPacket*)(packet->data + sizeof(PacketHeader));
                //account for input
                break;

            case PacketType :: GAMESTATE: 
                GamestatePacket *gState = (GamestatePacket*)(packet->data + sizeof(PacketHeader));
                //make changes to game state
                break;
        }
       


    }

void onDisconnect(ENetPeer*)
    {
        printf("Do something onDisconnect");
    }