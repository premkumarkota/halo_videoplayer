#ifndef FLUTTER_PLUGIN_HALO_VIDEOPLAYER_PLUGIN_H_
#define FLUTTER_PLUGIN_HALO_VIDEOPLAYER_PLUGIN_H_

#include <flutter/method_channel.h>
#include <flutter/plugin_registrar_windows.h>

#include <memory>

namespace halo_videoplayer {

class HaloVideoplayerPlugin : public flutter::Plugin {
 public:
  static void RegisterWithRegistrar(flutter::PluginRegistrarWindows *registrar);

  HaloVideoplayerPlugin();

  virtual ~HaloVideoplayerPlugin();

  // Disallow copy and assign.
  HaloVideoplayerPlugin(const HaloVideoplayerPlugin&) = delete;
  HaloVideoplayerPlugin& operator=(const HaloVideoplayerPlugin&) = delete;

  // Called when a method is called on this plugin's channel from Dart.
  void HandleMethodCall(
      const flutter::MethodCall<flutter::EncodableValue> &method_call,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result);
};

}  // namespace halo_videoplayer

#endif  // FLUTTER_PLUGIN_HALO_VIDEOPLAYER_PLUGIN_H_
