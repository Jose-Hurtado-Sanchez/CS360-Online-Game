#include "../shared/packets.hpp"
#include <enet/enet.h>

class Player 
{
    private: 
        float x;
        float health; 
        uint32_t id; // used to track who is who server host is ALWAYS p1
        ENetPeer* peer; // need this because it is were data will be communicated back and forth from server

    public:
        // Getters ----------------------------------------------------------------------
        float getX() 
        {
            return x; 
        }
        float getHealth()  
        { 
            return health; 
        }
        uint32_t getId()  
        { 
            return id; 
        }
        ENetPeer* getPeer()  
        { 
            return peer; 
        }
        // Getters ----------------------------------------------------------------------


        // Setters ------------------------------------------------------------------------
        void setX(float newX) 
        {
             x = newX; 
        }
        void setHealth(float newHealth) 
        { 
            health = newHealth; 
        }
        void setId(uint32_t newId) 
        { 
            id = newId; 
        }
        void setPeer(ENetPeer* newPeer) 
        { 
            peer = newPeer; 
        }
        // Setters ------------------------------------------------------------------------

        void takeDamage()
        {
            health -= 2; 
            if (health < 0) 
            {
                health = 0; //if health is going to be negative set it to 0 instead
            }
        }

        void startGame() 
        {
            health = 100; //Reset health for new round
            
            //Determine x position based on whether the player is the host
            if (id == 1) {
                // Host player starts on the left
                x = 100.0;
            } else {
                //Non-host player starts on the right
                x = 300.0;
            }
        }

        void applyInput(InputPacket input) 
        {
            // Update player state based on input
            switch (input.move) 
            {
                case MoveType::LEFT:
                    x -= 1.0; // Move left
                    break;
                case MoveType::RIGHT:
                    x += 1.0; // Move right
                    break;
                case MoveType::NONE:
                    //No movement
                    break;
            }

            switch (input.action) 
            {
                case ActionType::LEFT_ATTACK:
                    // TODO: Implement left attack logic
                    break;
                case ActionType::RIGHT_ATTACK:
                    // TODO: Implement right attack logic
                    break;
                case ActionType::LEFT_BLOCK:
                    // Implement left block logic
                    break;
                case ActionType::RIGHT_BLOCK:
                    // Implement right block logic
                    break;
                default:
                    break;
            }
        }


        


};