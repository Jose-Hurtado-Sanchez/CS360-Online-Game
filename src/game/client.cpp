#include "client.hpp"

static float p1x = 100.0f; // player 1's x position
static float p2x = 700.0f; // player 2's x position
static float p1Health = 100.0f; // player 1's health
static float p2Health = 100.0f; // player 2's health

//enum for player movement types
enum class AnimationState
{
    IDLE,
    WALKING,
    JAB,
    CROSS, 
    BLOCK_JAB, 
    BLOCK_CROSS
};

static AnimationState p1State = AnimationState::IDLE; // player 1's animation state
static AnimationState p2State = AnimationState::IDLE; // player 2's animation state

static float prevP1x = 0.0f; // player 1's previous x position
static float prevP2x = 0.0f; // player 2's previous x position

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
        ENET_PACKET_FLAG_RELIABLE // reliable packet to ensure it arrives(lowk might wanna change it to unreliable to make it faster)
    );

    //add packet header to the specific packet 
    PacketHeader* header = reinterpret_cast<PacketHeader*>(packet->data); // point the header to the start of the packet data
    header->type = PacketType::INPUT; // set the packet type to input

    //copy the input data into the packet
    std::memcpy(packet->data + sizeof(PacketHeader), &input, sizeof(InputPacket)); // copy the input data into the packet right after the header

    //send it back to the channel
    enet_peer_send(peer, 0, packet); // send the packet to the server on channel 0
}

void updateAnimation(ActionType p1Action, ActionType p2Action){
    //player 1 animationstate
    if(p1Action == ActionType::LEFT_ATTACK){
        p1State = AnimationState::JAB;
    } else if(p1Action == ActionType::LEFT_BLOCK){
        p1State = AnimationState::BLOCK_JAB;
    } else if(p1Action == ActionType::RIGHT_ATTACK){
        p1State = AnimationState::CROSS;
    } else if(p1Action == ActionType::RIGHT_BLOCK){
        p1State = AnimationState::BLOCK_CROSS;
    } else if(p1x != prevP1x){
        p1State = AnimationState::WALKING;
    } else {
        p1State = AnimationState::IDLE;
    }
    //player 2 animationstate
    if(p2Action == ActionType::LEFT_ATTACK){
        p2State = AnimationState::JAB;
    } else if(p2Action == ActionType::LEFT_BLOCK){
        p2State = AnimationState::BLOCK_JAB;
    } else if(p2Action == ActionType::RIGHT_ATTACK){
        p2State = AnimationState::CROSS;
    } else if(p2Action == ActionType::RIGHT_BLOCK){
        p2State = AnimationState::BLOCK_CROSS;
    } else if(p2x != prevP2x){
        p2State = AnimationState::WALKING;
    } else {
        p2State = AnimationState::IDLE;
    }

    prevP1x = p1x;
    prevP2x = p2x;
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
        case PacketType::GAMESTATE: // THIS SHOULD NEVER BE SENT BY CLIENT 
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
            updateAnimation(gs->p1_action, gs->p2_action); //update the animation states based on the actions in the gamestate packet
            //log the game state
            // log the game state
            std::cout << "P1 X: " << gs->p1_x
             << " P2 X: " << gs->p2_x
             << " P1 Health: " << gs->p1_health
             << " P2 Health: " << gs->p2_health << "\n";
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
    enet_address_set_host(&address, "127.0.0.1"); // ADD IP LATER! 127.0.0.1 is for testing
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
   
   //main game loop
MoveType move = MoveType::NONE;

//default action state
ActionType action = ActionType::NONE;

//create the game window
sf::RenderWindow window(sf::VideoMode({800, 600}), "Fighter Game Client");
window.setKeyRepeatEnabled(false);
//load textures for the players and background
sf::Texture idleTexture;
sf::Texture walkTexture;
sf::Texture background;
sf::Texture gameOverTexture; //texture for game over screen, will be set when the game ends

//load animation frames
std::vector<sf::Texture> jabFrames(4);
std::vector<sf::Texture> crossFrames(4);
std::vector<sf::Texture> blockJabFrames(4);
std::vector<sf::Texture> blockCrossFrames(5);

//check if textures are loaded successfully
if(!idleTexture.loadFromFile("assets/p1_base_stance.png")) {
    std::cout << "Failed to load idle texture: assets/p1_base_stance.png\n";
    return 1;
}
if(!walkTexture.loadFromFile("assets/p1_base_stance.png")) {
    std::cout << "Failed to load walk texture: assets/p1_base_stance.png\n";
    return 1;
}
if(!background.loadFromFile("assets/fightingGameBG.png")) {
    std::cout << "Failed to load background texture: assets/fightingGameBG.png\n";
    return 1;
}
if(!gameOverTexture.loadFromFile("assets/endGamephoto.png")) {
    std::cout << "Warning: Failed to load game over texture: assets/endGamephoto.png (optional)\n";
}

//load animation frames
if (!jabFrames[0].loadFromFile("assets/p1_jab1.png") ||
    !jabFrames[1].loadFromFile("assets/p1_jab2.png") ||
    !jabFrames[2].loadFromFile("assets/p1_jab3.png") ||
    !jabFrames[3].loadFromFile("assets/p1_jab4.png") ||
    !blockJabFrames[0].loadFromFile("assets/p1_block_jab1.png") ||
    !blockJabFrames[1].loadFromFile("assets/p1_block_jab2.png") ||
    !blockJabFrames[2].loadFromFile("assets/p1_block_jab3.png") ||
    !blockJabFrames[3].loadFromFile("assets/p1_block_jab4.png") || 
    !crossFrames[0].loadFromFile("assets/p1_cross1.png") ||
    !crossFrames[1].loadFromFile("assets/p1_cross2.png") ||
    !crossFrames[2].loadFromFile("assets/p1_cross3.png") ||
    !crossFrames[3].loadFromFile("assets/p1_cross4.png") ||
    !blockCrossFrames[0].loadFromFile("assets/p1_block_cross1.png") ||
    !blockCrossFrames[1].loadFromFile("assets/p1_block_cross2.png") ||
    !blockCrossFrames[2].loadFromFile("assets/p1_block_cross3.png") ||
    !blockCrossFrames[3].loadFromFile("assets/p1_block_cross4.png") ||
    !blockCrossFrames[4].loadFromFile("assets/p1_block_cross5.png"))
{
    std::cout << "Failed to load animation frames\n";
    return 1;
}   

//create sprites for the players and background
sf::Sprite p1Sprite(idleTexture);
sf::Sprite p2Sprite(idleTexture);
sf::Sprite bgSprite(background); 
sf::Sprite gameOverSprite(gameOverTexture); //sprite for game over screen, texture will be set when the game ends

//scale the player sprites to make them larger
p1Sprite.setScale({5.f, 5.f});
p2Sprite.setScale({-5.f, 5.f});

//scale the background to fit the window
bgSprite.setPosition({0.f, 0.f});
sf::Vector2u bgSize = background.getSize();
sf::Vector2u windowSize = window.getSize();

//sclae the endPhoto to fit the window
gameOverSprite.setPosition({0.f, 0.f});
sf::Vector2u gameOverSize = gameOverTexture.getSize();  
sf::Vector2u gameOverWindowSize = window.getSize();

float windowWidth = static_cast<float>(windowSize.x);
//make sure that the x and y fit the background window
bgSprite.setScale(sf::Vector2f(
    static_cast<float>(windowSize.x) / bgSize.x,
    static_cast<float>(windowSize.y) / bgSize.y
));

//make sure that the x and y fit the endgame photo to the window
gameOverSprite.setScale(sf::Vector2f(
    static_cast<float>(windowSize.x) / gameOverSize.x,
    static_cast<float>(windowSize.y) / gameOverSize.y
)); 

//set initial positions for the player sprites
p1Sprite.setPosition({100.f, 400.f});
p2Sprite.setPosition({500.f, 400.f});

//animation frame tracking
static int p1JabFrame = 0;
static int p1BlockJabFrame = 0;
static int p1BlockCrossFrame = 0;
static int p1CrossFrame = 0;
static int p1AttackCounter = 0;
static int p1BlockCounter = 0;
static const int FRAME_DELAY = 100; // frames per animation frame

//animation frame tracking
static int p2JabFrame = 0;
static int p2BlockJabFrame = 0;
static int p2BlockCrossFrame = 0;
static int p2CrossFrame = 0;
static int p2AttackCounter = 0;
static int p2BlockCounter = 0;


//main game loop
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
        move = MoveType::NONE;
        action = ActionType::NONE;

        //movement input
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A))
            move = MoveType::LEFT;
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D))
            move = MoveType::RIGHT;

    //action input not working due to animations needed to be added and bandwith issues (4/20/26)
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::J))
     action = ActionType::LEFT_ATTACK;
    else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::L))
     action = ActionType::RIGHT_ATTACK;
    else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::K))
        action = ActionType::LEFT_BLOCK;
    else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Semicolon))
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

    //update player 1 sprite based on animation state
        switch(p1State){
            case AnimationState::IDLE:
                p1Sprite.setTexture(idleTexture);
                p1AttackCounter = 0;
                p1BlockCounter = 0;
                p1JabFrame = 0;
                p1CrossFrame = 0;
                p1BlockJabFrame = 0;
                p1BlockCrossFrame =0;
                break;
            case AnimationState::WALKING:
                p1Sprite.setTexture(walkTexture);
                break;
            case AnimationState::JAB:
                p1AttackCounter++;
                if (p1AttackCounter >= FRAME_DELAY) {
                    p1AttackCounter = 0;
                    if (p1JabFrame < 3) {
                        p1Sprite.setTexture(jabFrames[p1JabFrame],true);
                        p1JabFrame++;
                    } else {
                        p1State = AnimationState::IDLE;
                    }
                }
                break;
            case AnimationState::CROSS:
                p1AttackCounter++;
                if (p1AttackCounter >= FRAME_DELAY) {
                    p1AttackCounter = 0;
                    if (p1CrossFrame < 3) {
                        p1Sprite.setTexture(crossFrames[p1CrossFrame],true);
                        p1CrossFrame++;
                    } else {
                        p1State = AnimationState::IDLE;
                    }
                }
                break;
            case AnimationState::BLOCK_JAB:
                p1BlockCounter++;
                if (p1BlockCounter >= FRAME_DELAY) {
                    p1BlockCounter = 0;
                    if (p1BlockJabFrame < 3) {
                        p1Sprite.setTexture(blockJabFrames[p1BlockJabFrame],true);
                        p1BlockJabFrame++;
                    } else {
                        p1State = AnimationState::IDLE;
                    }
                }
                break;
            case AnimationState::BLOCK_CROSS:
                p1BlockCounter++;
                if (p1BlockCounter >= FRAME_DELAY) {
                    p1BlockCounter = 0;
                    if (p1BlockCrossFrame < 4) {
                        p1Sprite.setTexture(blockCrossFrames[p1BlockCrossFrame],true);
                        p1BlockCrossFrame++;
                    } else {
                        p1State = AnimationState::IDLE;
                    }
                }
                break;
        }

        //update player 2 sprite based on animation state
        switch(p2State){
            case AnimationState::IDLE:
                p2Sprite.setTexture(idleTexture);
                p2AttackCounter = 0;
                p2BlockCounter = 0;
                p2JabFrame = 0;
                p2CrossFrame = 0;
                p2BlockJabFrame = 0;
                p2BlockCrossFrame =0;
                break;
            case AnimationState::WALKING:
                p2Sprite.setTexture(walkTexture);
                break;
            case AnimationState::JAB:
                p2AttackCounter++;
                if (p2AttackCounter >= FRAME_DELAY) {
                    p2AttackCounter = 0;
                    if (p2JabFrame < 3) {
                        p2Sprite.setTexture(jabFrames[p2JabFrame],true);
                        p2JabFrame++;
                    } else {
                        p2State = AnimationState::IDLE;
                    }
                }
                break;
            case AnimationState::CROSS:
                p2AttackCounter++;
                if (p2AttackCounter >= FRAME_DELAY) {
                    p2AttackCounter = 0;
                    if (p2CrossFrame < 3) {
                        p2Sprite.setTexture(crossFrames[p2CrossFrame],true);
                        p2CrossFrame++;
                    } else {
                        p2State = AnimationState::IDLE;
                    }
                }
                break;
            case AnimationState::BLOCK_JAB:
                p2BlockCounter++;
                if (p2BlockCounter >= FRAME_DELAY) {
                    p2BlockCounter = 0;
                    if (p2BlockJabFrame < 3) {
                        p2Sprite.setTexture(blockJabFrames[p2BlockJabFrame],true);
                        p2BlockJabFrame++;
                    } else {
                        p2State = AnimationState::IDLE;
                    }
                }
                break;
            case AnimationState::BLOCK_CROSS:
                p2BlockCounter++;
                if (p2BlockCounter >= FRAME_DELAY) {
                    p2BlockCounter = 0;
                    if (p2BlockCrossFrame < 4) {
                        p2Sprite.setTexture(blockCrossFrames[p2BlockCrossFrame],true);
                        p2BlockCrossFrame++;
                    } else {
                        p2State = AnimationState::IDLE;
                    }
                }
                break;
        }

//update player sprite positions based on the game state
        p1Sprite.setPosition({p1x, 200.f});
        p2Sprite.setPosition({p2x, 200.f}); 


//render the game
        window.clear();
        window.draw(bgSprite);
        window.draw(p1Sprite);
        window.draw(p2Sprite);
        if (p1Health <= 0 || p2Health <= 0) 
        {
            window.draw(gameOverSprite);
        }
        window.display();


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
