#pragma once
#include "packetTypes.hpp"

enum class MoveType : uint8_t
{
    NONE,
    LEFT,
    RIGHT
};

enum class ActionType : uint8_t
{
    NONE,
    LEFT_ATTACK,
    RIGHT_ATTACK,
    LEFT_BLOCK,
    RIGHT_BLOCK,
};

struct InputPacket
{
    MoveType move;
    ActionType action;
};


struct GamestatePacket
{
   uint8_t p1_id;
   uint8_t p2_id;
   float p1_x; 
   float p2_x; 
   float p1_health;
   float p2_health;
   MoveType p1_move;
   MoveType p2_move;
   ActionType p1_action;
   ActionType p2_action;

};
