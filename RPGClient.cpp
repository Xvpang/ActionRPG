//
// Created by xvpang on 2023/4/22.
//

#include "RPGClient.h"
#include "DS.h"


RPGClient::RPGClient(const brynet::net::TcpConnection::Ptr &session)
        : _session(session)
{
    _session->setDataCallback(
            [this](brynet::base::BasePacketReader& reader)
            {
                try
                {
                    while (reader.enough(8))
                    {
                        auto type = static_cast<EMessageType>(reader.readINT32());
                        std::string type_str = getMessageTypeString(type);
                        if (type_str.empty())
                        {
                            std::cout << "Recv error type: " << type << std::endl;
                            reader.consumeAll();
                            return;
                        }
                        int32_t data_size = reader.readINT32();

                        std::cout << "Recv type: " << type_str << "  data_size: " << data_size;

                        rapidjson::Document doc;
                        if (data_size > 0)
                        {
                            if (reader.enough(data_size))
                            {
                                const char* buffer = reader.currentBuffer();
                                std::string json_str = std::string(buffer, data_size);
                                std::cout << "  data: " << json_str << std::endl;

                                const rapidjson::GenericDocument<rapidjson::UTF8<>>& res = doc.Parse(json_str.c_str());
                                if (!res.HasParseError())
                                {
                                    handleMessage(type, doc);
                                }

                                else
                                {
                                    std::cout << "parse error:" << res.GetParseError() << std::endl;
                                }
                            }
                            reader.addPos(data_size);
                        }
                        else
                        {
                            std::cout << std::endl;
                            handleMessage(type, doc);
                        }
                    }
                    reader.savePos();
                }
                catch (...)
                {
                    reader.consumeAll();
                }
            });
}



void RPGClient::sendMessage(EMessageType type, const rapidjson::StringBuffer *str_buffer)
{
    std::string type_str = getMessageTypeString(type);
    if (type_str.empty())
    {
        std::cout << "Send error type: " << type << std::endl;
        return;
    }
    int32_t data_size = str_buffer ? static_cast<int32_t>(str_buffer->GetSize()) : 0;
    _session->send(reinterpret_cast<const char*>(&type), 4);
    _session->send(reinterpret_cast<const char*>(&data_size), 4);

    std::string str;
    if (str_buffer)
    {
        str = str_buffer->GetString();
        _session->send(str.c_str(), data_size);
    }

    std::cout << "Send type: " << type_str << "  data_size:" << data_size << "  data: " << str << std::endl;
}

std::string getMessageTypeString(EMessageType type)
{
    switch (type)
    {
        case SayHello:
            return "SayHello";
        case ServerUpdateReadyNtf:
            return "ServerUpdateReadyNtf";
        case ServerStartGameResponse:
            return "ServerStartGameResponse";
        case ClientStartGameResponse:
            return "ClientStartGameResponse";
        case ClientStartGameRequest:
            return "ClientStartGameRequest";
        case RefreshGameSessionRequest:
            return "RefreshGameSessionRequest";
        case RefreshGameSessionResponse:
            return "RefreshGameSessionResponse";
        case JoinSessionRequest:
            return "JoinSessionRequest";
        case JoinSessionResponse:
            return "JoinSessionResponse";
        case LeaveSessionRequest:
            return "LeaveSessionRequest";
        case LeaveSessionResponse:
            return "LeaveSessionResponse";
        default:
            return "";
    }
}
