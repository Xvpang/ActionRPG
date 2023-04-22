//
// Created by xvpang on 2023/4/22.
//

#ifndef TAPSERVER_GAMECLIENT_H
#define TAPSERVER_GAMECLIENT_H

#include "RPGClient.h"

class DS;

class GameClient : public RPGClient, public std::enable_shared_from_this<GameClient>
{
public:
    explicit GameClient(const brynet::net::TcpConnection::Ptr& session);

    void makeJson(rapidjson::Writer<rapidjson::StringBuffer>& writer) const;

    void handleMessage(EMessageType type, const rapidjson::Document &doc) override;

    void SendRefreshGameSessionResponse();

    bool joinSession(const std::shared_ptr<DS>& ds);

    void leaveSession();

    void clientTravel(const std::string& ip);

    void onDisconnect() override;

    std::string _id;
    std::string _avatar_url;
    std::string _nick_name;
    brynet::net::TcpConnection::Ptr _session;
    std::weak_ptr<DS> _in_ds;
};


#endif //TAPSERVER_GAMECLIENT_H
