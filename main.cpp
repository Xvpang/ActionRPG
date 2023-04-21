#include <atomic>
#include <brynet/base/AppStatus.hpp>
#include <brynet/net/EventLoop.hpp>
#include <brynet/net/TcpService.hpp>
#include <brynet/net/wrapper/ServiceBuilder.hpp>
#include <iostream>
#include <mutex>

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include <iostream>

using namespace rapidjson;


using namespace brynet;
using namespace brynet::net;


class RPGServer
{
public:
    RPGServer(){}

    bool startUp(size_t threads_num, size_t port)
    {
        if (_service) return false;

        _service = IOThreadTcpService::Create();
        _service->startWorkerThread(threads_num);

        auto enterCallback = [this](const TcpConnection::Ptr& session) {
            _connections.emplace_back(session);
            std::cout << "join" << std::endl;
            session->setDataCallback([session, this](brynet::base::BasePacketReader& reader) {
                while (reader.enough(sizeof(int32_t)))
                {
                    int32_t size = reader.readINT32();
                    if (reader.enough(size))
                    {
                        const char* buffer = reader.currentBuffer();

                        std::cout << "size:" << size << " json string:" << std::string(buffer) << std::endl;
                        Document d;
                        const GenericDocument<UTF8<>>& res = d.Parse(buffer);
                        if (!res.HasParseError())
                        {
                            HandleRecv(session, d);
                        }
                        else
                        {
                            std::cout << "parse error:" << res.GetParseError() << std::endl;
                        }
                    }
                }
                reader.consumeAll();
            });

            session->setDisConnectCallback([this](const TcpConnection::Ptr& session) {
                (void) session;
                auto iter = std::find(_connections.begin(), _connections.end(), session);
                if (iter != _connections.end())
                {
                    std::cout << "leave" << std::endl;
                    _connections.erase(iter);
                }
            });
        };

        _listener.WithService(_service)
                .AddSocketProcess({[](TcpSocket& socket) {
                    socket.setNodelay();
                }})
                .WithMaxRecvBufferSize(1024)
                .AddEnterCallback(enterCallback)
                .WithAddr(false, "0.0.0.0", port)
                .asyncRun();
    }

    void Send(const TcpConnection::Ptr& session, const StringBuffer& str_buffer)
    {
        std::string str = str_buffer.GetString();
        int32_t header_size = sizeof(int32_t);
        int32_t data_size = str.size() + 1;

        session->send(reinterpret_cast<const char*>(&data_size), header_size);
        session->send(str.c_str(), data_size);

        std::cout << "send size:" << str_buffer.GetSize() << "content:" << str_buffer.GetString() << std::endl;
    }

protected:
    virtual void HandleRecv(const TcpConnection::Ptr& session, const Document& doc)
    {
        if (doc.HasMember("Hello"))
        {
            const GenericValue<UTF8<>>& val = doc["Hello"];
            const std::string str = val.GetString();
            std::cout << str << std::endl;
        }

        StringBuffer send_buffer;
        Writer<StringBuffer> writer(send_buffer);

        writer.StartObject();
        writer.Key("key"), writer.String("value");
        writer.EndObject();

        Send(session, send_buffer);
    }

    std::vector<TcpConnection::Ptr> _connections;
    IOThreadTcpService::Ptr _service;
    wrapper::ListenerBuilder _listener;
};

int main(int argc, char** argv)
{
    RPGServer Server;
    Server.startUp(1, 10103);

    EventLoop mainLoop;
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
