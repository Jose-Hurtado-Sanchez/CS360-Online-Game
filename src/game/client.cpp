#include "client.hpp"

static float p1x = 0.0f; // player 1's x position
static float p2x = 0.0f; // player 2's x position
static float p1Health = 100.0f; // player 1's health
static float p2Health = 100.0f; // player 2's health

//function to send player input to the server 
void sendInput(ENetPeer* peer, MoveType move, ActionType action)
{
    //ensure that the connection is valid
    if (!peer) return;

    //create and populate the input packet with player input data
    InputPacket input{};
    input.move = move;
    input.action = action;


    //enet packet with space for the header 
    ENetPacket* packet = enet_packet_create(
        nullptr, //no data
        sizeof(PacketHeader) + sizeof(InputPacket), //total size of the packet
        ENET_PACKET_FLAG_RELIABLE
    );

    //add packet header to the specific packet 
    PacketHeader* header = reinterpret_cast<PacketHeader*>(packet->data);
    header->type = PacketType::INPUT;

    //copy the input data into the packet
    std::memcpy(packet->data + sizeof(PacketHeader), &input, sizeof(InputPacket));

    //send it back to the channel
    enet_peer_send(peer, 0, packet);
}

//function to handle packets received from the server
void handlePacket(ENetPacket* packet)
{
    //ensure that the packet is valid 
    if (!packet || packet->dataLength < sizeof(PacketHeader)) return;

    //extract the packet header 
    PacketHeader* header = reinterpret_cast<PacketHeader*>(packet->data);

    //handle the packet 
    switch (header->type)
    {
        case PacketType::GAMESTATE:
        {
            if (packet->dataLength < sizeof(PacketHeader) + sizeof(GamestatePacket))
                return;

            GamestatePacket* gs =
                reinterpret_cast<GamestatePacket*>(packet->data + sizeof(PacketHeader));

            //update the gamestate variables 
            p1x = gs->p1_x; //player 1's x position
            p2x = gs->p2_x; //player 2's x position
            p1Health = gs->p1_health; //player 1's health
            p2Health = gs->p2_health; //player 2's health

            //log the game state
            // log the game state
            std::cout << "P1 X: " << gs->p1_x
             << " P2 X: " << gs->p2_x
             << " P1 Health: " << gs->p1_health
             << " P2 Health: " << gs->p2_health << "\n";
        }

        default:
        //error for unknown type
            std::cout << "Unknown packet type received\n";
            break;
    }
}

int main()
{
    if (enet_initialize() != 0)//initialize eNet
    {
        std::cout << "ENet failed to initialize\n";
        return 1;
    }

    atexit(enet_deinitialize);

    //create client host
    ENetHost* client = enet_host_create(nullptr, 1, 1, 0, 0);
    if (!client)
    {
        std::cout << "Client was not created\n";
        return 1;
    }

    ENetAddress address;
    ENetEvent event;
    ENetPeer* peer;

    //set server port
    enet_address_set_host(&address, "127.0.0.1");
    address.port = 7777; // must match server

    //connecting with server
    peer = enet_host_connect(client, &address, 1, 0);
    if (!peer)
    {
        std::cout << "No available peers for connection\n";
        enet_host_destroy(client);
        return 1;
    }

    std::cout << "Attempting to connect...\n";

    //wait 5 seconds to connect
    if (enet_host_service(client, &event, 5000) > 0 &&
        event.type == ENET_EVENT_TYPE_CONNECT)
    {
        std::cout << "Connected to server\n";
    }
    else
    {
        //connection fails
        std::cout << "Connection failed\n";
        enet_peer_reset(peer);
        enet_host_destroy(client);
        return 1;
    }
   
MoveType move = MoveType::NONE;

ActionType action = ActionType::NONE;

sf::Window window(sf::VideoMode({800, 600}), "Fighter Game Client");

while (window.isOpen())
    {


        while (const std::optional<sf::Event> ev = window.pollEvent())
        {
            if (ev->is<sf::Event::Closed>())
            {
                window.close();
            }
        }

        //default state
       MoveType move = MoveType::NONE;
        ActionType action = ActionType::NONE;

        //movement input
       if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A))
        move = MoveType::LEFT;
    else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D))
        move = MoveType::RIGHT;

        //action input
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::J))
     action = ActionType::LEFT_ATTACK;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::J))
     action = ActionType::RIGHT_ATTACK;
    else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::K))
        action = ActionType::LEFT_BLOCK;
    else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::K))
        action = ActionType::RIGHT_BLOCK;

        //send player input to the server
        sendInput(peer, move, action);
        enet_host_flush(client);

        while (enet_host_service(client, &event, 0) > 0)
        {
            switch (event.type)
            {
                //handle incoming packets from the server
                case ENET_EVENT_TYPE_RECEIVE:
                    //update local game state
                    handlePacket(event.packet);
                    enet_packet_destroy(event.packet);
                    break;

                case ENET_EVENT_TYPE_DISCONNECT:
                    std::cout << "Disconnected from server\n";
                    peer = nullptr;
                    window.close();
                    break;

                case ENET_EVENT_TYPE_CONNECT:
                case ENET_EVENT_TYPE_NONE:
                    break;
            }
        }

    }

    if (peer)
    {
        //disconnecting from the server
        enet_peer_disconnect(peer, 0);

        while (enet_host_service(client, &event, 3000) > 0)
        {
            switch (event.type)
            {
                case ENET_EVENT_TYPE_RECEIVE:
                    enet_packet_destroy(event.packet);
                    break;
                case ENET_EVENT_TYPE_DISCONNECT:
                    std::cout << "Disconnected cleanly\n";
                    break;
                default:
                    break;
            }
        }
    }

}
