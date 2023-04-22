//
// Created by xvpang on 2023/4/22.
//

#include "GameClient.h"

#include "DS.h"
#include "RPGServer.h"

using namespace rapidjson;

extern std::shared_ptr<RPGServer<DS>> DSServer;

std::shared_ptr<DS> findDSServerBySessionId(const int32_t& session_id)
{
    if (DSServer)
    {
        for(auto& iter : DSServer->_client_map)
        {
            if (iter.second->_session_id == session_id)
            {
                return iter.second;
            }
        }
    }
    return nullptr;
}


GameClient::GameClient(const brynet::net::TcpConnection::Ptr &session)
    : RPGClient(session)
{

}

void GameClient::makeJson(rapidjson::Writer<rapidjson::StringBuffer> &writer) const
{
    writer.StartObject();
    writer.Key("Id"); writer.String(_id.c_str());
    writer.Key("AvatarUrl"); writer.String(_avatar_url.c_str());
    writer.Key("NickName"); writer.String(_nick_name.c_str());
    writer.EndObject();
}

void GameClient::handleMessage(EMessageType type, const rapidjson::Document &doc)
{
    switch (type)
    {
        case SayHello:
        {
            if (doc.HasMember("Id") && doc["Id"].IsString())
            {
                _id = doc["Id"].GetString();
            }
            if (doc.HasMember("AvatarUrl") && doc["AvatarUrl"].IsString())
            {
                _avatar_url = doc["AvatarUrl"].GetString();
            }
            if (doc.HasMember("NickName") && doc["NickName"].IsString())
            {
                _nick_name = doc["NickName"].GetString();
            }
        }
            break;
        case ClientStartGameRequest:
        {
            if (!_in_ds.expired())
            {
                _in_ds.lock()->travel();
            }
        }
            break;
        case RefreshGameSessionRequest:
        {
            SendRefreshGameSessionResponse();
        }
            break;
        case JoinSessionRequest:
        {
            if (doc.HasMember("SessionId") && doc["SessionId"].IsInt())
            {
                StringBuffer send_buffer;
                Writer<StringBuffer> writer(send_buffer);

                writer.StartObject();

                int32_t session_id = doc["SessionId"].GetInt();
                if (std::shared_ptr<DS> ds = findDSServerBySessionId(session_id))
                {
                    if (joinSession(ds))
                    {
                        ds->makeAllJson(writer);
                    }
                    else
                    {
                        writer.Key("ErrorCode"); writer.Int(JoinFailed);
                        SendRefreshGameSessionResponse();//暂时不知道混合着写有没有问题
                    }
                }
                else
                {
                    writer.Key("ErrorCode"); writer.Int(InvalidSessionId);
                    SendRefreshGameSessionResponse();
                }

                writer.EndObject();

                sendMessage(EMessageType::JoinSessionResponse, &send_buffer);
            }
            else
            {
                std::cout << "Join session - Invalid message" << std::endl;
            }
        }
            break;
        case LeaveSessionRequest:
        {
            leaveSession();

            sendMessage(EMessageType::LeaveSessionResponse, nullptr);

            SendRefreshGameSessionResponse();
        }
            break;
        default: ;
    }
}

void GameClient::SendRefreshGameSessionResponse()
{
    StringBuffer send_buffer;
    Writer<StringBuffer> writer(send_buffer);

    writer.StartObject();

    writer.Key("Sessions");
    writer.StartArray();
    for (auto & iter : DSServer->_client_map)
    {
        if (iter.second->_ready)
        {
            iter.second->makeDSJson(writer);
        }
    }
    writer.EndArray();
    writer.EndObject();

    sendMessage(EMessageType::RefreshGameSessionResponse, &send_buffer);
}

bool GameClient::joinSession(const std::shared_ptr<DS> &ds)
{
    if (ds->handleJoin(shared_from_this()))
    {
        _in_ds = ds;
        return true;
    }
    return false;
}

void GameClient::leaveSession()
{
    if (!_in_ds.expired())
    {
        _in_ds.lock()->handleLeave(shared_from_this());
        _in_ds.reset();
    }
}

void GameClient::onDisconnect()
{
    leaveSession();
}

void GameClient::clientTravel(const std::string &ip)
{
    StringBuffer send_buffer;
    Writer<StringBuffer> writer(send_buffer);

    writer.StartObject();
    writer.Key("Ip"); writer.String(ip.c_str());
    writer.EndObject();

    sendMessage(EMessageType::ClientStartGameResponse, &send_buffer);
}

