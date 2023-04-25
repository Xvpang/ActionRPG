//
// Created by xvpang on 2023/4/22.
//

#include "DS.h"

#include "GameClient.h"
#include "RPGServer.h"

using namespace rapidjson;
extern std::shared_ptr<RPGServer<GameClient>> GameServer;

DS::DS(const brynet::net::TcpConnection::Ptr &session)
        : RPGClient(session)
{
    static int32_t static_id = 0;
    while(static_id++ == 0)
    {}
    _session_id = static_id;
}

void DS::makeDSJson(rapidjson::Writer<rapidjson::StringBuffer> &writer)
{
    writer.StartObject();

    std::string SessionName = _session_name.length() > 0 ? _session_name : std::string(_session->getIP());

    writer.Key("SessionId"); writer.Int(_session_id);
    writer.Key("TdsSessionName"); writer.String(SessionName.c_str());
    writer.Key("MaxPlayers"); writer.Int(_max_clients);
    writer.Key("CurrentPlayer"); writer.Int(static_cast<int32_t>(_game_clients.size()));

    writer.EndObject();
}

void DS::makeAllJson(rapidjson::Writer<rapidjson::StringBuffer> &writer)
{
    writer.StartObject();

    std::string SessionName = _session_name.length() > 0 ? _session_name : std::string(_session->getIP());

    writer.Key("SessionId"); writer.Int(_session_id);
    writer.Key("TdsSessionName"); writer.String(SessionName.c_str());
    writer.Key("MaxPlayers"); writer.Int(_max_clients);
    writer.Key("CurrentPlayer"); writer.Int(static_cast<int32_t>(_game_clients.size()));

    writer.Key("Players");
    writer.StartArray();
    for (auto& iter : _game_clients)
    {
        iter.lock()->makeJson(writer);
    }
    writer.EndArray();

    writer.EndObject();
}

void DS::handleMessage(EMessageType type, const rapidjson::Document &doc)
{
    switch (type)
    {
        case SayHello:
        {
            if (doc.HasMember("TdsSessionName") && doc["TdsSessionName"].IsString())
            {
                _session_name = doc["TdsSessionName"].GetString();
            }
            if (doc.HasMember("MaxPlayers") && doc["MaxPlayers"].IsInt())
            {
                _max_clients = doc["MaxPlayers"].GetInt();
            }
            if (doc.HasMember("IpOverride") && doc["IpOverride"].IsString())
            {
                _ip_override = doc["IpOverride"].GetString();
            }
        }
            break;
        case ServerUpdateReadyNtf:
        {
            if (doc.HasMember("Ready") && doc["Ready"].IsBool())
            {
                _ready = doc["Ready"].GetBool();
                std::string session_name;
            }
        }
            break;
        default: ;
    }
}

bool DS::handleJoin(const std::shared_ptr<GameClient>& client)
{
    if (_game_clients.size() <= _max_clients)
    {
        _game_clients.push_back(client);
        return true;
    }
    return false;
}

void DS::handleLeave(const std::shared_ptr<GameClient> &client)
{
    for (auto iter = _game_clients.begin(); iter != _game_clients.end(); ++iter)
    {
        if (iter->lock() == client)
        {
            _game_clients.erase(iter);
            return;
        }
    }
}

void DS::travel()
{
    std::string ip = _ip_override.empty() ? _session->getIP() : _ip_override;
    for (auto iter: _game_clients)
    {
        if (!iter.expired())
        {
            iter.lock()->clientTravel(ip);
        }
    }
}

void DS::broadcastSessionInfo()
{
    StringBuffer send_buffer;
    Writer<StringBuffer> writer(send_buffer);

    makeAllJson(writer);

    for (const std::weak_ptr<GameClient>& iter : _game_clients)
    {
        iter.lock()->sendMessage(EMessageType::JoinSessionResponse, &send_buffer);
    }
}
