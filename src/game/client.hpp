#ifndef MAIN_CLIENT_HPP
#define MAIN_CLIENT_HPP

#include <iostream>
#include <cstring>
#include <enet/enet.h>
#include <SFML/Graphics.hpp>
#include "../shared/packets.hpp"

//function to send player input to the server
void sendInput(ENetPeer* peer, float dx, float dy, bool attack = false, bool block = false);
//function to handle the packets received from the server 
void handlePacket(ENetPacket* packet);

#endif