//
// Created by xvpang on 2023/4/22.
//

#ifndef TAPSERVER_RPGSERVER_H
#define TAPSERVER_RPGSERVER_H


#include <brynet/base/AppStatus.hpp>
#include <brynet/net/TcpService.hpp>
#include <brynet/net/wrapper/ServiceBuilder.hpp>

#include <mutex>
#include <cstddef>
#include <iostream>
#include <unordered_map>



template<typename ClientType>
class RPGServer
{
public:
    bool startUp(size_t threads_num, size_t port)
    {
        if (_service) return false;

        _service = brynet::net::IOThreadTcpService::Create();
        _service->startWorkerThread(threads_num);

        _listener.WithService(_service)
                .AddSocketProcess(
                        {[](brynet::net::TcpSocket& socket) {
                            socket.setNodelay();
                        }})
                .WithMaxRecvBufferSize(1024)
                .AddEnterCallback([this](const brynet::net::TcpConnection::Ptr& session){
                    std::cout  << typeid(ClientType).name() << "  Ip: " << session->getIP() << " - join" << std::endl;

                    _client_map.emplace(session, std::make_shared<ClientType>(session));

                    session->setDisConnectCallback([this](const brynet::net::TcpConnection::Ptr& session){
                        (void) session;
                        auto iter = _client_map.find(session);
                        if (iter != _client_map.end())
                        {
                            std::cout << typeid(ClientType).name() << "  Ip: " << session->getIP() << " - leave" << std::endl;
                            iter->second->onDisconnect();
                            _client_map.erase(iter);
                        }
                    });
                })
                .WithAddr(false, "0.0.0.0", port)
                .asyncRun();
        return false;
    }

    std::unordered_map<brynet::net::TcpConnection::Ptr, std::shared_ptr<ClientType>> _client_map;

protected:
    brynet::net::IOThreadTcpService::Ptr _service;
    brynet::net::wrapper::ListenerBuilder _listener;
};


#endif //TAPSERVER_RPGSERVER_H
