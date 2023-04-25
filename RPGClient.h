//
// Created by xvpang on 2023/4/22.
//

#ifndef TAPSERVER_RPGCLIENT_H
#define TAPSERVER_RPGCLIENT_H
#include <brynet/base/AppStatus.hpp>
#include <brynet/net/TcpService.hpp>

#include <iostream>
#include <mutex>

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"


enum EMessageType : int32_t
{
    SayHello = 1,
    ServerUpdateReadyNtf,
    ServerStartGameResponse,//房间暂时不做验证

    ClientStartGameResponse,
    ClientStartGameRequest,

    Login,
    Logout,
    RefreshGameSessionRequest,
    RefreshGameSessionResponse,
    JoinSessionRequest,
    JoinSessionResponse,
    LeaveSessionRequest,
    LeaveSessionResponse
};

std::string getMessageTypeString(EMessageType type);

enum EErrorType : int32_t
{
    None = 0,
    InvalidSessionId = 1,
    JoinFailed,
};

class RPGClient
{
public:
    RPGClient() = delete;

    explicit RPGClient(const brynet::net::TcpConnection::Ptr& session);

    void sendMessage(EMessageType type, const rapidjson::StringBuffer* str_buffer);

    virtual void handleMessage(EMessageType type, const rapidjson::Document& doc) = 0;

    virtual void onDisconnect() {}

    brynet::net::TcpConnection::Ptr _session;
};

//
#endif //TAPSERVER_RPGCLIENT_H
