#include "include/halo_videoplayer/halo_videoplayer_plugin_c_api.h"

#include <flutter/plugin_registrar_windows.h>

#include "halo_videoplayer_plugin.h"

void HaloVideoplayerPluginCApiRegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar) {
  halo_videoplayer::HaloVideoplayerPlugin::RegisterWithRegistrar(
      flutter::PluginRegistrarManager::GetInstance()
          ->GetRegistrar<flutter::PluginRegistrarWindows>(registrar));
}
