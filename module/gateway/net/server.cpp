#include "net/server.h"

#include "net/connection.h"

namespace million {
namespace net {

Server::Server(IMillion* imillion)
    : imillion_(imillion) {}
Server::~Server() = default;

void Server::Start(uint16_t port) {
    // ��ȡһ��io_context����acceptor
    auto& io_context = imillion_->NextIoContext();
    asio::co_spawn(io_context, Listen(port), asio::detached);
}

void Server::Stop() {
    {
        auto guard = std::lock_guard(connections_mutex_);
        for (auto& connection : connections_) {
            connection->Close();
        }
    }
}

void Server::RemoveConnection(std::list<std::shared_ptr<Connection>>::iterator iter) {
    auto guard = std::lock_guard(connections_mutex_);
    connections_.erase(iter);
}

ConnectionHandle Server::AddConnection(asio::ip::tcp::socket&& socket, const asio::any_io_executor& executor) {
    decltype(connections_)::iterator iter;
    auto connection = std::make_shared<Connection>(this, std::move(socket), executor);
    auto handle = ConnectionHandle(connection);
    {
        auto guard = std::lock_guard(connections_mutex_);
        connections_.emplace_back(std::move(connection));
        iter = --connections_.end();
    }
    handle.connection().set_iter(iter);
    return handle;
}

asio::awaitable<void> Server::Listen(uint16_t port) {
    auto executor = co_await asio::this_coro::executor;
    asio::ip::tcp::acceptor acceptor(executor, { asio::ip::tcp::v4(), port });
    while (true) {
        asio::ip::tcp::socket socket = co_await acceptor.async_accept(asio::use_awaitable);
        // ��ȡio_context�������Ӱ󶨵���io_context��
        auto& io_context = imillion_->NextIoContext();
        auto handle = AddConnection(std::move(socket), io_context.get_executor());
        if (on_connection_) {
            on_connection_(handle);
        }
        handle.connection().Process();
    }
}

} // namespace net
} // namespace million