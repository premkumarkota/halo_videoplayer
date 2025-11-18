#include "include/halo_videoplayer/halo_videoplayer_plugin.h"

#include <flutter_linux/flutter_linux.h>
#include <gtk/gtk.h>
#include <gst/gst.h>
#include <gst/video/video.h>
#include <gst/app/gstappsrc.h>
#include <gst/app/gstappsink.h>

#include <cstring>
#include <map>
#include <memory>
#include <mutex>
#include <thread>

#include "halo_videoplayer_plugin_private.h"

#define HALO_VIDEOPLAYER_PLUGIN(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST((obj), halo_videoplayer_plugin_get_type(), \
                              HaloVideoplayerPlugin))

struct _HaloVideoplayerPlugin {
  GObject parent_instance;
  
  struct VideoPlayer {
    int player_id;
    int64_t texture_id;
    GstElement* pipeline;
    GstElement* appsink;
    GstElement* appsrc;
    GstBus* bus;
    bool is_playing;
    bool is_initialized;
    bool is_looping;
    double volume;
    double playback_speed;
    gint64 duration;
    gint64 position;
    std::thread bus_thread;
    bool should_stop;
    FlTextureRegistrar* texture_registrar;
    FlTexture* texture;
  };
  
  std::map<int, std::unique_ptr<VideoPlayer>> players_;
  int next_player_id_;
  std::mutex players_mutex_;
};

G_DEFINE_TYPE(HaloVideoplayerPlugin, halo_videoplayer_plugin, g_object_get_type())

static gboolean bus_call(GstBus* bus, GstMessage* msg, gpointer data) {
  HaloVideoplayerPlugin::VideoPlayer* player = 
      static_cast<HaloVideoplayerPlugin::VideoPlayer*>(data);
  
  switch (GST_MESSAGE_TYPE(msg)) {
    case GST_MESSAGE_EOS:
      if (player->is_looping) {
        gst_element_seek_simple(player->pipeline, GST_FORMAT_TIME,
                                 GST_SEEK_FLAG_FLUSH, 0);
      }
      break;
    case GST_MESSAGE_ERROR: {
      gchar* debug;
      GError* error;
      gst_message_parse_error(msg, &error, &debug);
      g_free(debug);
      g_error_free(error);
      break;
    }
    default:
      break;
  }
  return TRUE;
}

static void bus_thread_func(HaloVideoplayerPlugin::VideoPlayer* player) {
  while (!player->should_stop && player->bus) {
    GstMessage* msg = gst_bus_timed_pop_filtered(player->bus,
        GST_CLOCK_TIME_NONE,
        static_cast<GstMessageType>(GST_MESSAGE_ERROR | GST_MESSAGE_EOS | 
                                     GST_MESSAGE_STATE_CHANGED));
    if (msg) {
      bus_call(player->bus, msg, player);
      gst_message_unref(msg);
    }
  }
}

static int create_player(HaloVideoplayerPlugin* self, const gchar* source,
                         bool is_network, bool auto_play, bool looping,
                         double volume) {
  std::lock_guard<std::mutex> lock(self->players_mutex_);
  
  int player_id = self->next_player_id_++;
  auto player = std::make_unique<HaloVideoplayerPlugin::VideoPlayer>();
  player->player_id = player_id;
  player->is_playing = false;
  player->is_initialized = false;
  player->is_looping = looping;
  player->volume = volume;
  player->playback_speed = 1.0;
  player->should_stop = false;
  player->duration = 0;
  player->position = 0;
  
  // Create GStreamer pipeline
  GstElement* pipeline = gst_pipeline_new(nullptr);
  GstElement* source_element = nullptr;
  GstElement* decodebin = nullptr;
  GstElement* videoconvert = nullptr;
  GstElement* videoscale = nullptr;
  GstElement* videoflip = nullptr;
  GstElement* appsink = nullptr;
  
  if (is_network) {
    source_element = gst_element_factory_make("uridecodebin", nullptr);
    gchar* uri = g_strdup_printf("%s", source);
    g_object_set(G_OBJECT(source_element), "uri", uri, nullptr);
    g_free(uri);
  } else {
    source_element = gst_element_factory_make("filesrc", nullptr);
    g_object_set(G_OBJECT(source_element), "location", source, nullptr);
    decodebin = gst_element_factory_make("decodebin", nullptr);
  }
  
  videoconvert = gst_element_factory_make("videoconvert", nullptr);
  videoscale = gst_element_factory_make("videoscale", nullptr);
  videoflip = gst_element_factory_make("videoflip", nullptr);
  appsink = gst_element_factory_make("appsink", nullptr);
  
  if (!pipeline || !source_element || !videoconvert || !videoscale || 
      !videoflip || !appsink) {
    g_object_unref(pipeline);
    return -1;
  }
  
  gst_bin_add_many(GST_BIN(pipeline), source_element, videoconvert,
                   videoscale, videoflip, appsink, nullptr);
  
  if (decodebin) {
    gst_bin_add(GST_BIN(pipeline), decodebin);
    if (!gst_element_link(source_element, decodebin)) {
      g_object_unref(pipeline);
      return -1;
    }
    if (!gst_element_link_many(decodebin, videoconvert, videoscale,
                               videoflip, appsink, nullptr)) {
      g_object_unref(pipeline);
      return -1;
    }
  } else {
    // For uridecodebin, we need to connect dynamically
    g_signal_connect(source_element, "pad-added",
                     G_CALLBACK(+[](GstElement* src, GstPad* pad, gpointer data) {
      GstElement* decodebin = GST_ELEMENT(data);
      GstPad* sink_pad = gst_element_get_static_pad(decodebin, "sink");
      if (gst_pad_is_linked(sink_pad)) {
        gst_object_unref(sink_pad);
        return;
      }
      gst_pad_link(pad, sink_pad);
      gst_object_unref(sink_pad);
    }), decodebin);
    
    if (!gst_element_link_many(videoconvert, videoscale, videoflip,
                               appsink, nullptr)) {
      g_object_unref(pipeline);
      return -1;
    }
  }
  
  // Configure appsink
  GstCaps* caps = gst_caps_new_simple("video/x-raw",
                                      "format", G_TYPE_STRING, "RGBA",
                                      nullptr);
  g_object_set(appsink, "caps", caps, "emit-signals", TRUE, nullptr);
  gst_caps_unref(caps);
  
  player->pipeline = pipeline;
  player->appsink = appsink;
  player->appsrc = nullptr;
  player->bus = gst_pipeline_get_bus(GST_PIPELINE(pipeline));
  
  // Start bus watching thread
  player->bus_thread = std::thread(bus_thread_func, player.get());
  
  // Set initial state
  if (auto_play) {
    gst_element_set_state(pipeline, GST_STATE_PLAYING);
    player->is_playing = true;
  } else {
    gst_element_set_state(pipeline, GST_STATE_PAUSED);
  }
  
  player->is_initialized = true;
  
  self->players_[player_id] = std::move(player);
  return player_id;
}

static void dispose_player(HaloVideoplayerPlugin* self, int player_id) {
  std::lock_guard<std::mutex> lock(self->players_mutex_);
  auto it = self->players_.find(player_id);
  if (it != self->players_.end()) {
    auto* player = it->second.get();
    player->should_stop = true;
    if (player->bus_thread.joinable()) {
      player->bus_thread.join();
    }
    if (player->pipeline) {
      gst_element_set_state(player->pipeline, GST_STATE_NULL);
      gst_object_unref(player->pipeline);
    }
    if (player->bus) {
      gst_object_unref(player->bus);
    }
    self->players_.erase(it);
  }
}

static void play_player(HaloVideoplayerPlugin* self, int player_id) {
  std::lock_guard<std::mutex> lock(self->players_mutex_);
  auto it = self->players_.find(player_id);
  if (it != self->players_.end() && it->second) {
    gst_element_set_state(it->second->pipeline, GST_STATE_PLAYING);
    it->second->is_playing = true;
  }
}

static void pause_player(HaloVideoplayerPlugin* self, int player_id) {
  std::lock_guard<std::mutex> lock(self->players_mutex_);
  auto it = self->players_.find(player_id);
  if (it != self->players_.end() && it->second) {
    gst_element_set_state(it->second->pipeline, GST_STATE_PAUSED);
    it->second->is_playing = false;
  }
}

static void seek_player(HaloVideoplayerPlugin* self, int player_id,
                        int64_t position_ms) {
  std::lock_guard<std::mutex> lock(self->players_mutex_);
  auto it = self->players_.find(player_id);
  if (it != self->players_.end() && it->second) {
    gint64 position_ns = position_ms * GST_MSECOND;
    gst_element_seek_simple(it->second->pipeline, GST_FORMAT_TIME,
                            GST_SEEK_FLAG_FLUSH, position_ns);
    it->second->position = position_ns;
  }
}

static void set_volume(HaloVideoplayerPlugin* self, int player_id,
                       double volume) {
  std::lock_guard<std::mutex> lock(self->players_mutex_);
  auto it = self->players_.find(player_id);
  if (it != self->players_.end() && it->second) {
    it->second->volume = volume;
    GstElement* volume_element = gst_bin_get_by_name(
        GST_BIN(it->second->pipeline), "volume");
    if (volume_element) {
      g_object_set(volume_element, "volume", volume, nullptr);
      gst_object_unref(volume_element);
    }
  }
}

static void set_playback_speed(HaloVideoplayerPlugin* self, int player_id,
                               double speed) {
  std::lock_guard<std::mutex> lock(self->players_mutex_);
  auto it = self->players_.find(player_id);
  if (it != self->players_.end() && it->second) {
    it->second->playback_speed = speed;
    // GStreamer speed change would go here
  }
}

static void set_looping(HaloVideoplayerPlugin* self, int player_id,
                        bool looping) {
  std::lock_guard<std::mutex> lock(self->players_mutex_);
  auto it = self->players_.find(player_id);
  if (it != self->players_.end() && it->second) {
    it->second->is_looping = looping;
  }
}

static FlValue* get_player_value(HaloVideoplayerPlugin* self, int player_id) {
  std::lock_guard<std::mutex> lock(self->players_mutex_);
  auto it = self->players_.find(player_id);
  if (it == self->players_.end() || !it->second) {
    FlValue* value = fl_value_new_map();
    fl_value_set_string_take(value, "isInitialized", fl_value_new_bool(FALSE));
    return value;
  }
  
  auto* player = it->second.get();
  FlValue* value = fl_value_new_map();
  fl_value_set_string_take(value, "isInitialized",
                           fl_value_new_bool(player->is_initialized ? TRUE : FALSE));
  fl_value_set_string_take(value, "isPlaying",
                           fl_value_new_bool(player->is_playing ? TRUE : FALSE));
  fl_value_set_string_take(value, "isBuffering", fl_value_new_bool(FALSE));
  fl_value_set_string_take(value, "position",
                           fl_value_new_int(GST_TIME_AS_MSECONDS(player->position)));
  fl_value_set_string_take(value, "duration",
                           fl_value_new_int(GST_TIME_AS_MSECONDS(player->duration)));
  fl_value_set_string_take(value, "aspectRatio", fl_value_new_float(16.0 / 9.0));
  fl_value_set_string_take(value, "volume", fl_value_new_float(player->volume));
  fl_value_set_string_take(value, "playbackSpeed",
                           fl_value_new_float(player->playback_speed));
  
  return value;
}

static int get_texture_id(HaloVideoplayerPlugin* self, int player_id) {
  std::lock_guard<std::mutex> lock(self->players_mutex_);
  auto it = self->players_.find(player_id);
  if (it != self->players_.end() && it->second) {
    return it->second->texture_id;
  }
  return -1;
}

// Called when a method call is received from Flutter.
static void halo_videoplayer_plugin_handle_method_call(
    HaloVideoplayerPlugin* self,
    FlMethodCall* method_call) {
  g_autoptr(FlMethodResponse) response = nullptr;

  const gchar* method = fl_method_call_get_name(method_call);
  FlValue* args = fl_method_call_get_args(method_call);

  if (strcmp(method, "initialize") == 0) {
    FlValue* source_val = fl_value_lookup_string(args, "source");
    FlValue* is_network_val = fl_value_lookup_string(args, "isNetwork");
    FlValue* auto_play_val = fl_value_lookup_string(args, "autoPlay");
    FlValue* looping_val = fl_value_lookup_string(args, "looping");
    FlValue* volume_val = fl_value_lookup_string(args, "volume");
    
    if (!source_val) {
      response = FL_METHOD_RESPONSE(fl_method_error_response_new(
          "InvalidArguments", "Missing source", nullptr));
    } else {
      const gchar* source = fl_value_get_string(source_val);
      bool is_network = is_network_val && fl_value_get_bool(is_network_val);
      bool auto_play = auto_play_val && fl_value_get_bool(auto_play_val);
      bool looping = looping_val && fl_value_get_bool(looping_val);
      double volume = volume_val ? fl_value_get_float(volume_val) : 1.0;
      
      int player_id = create_player(self, source, is_network, auto_play,
                                   looping, volume);
      if (player_id >= 0) {
        g_autoptr(FlValue) result = fl_value_new_int(player_id);
        response = FL_METHOD_RESPONSE(fl_method_success_response_new(result));
      } else {
        response = FL_METHOD_RESPONSE(fl_method_error_response_new(
            "InitializationFailed", "Failed to create player", nullptr));
      }
    }
  } else if (strcmp(method, "dispose") == 0) {
    FlValue* player_id_val = fl_value_lookup_string(args, "playerId");
    if (!player_id_val) {
      response = FL_METHOD_RESPONSE(fl_method_error_response_new(
          "InvalidArguments", "Missing playerId", nullptr));
    } else {
      int player_id = fl_value_get_int(player_id_val);
      dispose_player(self, player_id);
      response = FL_METHOD_RESPONSE(fl_method_success_response_new(nullptr));
    }
  } else if (strcmp(method, "play") == 0) {
    FlValue* player_id_val = fl_value_lookup_string(args, "playerId");
    if (!player_id_val) {
      response = FL_METHOD_RESPONSE(fl_method_error_response_new(
          "InvalidArguments", "Missing playerId", nullptr));
    } else {
      int player_id = fl_value_get_int(player_id_val);
      play_player(self, player_id);
      response = FL_METHOD_RESPONSE(fl_method_success_response_new(nullptr));
    }
  } else if (strcmp(method, "pause") == 0) {
    FlValue* player_id_val = fl_value_lookup_string(args, "playerId");
    if (!player_id_val) {
      response = FL_METHOD_RESPONSE(fl_method_error_response_new(
          "InvalidArguments", "Missing playerId", nullptr));
    } else {
      int player_id = fl_value_get_int(player_id_val);
      pause_player(self, player_id);
      response = FL_METHOD_RESPONSE(fl_method_success_response_new(nullptr));
    }
  } else if (strcmp(method, "seekTo") == 0) {
    FlValue* player_id_val = fl_value_lookup_string(args, "playerId");
    FlValue* position_val = fl_value_lookup_string(args, "position");
    if (!player_id_val || !position_val) {
      response = FL_METHOD_RESPONSE(fl_method_error_response_new(
          "InvalidArguments", "Missing playerId or position", nullptr));
    } else {
      int player_id = fl_value_get_int(player_id_val);
      int64_t position_ms = fl_value_get_int(position_val);
      seek_player(self, player_id, position_ms);
      response = FL_METHOD_RESPONSE(fl_method_success_response_new(nullptr));
    }
  } else if (strcmp(method, "setVolume") == 0) {
    FlValue* player_id_val = fl_value_lookup_string(args, "playerId");
    FlValue* volume_val = fl_value_lookup_string(args, "volume");
    if (!player_id_val || !volume_val) {
      response = FL_METHOD_RESPONSE(fl_method_error_response_new(
          "InvalidArguments", "Missing playerId or volume", nullptr));
    } else {
      int player_id = fl_value_get_int(player_id_val);
      double volume = fl_value_get_float(volume_val);
      set_volume(self, player_id, volume);
      response = FL_METHOD_RESPONSE(fl_method_success_response_new(nullptr));
    }
  } else if (strcmp(method, "setPlaybackSpeed") == 0) {
    FlValue* player_id_val = fl_value_lookup_string(args, "playerId");
    FlValue* speed_val = fl_value_lookup_string(args, "speed");
    if (!player_id_val || !speed_val) {
      response = FL_METHOD_RESPONSE(fl_method_error_response_new(
          "InvalidArguments", "Missing playerId or speed", nullptr));
    } else {
      int player_id = fl_value_get_int(player_id_val);
      double speed = fl_value_get_float(speed_val);
      set_playback_speed(self, player_id, speed);
      response = FL_METHOD_RESPONSE(fl_method_success_response_new(nullptr));
    }
  } else if (strcmp(method, "setLooping") == 0) {
    FlValue* player_id_val = fl_value_lookup_string(args, "playerId");
    FlValue* looping_val = fl_value_lookup_string(args, "looping");
    if (!player_id_val || !looping_val) {
      response = FL_METHOD_RESPONSE(fl_method_error_response_new(
          "InvalidArguments", "Missing playerId or looping", nullptr));
    } else {
      int player_id = fl_value_get_int(player_id_val);
      bool looping = fl_value_get_bool(looping_val);
      set_looping(self, player_id, looping);
      response = FL_METHOD_RESPONSE(fl_method_success_response_new(nullptr));
    }
  } else if (strcmp(method, "getValue") == 0) {
    FlValue* player_id_val = fl_value_lookup_string(args, "playerId");
    if (!player_id_val) {
      response = FL_METHOD_RESPONSE(fl_method_error_response_new(
          "InvalidArguments", "Missing playerId", nullptr));
    } else {
      int player_id = fl_value_get_int(player_id_val);
      g_autoptr(FlValue) value = get_player_value(self, player_id);
      response = FL_METHOD_RESPONSE(fl_method_success_response_new(value));
    }
  } else if (strcmp(method, "getTextureId") == 0) {
    FlValue* player_id_val = fl_value_lookup_string(args, "playerId");
    if (!player_id_val) {
      response = FL_METHOD_RESPONSE(fl_method_error_response_new(
          "InvalidArguments", "Missing playerId", nullptr));
    } else {
      int player_id = fl_value_get_int(player_id_val);
      int texture_id = get_texture_id(self, player_id);
      g_autoptr(FlValue) result = fl_value_new_int(texture_id);
      response = FL_METHOD_RESPONSE(fl_method_success_response_new(result));
    }
  } else {
    response = FL_METHOD_RESPONSE(fl_method_not_implemented_response_new());
  }

  fl_method_call_respond(method_call, response, nullptr);
}

static void halo_videoplayer_plugin_dispose(GObject* object) {
  HaloVideoplayerPlugin* self = HALO_VIDEOPLAYER_PLUGIN(object);
  
  std::lock_guard<std::mutex> lock(self->players_mutex_);
  for (auto& pair : self->players_) {
    dispose_player(self, pair.first);
  }
  self->players_.clear();
  
  G_OBJECT_CLASS(halo_videoplayer_plugin_parent_class)->dispose(object);
}

static void halo_videoplayer_plugin_class_init(HaloVideoplayerPluginClass* klass) {
  G_OBJECT_CLASS(klass)->dispose = halo_videoplayer_plugin_dispose;
}

static void halo_videoplayer_plugin_init(HaloVideoplayerPlugin* self) {
  self->next_player_id_ = 1;
  
  // Initialize GStreamer
  gst_init(nullptr, nullptr);
}

static void method_call_cb(FlMethodChannel* channel, FlMethodCall* method_call,
                           gpointer user_data) {
  HaloVideoplayerPlugin* plugin = HALO_VIDEOPLAYER_PLUGIN(user_data);
  halo_videoplayer_plugin_handle_method_call(plugin, method_call);
}

void halo_videoplayer_plugin_register_with_registrar(FlPluginRegistrar* registrar) {
  HaloVideoplayerPlugin* plugin = HALO_VIDEOPLAYER_PLUGIN(
      g_object_new(halo_videoplayer_plugin_get_type(), nullptr));

  g_autoptr(FlStandardMethodCodec) codec = fl_standard_method_codec_new();
  g_autoptr(FlMethodChannel) channel =
      fl_method_channel_new(fl_plugin_registrar_get_messenger(registrar),
                            "halo_videoplayer",
                            FL_METHOD_CODEC(codec));
  fl_method_channel_set_method_call_handler(channel, method_call_cb,
                                            g_object_ref(plugin),
                                            g_object_unref);

  g_object_unref(plugin);
}
