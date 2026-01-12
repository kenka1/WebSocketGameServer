#include "world.hpp"

#include <chrono>
#include <cmath>
#include <cstdint>
#include <memory>
#include <thread>

#include <spdlog/spdlog.h>
#include "protocol/game_packet.hpp"
#include "protocol/packet_head.hpp"
#include "protocol/server_packet.hpp"
#include "spdlog/common.h"

// #include "protocol/net_packet.hpp"
#include "protocol/opcodes.hpp"
#include "tile/tile.hpp"
#include "utils/ts_queue.hpp"
#include "player/player.hpp"

namespace ep::game
{
  World::World(std::shared_ptr<NetworkSubsystem> net_subsystem, 
               std::shared_ptr<GameSubsystem> game_subsystem, 
               const GameConfig& config) :
    net_subsystem_(net_subsystem),
    game_subsystem_(game_subsystem),
    config_(config)
  {
    // Initialzie map
    map_.resize(config_.grid_x_ * config_.grid_y_);
    for (std::size_t y = 0; y < config_.grid_y_; y++) {
      for (std::size_t x = 0; x < config_.grid_x_; x++) {
        std::size_t index = y * config_.grid_x_ + x;
        // Create tile
        if (config_.map_[index] != 0) {
          map_[index] = Tile{static_cast<float>(x) * config_.tile_, 
                             static_cast<float>(y) * config_.tile_, 
                             config_.tile_, 
                             config_.tile_,
                             TileType::Solid};
        }
      }
    }
  }

  void World::GameLoop()
  {
    const double tick_seconds = 1.0 / config_.tick_rate_;
    auto tick_duration = std::chrono::duration_cast<std::chrono::steady_clock::duration>(
      std::chrono::duration<double>(tick_seconds));

    auto last = std::chrono::steady_clock::now();

    for (;;) {
      auto current = std::chrono::steady_clock::now();
      // Time stamp
      tick_ = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();

      std::chrono::duration<double> delta = current - last;
      last = current;


      // spdlog::info("delta = {}, actual Hz = {}", delta.count(), 1.0 / delta.count());

      Tick(delta.count());

      std::this_thread::sleep_until(current + tick_duration);
    }
  }

  void World::Tick(double dt)
  {
    // Double buffering incoming queue.
    TSQueue<ServerPacket<PacketHead>> tick_queue{std::move(game_subsystem_->in_queue_)};

    // FIX Reset x velocity
    for (auto& item : players_)
      item.second->SetVel(0.0, item.second->GetVelY());

    // Handle packets
    while (!tick_queue.Empty()) {
      auto packet = tick_queue.TryPop();
      ProcessInput(packet.value(), dt);
    }

    // Update physics
    for (auto& item : players_)
      Update(*item.second, dt);
  }

  void World::ProcessInput(ServerPacket<PacketHead>& packet, double dt)
  {
    spdlog::info("(World::ProcessInput)");
    // FIX check valid player
    auto& player = players_[packet.GetID()];
    auto& head = packet.GetPacketHead();
    std::uint16_t opcode = head.opcode_;

    // TODO move to config / constants
    double speed = 100.0 * dt;
    double jump_force = 400.0 * dt;

    switch (to_opcode(opcode)) {
      case Opcodes::OPCODE_CREATE_PLAYER:
      {
        // TODO change 100(hp) to CONSTANT
        auto player  = std::make_unique<Player>(
          packet.GetID(), 100,
          config_.player_.player_start_x_ * config_.tile_ + config_.player_.player_offset_, 
          config_.player_.player_start_y_ * config_.tile_ + config_.player_.player_offset_,
          0.0, 0.0,
          config_.player_.width_,
          config_.player_.height_
        );
        AddPlayer(std::move(player));
        break;
      }
      case Opcodes::OPCODE_REMOVE_PLAYER:
        RemovePlayer(*player);
        break;
      case Opcodes::OPCODE_LEFT_MOVE:
        player->SetVel(-speed, player->GetVelY());
        break;
      case Opcodes::OPCODE_RIGHT_MOVE:
        player->SetVel(speed, player->GetVelY());
        break;
      case Opcodes::OPCODE_JUMP:
        if (player->OnGround()) {
          player->SetVel(player->GetVelX(), -jump_force);
          player->SetOnGround(false);
        }
        break;
      case Opcodes::OPCODE_LEFT_JUMP:
        if (player->OnGround()) {
          player->SetVel(-speed, -jump_force);
          player->SetOnGround(false);
        } else {
          player->SetVel(-speed, player->GetVelY());
        }
        break;
      case Opcodes::OPCODE_RIGHT_JUMP:
        if (player->OnGround()) {
          player->SetVel(speed, -jump_force);
          player->SetOnGround(false);
        } else {
          player->SetVel(speed, player->GetVelY());
        }
        break;
      default:
        spdlog::warn("Unknown opcode: {}", opcode);
    }
  }

  void World::Update(IPlayer& player, double dt)
  {
    spdlog::info("(World::Update)");
    const double g = 9.8;
    double vel_y = player.GetVelY() + g * dt;
    player.SetVel(player.GetVelX(), vel_y);
    MovePlayer(player);

    spdlog::info("make move packet");

    // TODO change 1(protocol version) to CONSTANT
    PacketHead head{
      1, 
      opcode_to_uint16(Opcodes::OPCODE_PLAYER_STATE),
      packet_type_to_uint8(PacketType::PACKET_TYPE_GAME),
      PACKET_HEAD_SIZE + GAME_PACKET_SIZE,
      tick_,
      player.GetAndIncrementSequenceID()
    };

    GamePacket body{
      player.GetHP(),
      player.GetX(),
      player.GetY()
    };

    game_subsystem_->out_queue_.Push({ResponseType::Broadcast, player.GetID(), head, body});
  }

  void World::MovePlayer(IPlayer& player)
  {
    spdlog::info("(World::MovePlayer)");
    float vel_x = player.GetVelX();
    float vel_y = player.GetVelY();

    /* ------ X Axis ------*/
    if (vel_x != 0 ) {
      spdlog::info("============== X AXIS ==============");
      // calculate collision along x axis
      SweptData swept = collision_.SweptAxis(player, 
                                             config_.tile_, config_.grid_x_, config_.grid_y_,
                                             map_,
                                             vel_x, 0.0);
    
      vel_x *= swept.entry_time_;
      player.Move(vel_x, 0.0);
      if (swept.hit_) {
        player.SetVel(0.0, vel_y);
      // spdlog::info("HIT SIDE WALL\n"\
      //              "x: {} y: {}", player.GetX(), player.GetY());
      }
      spdlog::info("============== X AXIS ==============");
    }

    /* ------ Y Axis ------*/
    if (vel_y != 0 ) {
      spdlog::info("============== Y AXIS ==============");
      // calculate collision along y axis
      SweptData swept = collision_.SweptAxis(
        player, 
        config_.tile_, config_.grid_x_, config_.grid_y_,
        map_,
        0.0, vel_y
      );
    
      vel_y *= swept.entry_time_;
      player.Move(0.0, vel_y);
      player.SetOnGround(false);
      if (swept.hit_) {
        if (vel_y >= 0.0) {
          player.SetOnGround(true);
          // spdlog::info("HIT GROUND");
        } else {
          // spdlog::info("HIT WALL");
        }
        player.SetVel(vel_x, 0.0);
      }
      spdlog::info("============== Y AXIS ==============");
    }
  }

  void World::AddPlayer(std::unique_ptr<IPlayer> player)
  {
    spdlog::info("(World::AddPlayer)");
    {
      std::lock_guard lock(players_mutex_);
      // TODO check if this id already exists
      // Otherwise it overwrite previous player
      players_[player->GetID()] = std::move(player);
    }

    // Create player on client side
    // TODO change 1(protocol version) to CONSTANT
    PacketHead create_player_head{
      1, 
      opcode_to_uint16(Opcodes::OPCODE_CREATE_PLAYER),
      packet_type_to_uint8(PacketType::PACKET_TYPE_GAME),
      PACKET_HEAD_SIZE + GAME_PACKET_SIZE,
      tick_,
      player->GetAndIncrementSequenceID()
    };

    GamePacket create_player_body{
      player->GetHP(),
      player->GetX(),
      player->GetY()
    };

    game_subsystem_->out_queue_.Push({ResponseType::Rpc, player->GetID(), create_player_head, create_player_body});

    // Send all players to new player
    PacketHead spawn_players_head{
      1, 
      opcode_to_uint16(Opcodes::OPCODE_SPAWN_PLAYERS),
      packet_type_to_uint8(PacketType::PACKET_TYPE_GAME),
      PACKET_HEAD_SIZE + GAME_PACKET_SIZE,
      tick_,
      player->GetAndIncrementSequenceID()
    };

    for (const auto& elem : players_) {
      if (elem.first != player->GetID()) {
        GamePacket spawn_players_body{
          player->GetHP(),
          player->GetX(),
          player->GetY()
        };
        game_subsystem_->out_queue_.Push({ResponseType::Rpc, player->GetID(), spawn_players_head, spawn_players_body});
      }
    }
    
    // Notify others
    PacketHead add_player_head{
      1, 
      opcode_to_uint16(Opcodes::OPCODE_ADD_PLAYER),
      packet_type_to_uint8(PacketType::PACKET_TYPE_GAME),
      PACKET_HEAD_SIZE + GAME_PACKET_SIZE,
      tick_,
      player->GetAndIncrementSequenceID()
    };

    game_subsystem_->out_queue_.Push({ResponseType::RpcOthers, player->GetID(), add_player_head, create_player_body});
  }

  void World::RemovePlayer(IPlayer& player)
  {
    spdlog::info("(World::RemovePlayer)");

    PacketHead head{
      1, 
      opcode_to_uint16(Opcodes::OPCODE_REMOVE_PLAYER),
      packet_type_to_uint8(PacketType::PACKET_TYPE_GAME),
      PACKET_HEAD_SIZE + GAME_PACKET_SIZE,
      tick_,
      player.GetID()
    };

    GamePacket body{
      player.GetHP(),
      player.GetX(),
      player.GetY()
    };

    game_subsystem_->out_queue_.Push({ResponseType::RpcOthers, player.GetID(), head, body});

    std::lock_guard lock(players_mutex_);
    // TODO check if this id is exists
    players_.erase(player.GetID());
  }

  std::size_t World::PlayerNumbers() const
  {
    std::lock_guard lock(players_mutex_);
    return players_.size(); 
  }
}
