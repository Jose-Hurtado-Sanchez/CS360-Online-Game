#include <iostream>
#include <cstring>
#include <enet/enet.h>
#include "../shared/packets.hpp"
#include "../game/player.hpp"

    Player player1;
    Player player2;
    uint32_t nextPlayerId = 1;

    void sendGameState()
    {
        if (!player1.getPeer() || !player2.getPeer()) 
        {
            return; // if one player is missing dont send anything.
        }
        GamestatePacket currentGameState;
        currentGameState.p1_id = player1.getId();
        currentGameState.p2_id = player2.getId();
        currentGameState.p1_x = player1.getX();
        currentGameState.p2_x = player2.getX();
        currentGameState.p1_health = player1.getHealth();
        currentGameState.p2_health = player2.getHealth();
        currentGameState.p1_move = MoveType::NONE;
        currentGameState.p2_move = MoveType::NONE;
        currentGameState.p1_action = player1.getCurrentAction();
        currentGameState.p2_action = player2.getCurrentAction();

        // Create packet
        ENetPacket* packet = enet_packet_create(nullptr, sizeof(PacketHeader) + sizeof(GamestatePacket), ENET_PACKET_FLAG_RELIABLE);
        PacketHeader* header = (PacketHeader*)packet->data;
        header->type = PacketType::GAMESTATE;
        memcpy(packet->data + sizeof(PacketHeader), &currentGameState, sizeof(GamestatePacket));

        // Send to both players
        enet_peer_send(player1.getPeer(), 0, packet);
        enet_peer_send(player2.getPeer(), 0, packet);
    }

    void onConnect(ENetPeer* peer)
    {
        if (nextPlayerId == 1) {
            player1.setId(nextPlayerId);
            player1.setPeer(peer);
            player1.startGame();  // Initialize health and position
            std::cout << "Player 1 (host) connected" <<std::endl;
        } else if (nextPlayerId == 2) {
            player2.setId(nextPlayerId);
            player2.setPeer(peer);
            player2.startGame();  // Initialize health and position
            std::cout << "Player 2 connected" << std::endl;
        } else {
            // Reject additional connections cause only 2 players allowed
            enet_peer_disconnect(peer, 0);
            return;
        }
        nextPlayerId++;
    }

    void onRecieve(ENetPeer* peer, ENetPacket* packet)
    {
        //printf("Do something onRecieve with packet");

        PacketHeader * header = (PacketHeader*)packet->data; // gets what type of packet

        switch(header->type)
        {
            case PacketType :: INPUT:
                InputPacket * input = (InputPacket*)(packet->data + sizeof(PacketHeader));
                
                // Find the player by peer
                Player* currentPlayer = nullptr;
                if (peer == player1.getPeer()) 
                {
                    currentPlayer = &player1;
                } else if (peer == player2.getPeer()) 
                {
                    currentPlayer = &player2;
                }
                
                
                player1.applyInput(*input, peer, &player2);
                player2.applyInput(*input, peer, &player1);
                //  printf("Applied input for Player %u\n", currentPlayer->getId());
                
                // After applying input, broadcast the updated game state
                sendGameState();
                
            
             break;
        }
    }          

    void onDisconnect(ENetPeer*)
    {
        std::cout << "Do something onDisconnect" << std::endl;
    }


