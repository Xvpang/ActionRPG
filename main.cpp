
#include "RPGServer.h"
#include "GameClient.h"
#include "DS.h"

#include <brynet/net/EventLoop.hpp>

std::shared_ptr<RPGServer<DS>> DSServer;
std::shared_ptr<RPGServer<GameClient>> GameServer;

int main(int argc, char** argv)
{
    DSServer = std::make_shared<RPGServer<DS>>();
    GameServer = std::make_shared<RPGServer<GameClient>>();

    DSServer->startUp(1, 10103);
    GameServer->startUp(1, 10104);

    brynet::net::EventLoop mainLoop;
    while (true)
    {
        mainLoop.loop(1000);

        if (brynet::base::app_kbhit())
        {
            break;
        }
    }

    return 0;
}
