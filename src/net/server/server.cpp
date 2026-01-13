#include "server.hpp"

#include <cstddef>
#include <cstdint>

#include <spdlog/spdlog.h>
#include <boost/asio/ssl/context.hpp>
#include <boost/system/detail/error_code.hpp>
#include <boost/asio/strand.hpp>

#include "protocol/opcodes.hpp"
#include "protocol/packet_head.hpp"
#include "protocol/server_packet.hpp"
#include "session/session.hpp"
#include "socket/ws_socket.hpp"

namespace ep::net
{
  Server::Server(boost::asio::io_context& ioc, 
                 ssl::context& ctx, 
                 std::shared_ptr<NetworkSubsystem> net_susbsystem, 
                 std::shared_ptr<GameSubsystem> game_subsystem) noexcept :
    ioc_{ioc},
    ctx_{ctx},
    acceptor_{ioc},
    new_session_id_{0},
    net_susbsystem_(net_susbsystem),
    game_susbsystem_(game_subsystem)
  {}

  bool Server::StartListen(tcp::endpoint endpoint) noexcept
  {
    boost::system::error_code ec;

    // Open the acceptor.
    ec = acceptor_.open(endpoint.protocol(), ec);
    if (ec) {
      spdlog::error("open: {}", ec.what());
      return false;
    }

    // Allow address reuse.
    ec = acceptor_.set_option(boost::asio::socket_base::reuse_address(true), ec);
    if (ec) {
      spdlog::error("set_option: {}", ec.what());
      return false;
    }

    // Bind to the server address.
    ec = acceptor_.bind(endpoint, ec);
    if (ec) {
      spdlog::error("bind: {}", ec.what());
      return false;
    }

    // Start listening for connections.
    ec = acceptor_.listen(boost::asio::socket_base::max_listen_connections, ec);
    if (ec) {
      spdlog::error("listen: {}", ec.what());
      return false;
    }

    return true;
  }

  void Server::Run()
  {
    acceptor_.async_accept(
      net::make_strand(ioc_),
      [this](const boost::system::error_code& ec, tcp::socket socket)
      {
        if (!ec) {
          // auto wsssocket = std::make_shared<WSSSocket>(std::move(socket), ctx_);
          auto wssocket = std::make_unique<WSSocket>(std::move(socket));
          std::size_t id = new_session_id_.fetch_add(1);
          auto session = std::make_shared<Session>(shared_from_this(), std::move(wssocket), id);
          session->Run();
        } else {
          spdlog::error("async_accept: {}", ec.what());
        }

        Run();
      }
    );
  }

  void Server::AddSession(std::shared_ptr<Session> session) noexcept
  {
    spdlog::info("Server::AddSession");
    {
      std::lock_guard lock(sessions_mutex_);
      sessions_[session->GetID()] = session;
    }

    // Push event to game
    PacketHead packet{
      1,
      opcode_to_uint16(Opcodes::OPCODE_CREATE_PLAYER),
      packet_type_to_uint8(PacketType::PACKET_TYPE_EVENT),
    };
    game_susbsystem_->in_queue_.Push({ResponseType::Incoming, session->GetID(), packet});
  }
 
  void Server::Sender()
  {
    spdlog::info("Server::Sender");
    for (;;) {
      auto packet = game_susbsystem_->out_queue_.WaitAndPop();
      std::uint32_t buf_size = packet.GetPacketHead().size_;
      std::shared_ptr<std::uint8_t[]> buf{packet.SerializePacket()};

      switch (packet.GetResponseType()) {
        case ResponseType::Rpc:
          sessions_[packet.GetID()]->PushToSend({buf_size, std::move(buf)});
          break;
        case ResponseType::Broadcast:
          for (const auto& elem: sessions_) {
            elem.second->PushToSend({buf_size, buf});
          }
          break;
        case ResponseType::RpcOthers:
          for (const auto& elem: sessions_) {
            if (elem.first != packet.GetID()) {
              elem.second->PushToSend({buf_size, buf});
            }
          }
          break;
        default:
          spdlog::error("unknown packet type");
      }
    }
  }

  void Server::PushPacketToGame(ServerPacket<PacketHead> packet)
  {
    game_susbsystem_->in_queue_.Push(packet);
  }

  void Server::CloseSession(std::size_t id)
  {
    std::lock_guard lock(sessions_mutex_);
    if (sessions_.find(id) != sessions_.end()) {
      sessions_.erase(id);
  
      // Push event to game
      PacketHead packet{
        1,
        opcode_to_uint16(Opcodes::OPCODE_REMOVE_PLAYER),
        packet_type_to_uint8(PacketType::PACKET_TYPE_EVENT),
      };

      game_susbsystem_->in_queue_.Push({ResponseType::Incoming, id, packet});
    } else {
      spdlog::error("Server::CloseSession errror id: {}", id);
    }
  }
}
