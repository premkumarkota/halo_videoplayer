import 'dart:async';
import 'dart:io';
import 'package:flutter/foundation.dart';
import 'package:flutter/services.dart';
import 'package:video_player/video_player.dart';

import 'halo_videoplayer_platform_interface.dart';

/// An implementation of [HaloVideoplayerPlatform] that uses video_player for
/// Android, iOS, macOS, and Web platforms.
class MethodChannelHaloVideoplayer extends HaloVideoplayerPlatform {
  /// The method channel used to interact with the native platform.
  @visibleForTesting
  final methodChannel = const MethodChannel('halo_videoplayer');

  /// Map of player IDs to VideoPlayerController instances
  final Map<int, VideoPlayerController> _controllers = {};

  /// Map of texture IDs to player IDs (for video_player, textureId == playerId)
  final Map<int, int> _textureIdToPlayerId = {};

  /// Map of player IDs to VideoPlayerController (for accessing in widget)
  final Map<int, VideoPlayerController> _playerIdToController = {};

  /// Map of player IDs to value controllers
  final Map<int, StreamController<HaloVideoPlayerValue>> _valueControllers = {};

  /// Next available player ID
  int _nextPlayerId = 1;

  MethodChannelHaloVideoplayer() {
    // Listen to platform messages for value updates
    methodChannel.setMethodCallHandler(_handleMethodCall);
  }

  Future<dynamic> _handleMethodCall(MethodCall call) async {
    // Handle platform-specific callbacks if needed
    return null;
  }

  @override
  Future<int> initialize({
    required HaloVideoPlayerDataSource dataSource,
    bool autoPlay = false,
    bool looping = false,
    double volume = 1.0,
  }) async {
    final playerId = _nextPlayerId++;
    VideoPlayerController controller;

    try {
      if (dataSource.isNetwork) {
        controller = VideoPlayerController.networkUrl(
          Uri.parse(dataSource.source),
          httpHeaders: dataSource.httpHeaders ?? {},
        );
      } else if (dataSource.isAsset) {
        controller = VideoPlayerController.asset(dataSource.source);
      } else {
        // File path
        if (kIsWeb) {
          throw UnsupportedError(
            'File-based video playback is not supported on web',
          );
        }
        controller = VideoPlayerController.file(File(dataSource.source));
      }

      await controller.initialize();
      controller.setLooping(looping);
      controller.setVolume(volume);
      if (autoPlay) {
        await controller.play();
      }

      _controllers[playerId] = controller;
      _playerIdToController[playerId] = controller;
      _valueControllers[playerId] =
          StreamController<HaloVideoPlayerValue>.broadcast();

      // Listen to controller updates
      controller.addListener(() {
        final valueController = _valueControllers[playerId];
        if (valueController != null && !valueController.isClosed) {
          valueController.add(_convertVideoPlayerValue(controller.value));
        }
      });

      return playerId;
    } catch (e) {
      // Clean up on error
      _controllers.remove(playerId);
      _playerIdToController.remove(playerId);
      _valueControllers.remove(playerId)?.close();
      rethrow;
    }
  }

  /// Get VideoPlayerController for a player ID (for internal use)
  VideoPlayerController? getVideoPlayerController(int playerId) {
    return _playerIdToController[playerId];
  }

  @override
  Future<void> dispose(int textureId) async {
    final playerId = _textureIdToPlayerId[textureId];
    if (playerId == null) return;

    final controller = _controllers.remove(playerId);
    if (controller != null) {
      await controller.dispose();
    }

    _valueControllers.remove(playerId)?.close();
    _textureIdToPlayerId.remove(textureId);
  }

  @override
  Future<void> play(int textureId) async {
    final playerId = _textureIdToPlayerId[textureId];
    final controller = _controllers[playerId];
    if (controller != null) {
      await controller.play();
    }
  }

  @override
  Future<void> pause(int textureId) async {
    final playerId = _textureIdToPlayerId[textureId];
    final controller = _controllers[playerId];
    if (controller != null) {
      await controller.pause();
    }
  }

  @override
  Future<void> seekTo(int textureId, Duration position) async {
    final playerId = _textureIdToPlayerId[textureId];
    final controller = _controllers[playerId];
    if (controller != null) {
      await controller.seekTo(position);
    }
  }

  @override
  Future<void> setVolume(int textureId, double volume) async {
    final playerId = _textureIdToPlayerId[textureId];
    final controller = _controllers[playerId];
    if (controller != null) {
      await controller.setVolume(volume);
    }
  }

  @override
  Future<void> setPlaybackSpeed(int textureId, double speed) async {
    final playerId = _textureIdToPlayerId[textureId];
    final controller = _controllers[playerId];
    if (controller != null) {
      await controller.setPlaybackSpeed(speed);
    }
  }

  @override
  Future<void> setLooping(int textureId, bool looping) async {
    final playerId = _textureIdToPlayerId[textureId];
    final controller = _controllers[playerId];
    if (controller != null) {
      await controller.setLooping(looping);
    }
  }

  @override
  Future<HaloVideoPlayerValue> getValue(int textureId) async {
    final playerId = _textureIdToPlayerId[textureId];
    final controller = _controllers[playerId];
    if (controller != null) {
      return _convertVideoPlayerValue(controller.value);
    }
    return const HaloVideoPlayerValue();
  }

  @override
  Future<int?> getTextureId(int playerId) async {
    final controller = _controllers[playerId];
    if (controller != null && controller.value.isInitialized) {
      // For video_player, we use the player ID as texture ID
      // The VideoPlayer widget will handle the actual texture rendering
      final textureId = playerId; // Use player ID as texture ID
      _textureIdToPlayerId[textureId] = playerId;
      return textureId;
    }
    return null;
  }

  @override
  Stream<HaloVideoPlayerValue> getValueStream(int textureId) {
    final playerId = _textureIdToPlayerId[textureId];
    final valueController = _valueControllers[playerId];
    if (valueController != null) {
      return valueController.stream;
    }
    return const Stream<HaloVideoPlayerValue>.empty();
  }

  /// Convert VideoPlayerValue to HaloVideoPlayerValue
  HaloVideoPlayerValue _convertVideoPlayerValue(VideoPlayerValue videoValue) {
    return HaloVideoPlayerValue(
      isInitialized: videoValue.isInitialized,
      isPlaying: videoValue.isPlaying,
      isBuffering: videoValue.isBuffering,
      position: videoValue.position,
      duration: videoValue.duration,
      aspectRatio: videoValue.aspectRatio,
      volume: videoValue.volume,
      playbackSpeed: videoValue.playbackSpeed,
      errorDescription: videoValue.errorDescription,
    );
  }
}
