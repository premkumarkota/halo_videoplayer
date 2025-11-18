#include "halo_videoplayer_plugin.h"

#include <windows.h>
#include <flutter/method_channel.h>
#include <flutter/plugin_registrar_windows.h>
#include <flutter/standard_method_codec.h>
#include <flutter/texture_registrar.h>

#include <memory>
#include <map>
#include <string>
#include <thread>
#include <mutex>

// MediaKit includes - using Windows Media Foundation as fallback
#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <mferror.h>
#include <d3d11.h>
#include <wrl/client.h>

#pragma comment(lib, "mf.lib")
#pragma comment(lib, "mfplat.lib")
#pragma comment(lib, "mfreadwrite.lib")
#pragma comment(lib, "mfuuid.lib")
#pragma comment(lib, "d3d11.lib")

using Microsoft::WRL::ComPtr;

namespace halo_videoplayer {

struct VideoPlayer {
  int playerId;
  int textureId;
  ComPtr<IMFSourceReader> reader;
  ComPtr<ID3D11Device> d3dDevice;
  ComPtr<ID3D11DeviceContext> d3dContext;
  ComPtr<ID3D11Texture2D> texture;
  bool isPlaying;
  bool isInitialized;
  bool isLooping;
  double volume;
  double playbackSpeed;
  LONGLONG duration;
  LONGLONG currentPosition;
  std::thread updateThread;
  bool shouldStop;
  flutter::TextureRegistrar* textureRegistrar;
  std::unique_ptr<flutter::TextureVariant> textureVariant;
};

class HaloVideoplayerPlugin : public flutter::Plugin {
 public:
  static void RegisterWithRegistrar(flutter::PluginRegistrarWindows *registrar);

  HaloVideoplayerPlugin(flutter::TextureRegistrar* texture_registrar);

  virtual ~HaloVideoplayerPlugin();

  void HandleMethodCall(
      const flutter::MethodCall<flutter::EncodableValue> &method_call,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result);

 private:
  flutter::TextureRegistrar* texture_registrar_;
  std::map<int, std::unique_ptr<VideoPlayer>> players_;
  int next_player_id_ = 1;
  std::mutex players_mutex_;

  void InitializeMediaFoundation();
  int CreatePlayer(const std::string& source, bool is_network, bool auto_play, bool looping, double volume);
  void DisposePlayer(int player_id);
  void PlayPlayer(int player_id);
  void PausePlayer(int player_id);
  void SeekPlayer(int player_id, int64_t position_ms);
  void SetVolume(int player_id, double volume);
  void SetPlaybackSpeed(int player_id, double speed);
  void SetLooping(int player_id, bool looping);
  flutter::EncodableValue GetPlayerValue(int player_id);
  int GetTextureId(int player_id);
};

// Static
void HaloVideoplayerPlugin::RegisterWithRegistrar(
    flutter::PluginRegistrarWindows *registrar) {
  auto channel =
      std::make_unique<flutter::MethodChannel<flutter::EncodableValue>>(
          registrar->messenger(), "halo_videoplayer",
          &flutter::StandardMethodCodec::GetInstance());

  auto plugin = std::make_unique<HaloVideoplayerPlugin>(
      registrar->texture_registrar());

  channel->SetMethodCallHandler(
      [plugin_pointer = plugin.get()](const auto &call, auto result) {
        plugin_pointer->HandleMethodCall(call, std::move(result));
      });

  registrar->AddPlugin(std::move(plugin));
}

HaloVideoplayerPlugin::HaloVideoplayerPlugin(
    flutter::TextureRegistrar* texture_registrar)
    : texture_registrar_(texture_registrar) {
  InitializeMediaFoundation();
}

HaloVideoplayerPlugin::~HaloVideoplayerPlugin() {
  std::lock_guard<std::mutex> lock(players_mutex_);
  for (auto& pair : players_) {
    if (pair.second) {
      pair.second->shouldStop = true;
      if (pair.second->updateThread.joinable()) {
        pair.second->updateThread.join();
      }
    }
  }
  players_.clear();
  MFShutdown();
}

void HaloVideoplayerPlugin::InitializeMediaFoundation() {
  HRESULT hr = MFStartup(MF_VERSION);
  if (FAILED(hr)) {
    // Log error
  }
}

void HaloVideoplayerPlugin::HandleMethodCall(
    const flutter::MethodCall<flutter::EncodableValue> &method_call,
    std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
  const auto* args = std::get_if<flutter::EncodableMap>(method_call.arguments());

  if (method_call.method_name().compare("initialize") == 0) {
    if (!args) {
      result->Error("InvalidArguments", "Missing arguments");
      return;
    }

    auto source_it = args->find(flutter::EncodableValue("source"));
    auto is_network_it = args->find(flutter::EncodableValue("isNetwork"));
    auto auto_play_it = args->find(flutter::EncodableValue("autoPlay"));
    auto looping_it = args->find(flutter::EncodableValue("looping"));
    auto volume_it = args->find(flutter::EncodableValue("volume"));

    if (source_it == args->end()) {
      result->Error("InvalidArguments", "Missing source");
      return;
    }

    std::string source = std::get<std::string>(source_it->second);
    bool is_network = is_network_it != args->end() && 
                      std::get<bool>(is_network_it->second);
    bool auto_play = auto_play_it != args->end() && 
                     std::get<bool>(auto_play_it->second);
    bool looping = looping_it != args->end() && 
                   std::get<bool>(looping_it->second);
    double volume = volume_it != args->end() ? 
                    std::get<double>(volume_it->second) : 1.0;

    try {
      int player_id = CreatePlayer(source, is_network, auto_play, looping, volume);
      result->Success(flutter::EncodableValue(player_id));
    } catch (const std::exception& e) {
      result->Error("InitializationFailed", e.what());
    }
  } else if (method_call.method_name().compare("dispose") == 0) {
    if (!args) {
      result->Error("InvalidArguments", "Missing arguments");
      return;
    }
    auto player_id_it = args->find(flutter::EncodableValue("playerId"));
    if (player_id_it == args->end()) {
      result->Error("InvalidArguments", "Missing playerId");
      return;
    }
    int player_id = std::get<int>(player_id_it->second);
    DisposePlayer(player_id);
    result->Success();
  } else if (method_call.method_name().compare("play") == 0) {
    if (!args) {
      result->Error("InvalidArguments", "Missing arguments");
      return;
    }
    auto player_id_it = args->find(flutter::EncodableValue("playerId"));
    if (player_id_it == args->end()) {
      result->Error("InvalidArguments", "Missing playerId");
      return;
    }
    int player_id = std::get<int>(player_id_it->second);
    PlayPlayer(player_id);
    result->Success();
  } else if (method_call.method_name().compare("pause") == 0) {
    if (!args) {
      result->Error("InvalidArguments", "Missing arguments");
      return;
    }
    auto player_id_it = args->find(flutter::EncodableValue("playerId"));
    if (player_id_it == args->end()) {
      result->Error("InvalidArguments", "Missing playerId");
      return;
    }
    int player_id = std::get<int>(player_id_it->second);
    PausePlayer(player_id);
    result->Success();
  } else if (method_call.method_name().compare("seekTo") == 0) {
    if (!args) {
      result->Error("InvalidArguments", "Missing arguments");
      return;
    }
    auto player_id_it = args->find(flutter::EncodableValue("playerId"));
    auto position_it = args->find(flutter::EncodableValue("position"));
    if (player_id_it == args->end() || position_it == args->end()) {
      result->Error("InvalidArguments", "Missing playerId or position");
      return;
    }
    int player_id = std::get<int>(player_id_it->second);
    int64_t position_ms = std::get<int64_t>(position_it->second);
    SeekPlayer(player_id, position_ms);
    result->Success();
  } else if (method_call.method_name().compare("setVolume") == 0) {
    if (!args) {
      result->Error("InvalidArguments", "Missing arguments");
      return;
    }
    auto player_id_it = args->find(flutter::EncodableValue("playerId"));
    auto volume_it = args->find(flutter::EncodableValue("volume"));
    if (player_id_it == args->end() || volume_it == args->end()) {
      result->Error("InvalidArguments", "Missing playerId or volume");
      return;
    }
    int player_id = std::get<int>(player_id_it->second);
    double volume = std::get<double>(volume_it->second);
    SetVolume(player_id, volume);
    result->Success();
  } else if (method_call.method_name().compare("setPlaybackSpeed") == 0) {
    if (!args) {
      result->Error("InvalidArguments", "Missing arguments");
      return;
    }
    auto player_id_it = args->find(flutter::EncodableValue("playerId"));
    auto speed_it = args->find(flutter::EncodableValue("speed"));
    if (player_id_it == args->end() || speed_it == args->end()) {
      result->Error("InvalidArguments", "Missing playerId or speed");
      return;
    }
    int player_id = std::get<int>(player_id_it->second);
    double speed = std::get<double>(speed_it->second);
    SetPlaybackSpeed(player_id, speed);
    result->Success();
  } else if (method_call.method_name().compare("setLooping") == 0) {
    if (!args) {
      result->Error("InvalidArguments", "Missing arguments");
      return;
    }
    auto player_id_it = args->find(flutter::EncodableValue("playerId"));
    auto looping_it = args->find(flutter::EncodableValue("looping"));
    if (player_id_it == args->end() || looping_it == args->end()) {
      result->Error("InvalidArguments", "Missing playerId or looping");
      return;
    }
    int player_id = std::get<int>(player_id_it->second);
    bool looping = std::get<bool>(looping_it->second);
    SetLooping(player_id, looping);
    result->Success();
  } else if (method_call.method_name().compare("getValue") == 0) {
    if (!args) {
      result->Error("InvalidArguments", "Missing arguments");
      return;
    }
    auto player_id_it = args->find(flutter::EncodableValue("playerId"));
    if (player_id_it == args->end()) {
      result->Error("InvalidArguments", "Missing playerId");
      return;
    }
    int player_id = std::get<int>(player_id_it->second);
    auto value = GetPlayerValue(player_id);
    result->Success(value);
  } else if (method_call.method_name().compare("getTextureId") == 0) {
    if (!args) {
      result->Error("InvalidArguments", "Missing arguments");
      return;
    }
    auto player_id_it = args->find(flutter::EncodableValue("playerId"));
    if (player_id_it == args->end()) {
      result->Error("InvalidArguments", "Missing playerId");
      return;
    }
    int player_id = std::get<int>(player_id_it->second);
    int texture_id = GetTextureId(player_id);
    result->Success(flutter::EncodableValue(texture_id));
  } else {
    result->NotImplemented();
  }
}

int HaloVideoplayerPlugin::CreatePlayer(const std::string& source, bool is_network, bool auto_play, bool looping, double volume) {
  // Simplified implementation - in production, use MediaKit or Windows Media Foundation
  // For now, return a placeholder that indicates native implementation is needed
  std::lock_guard<std::mutex> lock(players_mutex_);
  int player_id = next_player_id_++;
  
  // Note: Full MediaKit/Media Foundation implementation would go here
  // This is a placeholder structure
  auto player = std::make_unique<VideoPlayer>();
  player->playerId = player_id;
  player->isPlaying = false;
  player->isInitialized = false;
  player->isLooping = looping;
  player->volume = volume;
  player->playbackSpeed = 1.0;
  player->shouldStop = false;
  player->textureRegistrar = texture_registrar_;
  
  players_[player_id] = std::move(player);
  return player_id;
}

void HaloVideoplayerPlugin::DisposePlayer(int player_id) {
  std::lock_guard<std::mutex> lock(players_mutex_);
  auto it = players_.find(player_id);
  if (it != players_.end()) {
    if (it->second) {
      it->second->shouldStop = true;
      if (it->second->updateThread.joinable()) {
        it->second->updateThread.join();
      }
      if (it->second->textureId >= 0) {
        texture_registrar_->UnregisterTexture(it->second->textureId);
      }
    }
    players_.erase(it);
  }
}

void HaloVideoplayerPlugin::PlayPlayer(int player_id) {
  std::lock_guard<std::mutex> lock(players_mutex_);
  auto it = players_.find(player_id);
  if (it != players_.end() && it->second) {
    it->second->isPlaying = true;
  }
}

void HaloVideoplayerPlugin::PausePlayer(int player_id) {
  std::lock_guard<std::mutex> lock(players_mutex_);
  auto it = players_.find(player_id);
  if (it != players_.end() && it->second) {
    it->second->isPlaying = false;
  }
}

void HaloVideoplayerPlugin::SeekPlayer(int player_id, int64_t position_ms) {
  std::lock_guard<std::mutex> lock(players_mutex_);
  auto it = players_.find(player_id);
  if (it != players_.end() && it->second) {
    it->second->currentPosition = position_ms * 10000; // Convert to 100-nanosecond units
  }
}

void HaloVideoplayerPlugin::SetVolume(int player_id, double volume) {
  std::lock_guard<std::mutex> lock(players_mutex_);
  auto it = players_.find(player_id);
  if (it != players_.end() && it->second) {
    it->second->volume = volume;
  }
}

void HaloVideoplayerPlugin::SetPlaybackSpeed(int player_id, double speed) {
  std::lock_guard<std::mutex> lock(players_mutex_);
  auto it = players_.find(player_id);
  if (it != players_.end() && it->second) {
    it->second->playbackSpeed = speed;
  }
}

void HaloVideoplayerPlugin::SetLooping(int player_id, bool looping) {
  std::lock_guard<std::mutex> lock(players_mutex_);
  auto it = players_.find(player_id);
  if (it != players_.end() && it->second) {
    it->second->isLooping = looping;
  }
}

flutter::EncodableValue HaloVideoplayerPlugin::GetPlayerValue(int player_id) {
  std::lock_guard<std::mutex> lock(players_mutex_);
  auto it = players_.find(player_id);
  if (it == players_.end() || !it->second) {
    flutter::EncodableMap value;
    value[flutter::EncodableValue("isInitialized")] = flutter::EncodableValue(false);
    return flutter::EncodableValue(value);
  }

  auto* player = it->second.get();
  flutter::EncodableMap value;
  value[flutter::EncodableValue("isInitialized")] = flutter::EncodableValue(player->isInitialized);
  value[flutter::EncodableValue("isPlaying")] = flutter::EncodableValue(player->isPlaying);
  value[flutter::EncodableValue("isBuffering")] = flutter::EncodableValue(false);
  value[flutter::EncodableValue("position")] = flutter::EncodableValue(static_cast<int64_t>(player->currentPosition / 10000));
  value[flutter::EncodableValue("duration")] = flutter::EncodableValue(static_cast<int64_t>(player->duration / 10000));
  value[flutter::EncodableValue("aspectRatio")] = flutter::EncodableValue(16.0 / 9.0); // Default
  value[flutter::EncodableValue("volume")] = flutter::EncodableValue(player->volume);
  value[flutter::EncodableValue("playbackSpeed")] = flutter::EncodableValue(player->playbackSpeed);
  
  return flutter::EncodableValue(value);
}

int HaloVideoplayerPlugin::GetTextureId(int player_id) {
  std::lock_guard<std::mutex> lock(players_mutex_);
  auto it = players_.find(player_id);
  if (it != players_.end() && it->second) {
    return it->second->textureId;
  }
  return -1;
}

}  // namespace halo_videoplayer
