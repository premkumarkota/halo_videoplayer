export 'halo_videoplayer_platform_interface.dart';
export 'src/halo_videoplayer_windows.dart' if (dart.library.html) '';
export 'src/halo_videoplayer_linux.dart' if (dart.library.html) '';

import 'dart:async';
import 'package:flutter/material.dart';
import 'package:video_player/video_player.dart';

import 'halo_videoplayer_platform_interface.dart';
import 'halo_videoplayer_method_channel.dart';

/// A unified video player controller that works across all platforms
class HaloVideoPlayerController extends ValueNotifier<HaloVideoPlayerValue> {
  /// The data source for the video
  final HaloVideoPlayerDataSource dataSource;

  /// Whether the video should loop
  bool _looping = false;

  /// Whether the video should auto-play
  final bool _autoPlay;

  /// The volume of the video (0.0 to 1.0)
  double _volume = 1.0;

  /// The playback speed
  double _playbackSpeed = 1.0;

  /// The player ID
  int? _playerId;

  /// The texture ID for rendering
  int? _textureId;

  /// Internal VideoPlayerController for video_player platforms
  VideoPlayerController? _videoPlayerController;

  /// Stream subscription for value updates
  StreamSubscription<HaloVideoPlayerValue>? _valueSubscription;

  /// Timer for periodic position updates
  Timer? _positionUpdateTimer;

  /// Whether the controller has been disposed
  bool _isDisposed = false;

  HaloVideoPlayerController({
    required this.dataSource,
    bool autoPlay = false,
    bool looping = false,
    double volume = 1.0,
  }) : _autoPlay = autoPlay,
       _looping = looping,
       _volume = volume,
       super(const HaloVideoPlayerValue());

  /// Initialize the video player
  Future<void> initialize() async {
    if (_isDisposed) {
      throw StateError('Controller has been disposed');
    }

    try {
      _playerId = await HaloVideoplayerPlatform.instance.initialize(
        dataSource: dataSource,
        autoPlay: _autoPlay,
        looping: _looping,
        volume: _volume,
      );

      // Get texture ID for rendering
      _textureId = await HaloVideoplayerPlatform.instance.getTextureId(
        _playerId!,
      );

      // For video_player platforms, get the VideoPlayerController
      if (HaloVideoplayerPlatform.instance is MethodChannelHaloVideoplayer) {
        final methodChannelImpl =
            HaloVideoplayerPlatform.instance as MethodChannelHaloVideoplayer;
        _videoPlayerController = methodChannelImpl.getVideoPlayerController(
          _playerId!,
        );
      }

      // Start listening to value updates
      _valueSubscription = HaloVideoplayerPlatform.instance
          .getValueStream(_textureId!)
          .listen((newValue) {
            if (!_isDisposed) {
              super.value = newValue;
            }
          });

      // Start periodic position updates
      _positionUpdateTimer = Timer.periodic(const Duration(milliseconds: 100), (
        _,
      ) async {
        if (!_isDisposed && _textureId != null) {
          try {
            final currentValue = await HaloVideoplayerPlatform.instance
                .getValue(_textureId!);
            if (!_isDisposed) {
              super.value = currentValue;
            }
          } catch (e) {
            // Ignore errors during periodic updates
          }
        }
      });

      // Get initial value
      if (_textureId != null) {
        final initialValue = await HaloVideoplayerPlatform.instance.getValue(
          _textureId!,
        );
        super.value = initialValue;
      }
    } catch (e) {
      super.value = value.copyWith(errorDescription: e.toString());
      rethrow;
    }
  }

  /// Play the video
  Future<void> play() async {
    if (_textureId == null) {
      throw StateError('Video player not initialized');
    }
    await HaloVideoplayerPlatform.instance.play(_textureId!);
  }

  /// Pause the video
  Future<void> pause() async {
    if (_textureId == null) {
      throw StateError('Video player not initialized');
    }
    await HaloVideoplayerPlatform.instance.pause(_textureId!);
  }

  /// Seek to a specific position
  Future<void> seekTo(Duration position) async {
    if (_textureId == null) {
      throw StateError('Video player not initialized');
    }
    await HaloVideoplayerPlatform.instance.seekTo(_textureId!, position);
  }

  /// Set volume (0.0 to 1.0)
  Future<void> setVolume(double volume) async {
    if (volume < 0.0 || volume > 1.0) {
      throw ArgumentError('Volume must be between 0.0 and 1.0');
    }
    _volume = volume;
    if (_textureId != null) {
      await HaloVideoplayerPlatform.instance.setVolume(_textureId!, volume);
    }
  }

  /// Set playback speed
  Future<void> setPlaybackSpeed(double speed) async {
    if (speed <= 0.0) {
      throw ArgumentError('Playback speed must be greater than 0');
    }
    _playbackSpeed = speed;
    if (_textureId != null) {
      await HaloVideoplayerPlatform.instance.setPlaybackSpeed(
        _textureId!,
        speed,
      );
    }
  }

  /// Set looping
  Future<void> setLooping(bool looping) async {
    _looping = looping;
    if (_textureId != null) {
      await HaloVideoplayerPlatform.instance.setLooping(_textureId!, looping);
    }
  }

  /// Get the current volume
  double get volume => _volume;

  /// Get the current playback speed
  double get playbackSpeed => _playbackSpeed;

  /// Get whether the video is looping
  bool get looping => _looping;

  /// Get the texture ID for rendering
  int? get textureId => _textureId;

  @override
  Future<void> dispose() async {
    if (_isDisposed) {
      return;
    }
    _isDisposed = true;

    _positionUpdateTimer?.cancel();
    _positionUpdateTimer = null;

    await _valueSubscription?.cancel();
    _valueSubscription = null;

    if (_playerId != null) {
      try {
        await HaloVideoplayerPlatform.instance.dispose(_playerId!);
      } catch (e) {
        // Ignore disposal errors
      }
    }

    super.dispose();
  }
}

/// Widget that displays a video player
class HaloVideoPlayer extends StatelessWidget {
  final HaloVideoPlayerController controller;
  final BoxFit fit;

  const HaloVideoPlayer({
    super.key,
    required this.controller,
    this.fit = BoxFit.contain,
  });

  @override
  Widget build(BuildContext context) {
    if (controller.textureId == null) {
      return const SizedBox.shrink();
    }

    return ValueListenableBuilder<HaloVideoPlayerValue>(
      valueListenable: controller,
      builder: (context, value, child) {
        if (!value.isInitialized) {
          return const Center(child: CircularProgressIndicator());
        }

        if (value.errorDescription != null) {
          return Center(child: Text('Error: ${value.errorDescription}'));
        }

        // For video_player based platforms, use VideoPlayer widget directly
        // For native platforms (Windows/Linux), use Texture widget
        if (controller._videoPlayerController != null) {
          // Use video_player's VideoPlayer widget for Android/iOS/macOS/Web
          return VideoPlayer(controller._videoPlayerController!);
        } else {
          // Use Texture widget for Windows/Linux native implementations
          return Texture(
            textureId: controller.textureId!,
            filterQuality: FilterQuality.medium,
          );
        }
      },
    );
  }
}
