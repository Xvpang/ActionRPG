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
                        int32_t size = reader.readINT32();

                        std::cout << "Recv type: " << type << "  size: " << size << std::endl;

                        rapidjson::Document d;
                        if (reader.enough(size))
                        {
                            const char* buffer = reader.currentBuffer();

                            const rapidjson::GenericDocument<rapidjson::UTF8<>>& res = d.Parse(buffer);
                            if (!res.HasParseError())
                            {
                                handleMessage(type, d);
                            }
                            else
                            {
                                std::cout << "parse error:" << res.GetParseError() << std::endl;
                            }
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
    int32_t data_size = str_buffer ? static_cast<int32_t>(str_buffer->GetSize()) + 1 : 0;
    _session->send(reinterpret_cast<const char*>(&type), 4);
    _session->send(reinterpret_cast<const char*>(&data_size), 4);

    if (str_buffer)
    {
        std::string str = str_buffer->GetString();
        _session->send(str.c_str(), data_size);
    }

    std::cout << "Send type: " << type << "  size:" << data_size + 8 << std::endl;
}

