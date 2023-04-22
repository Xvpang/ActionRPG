//
// Created by xvpang on 2023/4/22.
//

#ifndef TAPSERVER_DS_H
#define TAPSERVER_DS_H

#include "RPGClient.h"

class GameClient;

class DS : public RPGClient
{
public:
    explicit DS(const brynet::net::TcpConnection::Ptr& session);

    void makeDSJson(rapidjson::Writer<rapidjson::StringBuffer>& writer);

    void makeAllJson(rapidjson::Writer<rapidjson::StringBuffer>& writer);

    void travel();

    bool _ready = false;

    std::vector<std::weak_ptr<GameClient>> _game_clients;
    int32_t _max_clients = 8;

    std::string _session_name;

    int32_t _session_id = 0;
    brynet::net::TcpConnection::Ptr _session;

    void handleMessage(EMessageType type, const rapidjson::Document &doc) override;

    bool handleJoin(const std::shared_ptr<GameClient>& client);

    void handleLeave(const std::shared_ptr<GameClient>& client);
};


#endif //TAPSERVER_DS_H
