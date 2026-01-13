#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <atomic>

#include "socket/i_socket.hpp"
#include "utils/ts_queue.hpp"
#include "config/constants.hpp"
#include "protocol/packet_head.hpp"
#include "socket/send_buffer.hpp"

namespace ep::net
{
  class Server;

  class Session : public std::enable_shared_from_this<Session> {
  public:

    enum class State : std::uint8_t {
      Connecting,
      Connected,
      Disconnecting,
      Disconnected,
      User,
    };

    enum class ReadState : std::uint8_t {
      ReadHeadPacket,
      ReadAuthPacket,
      ReadUnknown,
    };

    explicit Session(std::shared_ptr<Server> server, std::unique_ptr<ISocket> socket, std::size_t id);
    ~Session() = default;

    Session(const Session&) = delete;
    Session& operator=(const Session&) = delete;

    void Run();
    std::size_t GetID() const { return id_; }
    void PushToSend(SendBuffer packet);
  private:

    // Main funciton processing session input state
    void ProcessState();

    void AsyncRead();

    void OnRead();
    void OnReadHeadPacket();
    void OnReadAuthPacket();
    void OnReadUnknown();

    void Send();

    // Session state
    void SetConnecting() const noexcept { return state_.store(State::Connecting, std::memory_order_release); }
    void SetConnected() const noexcept { return state_.store(State::Connected, std::memory_order_release); }
    void SetDisconnecting() const noexcept { return state_.store(State::Disconnecting, std::memory_order_release); }
    void SetDisconnected() const noexcept { return state_.store(State::Disconnected, std::memory_order_release); }
    void SetUser() const noexcept { return state_.store(State::User, std::memory_order_release); }

    [[nodiscard]] State GetState() const noexcept { return state_.load(std::memory_order_acquire); }

    [[nodiscard]] bool IsConnected() const noexcept { return state_.load(std::memory_order_acquire) != State::Disconnected; }

    // Sending state
    [[maybe_unused]] bool StartSending() const noexcept { return sending_.test_and_set(std::memory_order_acq_rel); }
    void StopSending() const noexcept { return sending_.clear(std::memory_order_release); }

    std::shared_ptr<Server> server_;
    std::unique_ptr<ISocket> socket_;
    std::size_t id_;

    // Read packets
    std::size_t read_size_;
    std::array<std::uint8_t, TCP_READ_BUFFER> read_buffer_;
    ReadState read_state_;
    std::optional<PacketHead> packet_head_;

    // Flag indicate session sending state: sending/not
    mutable std::atomic_flag sending_;

    // Flag indicate session state: connecting/connected/disconnecting/disconnected/user etc...
    mutable std::atomic<State> state_;  

    // PacketHandler packet_handler_;
    TSQueue<SendBuffer> out_queue_;
  };
}
