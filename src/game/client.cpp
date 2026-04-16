#include "main_client.hpp"

//function to send player input to the server 
void sendInput(ENetPeer* peer, float dx, float dy, bool attack, bool block)
{
    //ensure that the connection is valid
    if (!peer) return;

    //create and populate the input packet with player input data
    InputPacket input{};
    input.dx = dx; //movement in the x direction
    input.dy = dy; //movement in the y direction
    input.attack = attack; //player is attacking
    input.block = block; //player is blocking


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
void handlePacket(ENetPacket* packet, float& p1x, float& p2x, int& p1Health, int& p2Health)
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
            std::cout << "GameState received: "
                      << "P1 x=" << p1x << " hp=" << p1Health
                      << " | P2 x=" << p2x << " hp=" << p2Health << "\n";
            break;
        }

        default:
        //error for unknown type
            std::cout << "Unknown packet type received\n";
            break;
    }
}

int main()
{
    if (enet_initialize() != 0)
    {
        std::cout << "ENet failed to initialize\n";
        return 1;
    }

    atexit(enet_deinitialize);

    ENetHost* client = enet_host_create(nullptr, 1, 1, 0, 0);
    if (!client)
    {
        std::cout << "Client was not created\n";
        return 1;
    }

    ENetAddress address;
    ENetEvent event;
    ENetPeer* peer;

    enet_address_set_host(&address, "127.0.0.1");
    address.port = 7777; // must match server

    peer = enet_host_connect(client, &address, 1, 0);
    if (!peer)
    {
        std::cout << "No available peers for connection\n";
        enet_host_destroy(client);
        return 1;
    }

    std::cout << "Attempting to connect...\n";

    if (enet_host_service(client, &event, 5000) > 0 &&
        event.type == ENET_EVENT_TYPE_CONNECT)
    {
        std::cout << "Connected to server\n";
    }
    else
    {
        std::cout << "Connection failed\n";
        enet_peer_reset(peer);
        enet_host_destroy(client);
        return 1;
    }

    sf::RenderWindow window(sf::VideoMode({1000, 600}), "Fighting Game Client");
    window.setFramerateLimit(60);

    sf::Clock clock;

    sf::RectangleShape player1Shape(sf::Vector2f(80.f, 120.f));
    sf::RectangleShape player2Shape(sf::Vector2f(80.f, 120.f));

    player1Shape.setPosition({200.f, 350.f});
    player2Shape.setPosition({700.f, 350.f});

    player1Shape.setFillColor(sf::Color::Blue);
    player2Shape.setFillColor(sf::Color::Red);

    float p1x = 200.f;
    float p2x = 700.f;
    int p1Health = 100;
    int p2Health = 100;

    while (window.isOpen())
    {
        float dt = clock.restart().asSeconds();
        (void)dt;

        while (const std::optional<sf::Event> ev = window.pollEvent())
        {
            if (ev->is<sf::Event::Closed>())
            {
                window.close();
            }
        }

        float dx = 0.0f;
        float dy = 0.0f;
        bool attack = false;
        bool block = false;

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A))
            dx = -1.0f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D))
            dx = 1.0f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W))
            dy = -1.0f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S))
            dy = 1.0f;

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::J))
            attack = true;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::K))
            block = true;

        sendInput(peer, dx, dy, attack, block);
        enet_host_flush(client);

        while (enet_host_service(client, &event, 0) > 0)
        {
            switch (event.type)
            {
                case ENET_EVENT_TYPE_RECEIVE:
                    handlePacket(event.packet, p1x, p2x, p1Health, p2Health);
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

        player1Shape.setPosition({p1x, 350.f});
        player2Shape.setPosition({p2x, 350.f});

        window.clear(sf::Color(30, 30, 30));
        window.draw(player1Shape);
        window.draw(player2Shape);
        window.display();
    }

    if (peer)
    {
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

    enet_host_destroy(client);
    return 0;
}