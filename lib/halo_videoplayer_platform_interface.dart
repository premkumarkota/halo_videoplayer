import 'dart:async';
import 'package:flutter/foundation.dart' show kIsWeb;
import 'package:plugin_platform_interface/plugin_platform_interface.dart';
import 'dart:io' show Platform;

import 'halo_videoplayer_method_channel.dart';

/// Video player value with playback state information
class HaloVideoPlayerValue {
  final bool isInitialized;
  final bool isPlaying;
  final bool isBuffering;
  final Duration position;
  final Duration duration;
  final double aspectRatio;
  final double volume;
  final double playbackSpeed;
  final String? errorDescription;

  const HaloVideoPlayerValue({
    this.isInitialized = false,
    this.isPlaying = false,
    this.isBuffering = false,
    this.position = Duration.zero,
    this.duration = Duration.zero,
    this.aspectRatio = 1.0,
    this.volume = 1.0,
    this.playbackSpeed = 1.0,
    this.errorDescription,
  });

  HaloVideoPlayerValue copyWith({
    bool? isInitialized,
    bool? isPlaying,
    bool? isBuffering,
    Duration? position,
    Duration? duration,
    double? aspectRatio,
    double? volume,
    double? playbackSpeed,
    String? errorDescription,
  }) {
    return HaloVideoPlayerValue(
      isInitialized: isInitialized ?? this.isInitialized,
      isPlaying: isPlaying ?? this.isPlaying,
      isBuffering: isBuffering ?? this.isBuffering,
      position: position ?? this.position,
      duration: duration ?? this.duration,
      aspectRatio: aspectRatio ?? this.aspectRatio,
      volume: volume ?? this.volume,
      playbackSpeed: playbackSpeed ?? this.playbackSpeed,
      errorDescription: errorDescription ?? this.errorDescription,
    );
  }
}

/// Video player data source
class HaloVideoPlayerDataSource {
  final String source;
  final bool isNetwork;
  final bool isAsset;
  final Map<String, String>? httpHeaders;

  const HaloVideoPlayerDataSource({
    required this.source,
    this.isNetwork = false,
    this.isAsset = false,
    this.httpHeaders,
  });

  factory HaloVideoPlayerDataSource.network(
    String url, {
    Map<String, String>? httpHeaders,
  }) {
    return HaloVideoPlayerDataSource(
      source: url,
      isNetwork: true,
      httpHeaders: httpHeaders,
    );
  }

  factory HaloVideoPlayerDataSource.asset(String path) {
    return HaloVideoPlayerDataSource(source: path, isAsset: true);
  }

  factory HaloVideoPlayerDataSource.file(String path) {
    return HaloVideoPlayerDataSource(source: path);
  }
}

abstract class HaloVideoplayerPlatform extends PlatformInterface {
  /// Constructs a HaloVideoplayerPlatform.
  HaloVideoplayerPlatform() : super(token: _token);

  static final Object _token = Object();

  static HaloVideoplayerPlatform _instance = _createPlatformInstance();

  /// Create the appropriate platform instance
  static HaloVideoplayerPlatform _createPlatformInstance() {
    if (kIsWeb) {
      return MethodChannelHaloVideoplayer();
    }

    if (Platform.isWindows) {
      // Use Windows-specific implementation
      try {
        return MethodChannelHaloVideoplayer(); // Will use native Windows implementation
      } catch (e) {
        return MethodChannelHaloVideoplayer();
      }
    }

    if (Platform.isLinux) {
      // Use Linux-specific implementation
      try {
        return MethodChannelHaloVideoplayer(); // Will use native Linux implementation
      } catch (e) {
        return MethodChannelHaloVideoplayer();
      }
    }

    // Android, iOS, macOS use video_player
    return MethodChannelHaloVideoplayer();
  }

  /// The default instance of [HaloVideoplayerPlatform] to use.
  ///
  /// Defaults to platform-specific implementation.
  static HaloVideoplayerPlatform get instance => _instance;

  /// Platform-specific implementations should set this with their own
  /// platform-specific class that extends [HaloVideoplayerPlatform] when
  /// they register themselves.
  static set instance(HaloVideoplayerPlatform instance) {
    PlatformInterface.verifyToken(instance, _token);
    _instance = instance;
  }

  /// Initialize a video player with the given data source
  Future<int> initialize({
    required HaloVideoPlayerDataSource dataSource,
    bool autoPlay = false,
    bool looping = false,
    double volume = 1.0,
  }) {
    throw UnimplementedError('initialize() has not been implemented.');
  }

  /// Dispose a video player
  Future<void> dispose(int textureId) {
    throw UnimplementedError('dispose() has not been implemented.');
  }

  /// Play the video
  Future<void> play(int textureId) {
    throw UnimplementedError('play() has not been implemented.');
  }

  /// Pause the video
  Future<void> pause(int textureId) {
    throw UnimplementedError('pause() has not been implemented.');
  }

  /// Seek to a specific position
  Future<void> seekTo(int textureId, Duration position) {
    throw UnimplementedError('seekTo() has not been implemented.');
  }

  /// Set volume (0.0 to 1.0)
  Future<void> setVolume(int textureId, double volume) {
    throw UnimplementedError('setVolume() has not been implemented.');
  }

  /// Set playback speed
  Future<void> setPlaybackSpeed(int textureId, double speed) {
    throw UnimplementedError('setPlaybackSpeed() has not been implemented.');
  }

  /// Set looping
  Future<void> setLooping(int textureId, bool looping) {
    throw UnimplementedError('setLooping() has not been implemented.');
  }

  /// Get current player value
  Future<HaloVideoPlayerValue> getValue(int textureId) {
    throw UnimplementedError('getValue() has not been implemented.');
  }

  /// Get video texture ID for rendering
  Future<int?> getTextureId(int playerId) {
    throw UnimplementedError('getTextureId() has not been implemented.');
  }

  /// Stream of player value updates
  Stream<HaloVideoPlayerValue> getValueStream(int textureId) {
    throw UnimplementedError('getValueStream() has not been implemented.');
  }
}
