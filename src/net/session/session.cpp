#include "session.hpp"

#include <atomic>
#include <boost/asio/ssl/error.hpp>
#include <sys/types.h>

#include <boost/asio/ssl/stream_base.hpp>
#include <boost/asio/buffer.hpp>
#include <boost/beast/core/error.hpp>
#include <boost/beast/websocket/ssl.hpp>
#include <boost/beast/websocket/error.hpp>
#include <spdlog/spdlog.h>

#include "config/constants.hpp"
#include "protocol/opcodes.hpp"
#include "protocol/server_packet.hpp"
#include "server/server.hpp"
#include "aliases/beast_aliases.hpp"
#include "protocol/packet_head.hpp"

namespace ep::net
{
  Session::Session(std::shared_ptr<Server> server, std::unique_ptr<ISocket> socket, std::size_t id) :
    server_(server),
    socket_(std::move(socket)),
    id_(id),
    read_size_(0),
    state_(State::Connecting),
    sending_(ATOMIC_FLAG_INIT)
  {}

  void Session::Run()
  {
    spdlog::info("Session::Accept");
    // TODO set timeout
    // TODO set decorator

    auto self = shared_from_this();
    socket_->async_accept(
      [self](const beast::error_code &ec)
      {
        // an error occured
        if (ec) {
          // client close connection
          if (ec == websocket::error::closed)
            spdlog::warn("WebSocket was closed cleanly");
          else
            spdlog::error("Accept error: {}", ec.what());

          self->SetDisconnecting();
        } else {
          // Finally connected
          self->SetConnected();
        }

        self->ProcessState();

      }
    );
  }

  void Session::ProcessState()
  {
    switch (GetState()) {
      case State::Connecting:
        break;
      case State::Connected:
        // Add client to server and game
        server_->AddSession(shared_from_this());

        // Start reading
        AsyncRead();
        break;
      case State::Disconnecting:
        socket_->close();
        server_->CloseSession(id_);
        SetDisconnected();
        break;
      case State::Disconnected:
        break;
      case State::User:
        break;
      default:
        spdlog::error("Unknown session state");
    }
  }

  void Session::AsyncRead()
  {
    spdlog::info("Session::AsyncRead");
    auto self = shared_from_this();
    socket_->async_read_some(
      read_buffer_.data() + read_size_,
      TCP_READ_BUFFER - read_size_,
      [self](const beast::error_code& ec, std::size_t size)
      {
        // An error occured
        if (ec) {
          // Client close connection
          if (ec == websocket::error::closed)
            spdlog::warn("WebSocket was closed cleanly");
          else
            spdlog::error("Read header error: {}", ec.what());

          // Close session process
          self->SetDisconnecting();
          self->ProcessState();
          return;
        }

        // Update read size
        self->read_size_ += size;

        // Next read step
        self->OnRead();
      }
    );
  }

  void Session::OnRead()
  {
    switch (read_state_) {
      case ReadState::ReadHeadPacket:
        OnReadHeadPacket();
        break;
      case ReadState::ReadAuthPacket:
        OnReadAuthPacket();
        break;
      case ReadState::ReadUnknown:
        OnReadUnknown();
        break;
    }
  }

  void Session::OnReadHeadPacket()
  {
    // Deserialize packet
    packet_head_ = ParsePacketHead(read_buffer_.data(), read_size_);

    // Continue reading
    if (!packet_head_)
      AsyncRead();

    // Update read size
    read_size_ -= PACKET_HEAD_SIZE; // TODO packet->size()

    // Update read state
    switch (to_packet_type(packet_head_.value().type_)) {
      case PacketType::PACKET_TYPE_AUTH:
        read_state_ = ReadState::ReadAuthPacket;
        break;
      case PacketType::PACKET_TYPE_GAME_INPUT:
        // Push packet to game incoming queue
        server_->PushPacketToGame({ResponseType::Incoming, id_, packet_head_.value()});
        break;
      default:
        read_state_ = ReadState::ReadUnknown;
    }
    
    OnRead();
  }

  void Session::OnReadAuthPacket()
  {
    spdlog::warn("Session::OnReadAuthPacket feature is not implemented");
  }

  void Session::OnReadUnknown()
  {
    spdlog::warn("Session::OnReadUnknown feature is not implemented");
  }

  void Session::PushToSend(SendBuffer packet)
  {
    out_queue_.Push(packet); 
    Send();
  }

  void Session::Send()
  {
    // Check if session is disconnected
    if (!IsConnected())
      return spdlog::info("Return from send operation, client is disconneted");

    // Check if send queue is empty
    if (out_queue_.Empty())
      return spdlog::info("Return from send, queue is empty");

    // Check if session is not sending
    if (StartSending())
      return spdlog::info("Return from send operation, previous send is not finished");

    auto packet = out_queue_.TryPop();
    // This check should never pass
    if (!packet)
      return spdlog::error("buffer is nullopt:\nfile: {} line: {}", __FILE__, __LINE__);

    auto buf = packet->buf_;
    auto size = packet->size_;

    auto self = shared_from_this();
    socket_->async_write(
      buf.get(),
      size,
      [self, buf](const beast::error_code& ec, std::size_t size)
      {
        // an error occured
        if (ec) {
          // client close connection
          if (ec == websocket::error::closed)
            spdlog::warn("WebSocket was closed cleanly");
          else
            spdlog::error("Write error: {}", ec.what());

          // Close session process
          self->SetDisconnecting();
          self->ProcessState();
          return;
        }

        self->StopSending();
        // If out queue is not empty send again
        if (!self->out_queue_.Empty())
          self->Send();
      }
    );
  }
}
