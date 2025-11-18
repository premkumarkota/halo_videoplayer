#ifndef FLUTTER_PLUGIN_HALO_VIDEOPLAYER_PLUGIN_H_
#define FLUTTER_PLUGIN_HALO_VIDEOPLAYER_PLUGIN_H_

#include <flutter/method_channel.h>
#include <flutter/plugin_registrar_windows.h>
#include <flutter/texture_registrar.h>

#include <memory>
#include <map>
#include <mutex>
#include <string>

namespace halo_videoplayer {

// Forward declaration
struct VideoPlayer;

class HaloVideoplayerPlugin : public flutter::Plugin {
 public:
  static void RegisterWithRegistrar(flutter::PluginRegistrarWindows *registrar);

  HaloVideoplayerPlugin(flutter::TextureRegistrar* texture_registrar);

  virtual ~HaloVideoplayerPlugin();

  // Disallow copy and assign.
  HaloVideoplayerPlugin(const HaloVideoplayerPlugin&) = delete;
  HaloVideoplayerPlugin& operator=(const HaloVideoplayerPlugin&) = delete;

  // Called when a method is called on this plugin's channel from Dart.
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

}  // namespace halo_videoplayer

#endif  // FLUTTER_PLUGIN_HALO_VIDEOPLAYER_PLUGIN_H_
