import 'dart:async';
import 'package:flutter/services.dart';
import '../halo_videoplayer_platform_interface.dart';

/// Linux-specific implementation using GStreamer via native code
class HaloVideoplayerLinux extends HaloVideoplayerPlatform {
  final MethodChannel _channel = const MethodChannel('halo_videoplayer');

  final Map<int, StreamController<HaloVideoPlayerValue>> _valueControllers = {};
  final Map<int, int> _textureIdToPlayerId = {};
  int _nextPlayerId = 1;

  HaloVideoplayerLinux() {
    _channel.setMethodCallHandler(_handleMethodCall);
  }

  Future<dynamic> _handleMethodCall(MethodCall call) async {
    switch (call.method) {
      case 'onVideoValueUpdate':
        final playerId = call.arguments['playerId'] as int;
        final valueMap = call.arguments['value'] as Map;
        final valueController = _valueControllers[playerId];
        if (valueController != null && !valueController.isClosed) {
          valueController.add(_parseValue(valueMap));
        }
        break;
      default:
        return null;
    }
  }

  HaloVideoPlayerValue _parseValue(Map<dynamic, dynamic> map) {
    return HaloVideoPlayerValue(
      isInitialized: map['isInitialized'] as bool? ?? false,
      isPlaying: map['isPlaying'] as bool? ?? false,
      isBuffering: map['isBuffering'] as bool? ?? false,
      position: Duration(milliseconds: map['position'] as int? ?? 0),
      duration: Duration(milliseconds: map['duration'] as int? ?? 0),
      aspectRatio: (map['aspectRatio'] as num?)?.toDouble() ?? 1.0,
      volume: (map['volume'] as num?)?.toDouble() ?? 1.0,
      playbackSpeed: (map['playbackSpeed'] as num?)?.toDouble() ?? 1.0,
      errorDescription: map['errorDescription'] as String?,
    );
  }

  @override
  Future<int> initialize({
    required HaloVideoPlayerDataSource dataSource,
    bool autoPlay = false,
    bool looping = false,
    double volume = 1.0,
  }) async {
    final playerId = _nextPlayerId++;
    try {
      await _channel.invokeMethod('initialize', {
        'playerId': playerId,
        'source': dataSource.source,
        'isNetwork': dataSource.isNetwork,
        'isAsset': dataSource.isAsset,
        'httpHeaders': dataSource.httpHeaders,
        'autoPlay': autoPlay,
        'looping': looping,
        'volume': volume,
      });

      _valueControllers[playerId] =
          StreamController<HaloVideoPlayerValue>.broadcast();

      return playerId;
    } catch (e) {
      _valueControllers.remove(playerId)?.close();
      rethrow;
    }
  }

  @override
  Future<void> dispose(int textureId) async {
    final playerId = _textureIdToPlayerId[textureId];
    if (playerId == null) return;

    await _channel.invokeMethod('dispose', {'playerId': playerId});
    _valueControllers.remove(playerId)?.close();
    _textureIdToPlayerId.remove(textureId);
  }

  @override
  Future<void> play(int textureId) async {
    final playerId = _textureIdToPlayerId[textureId];
    if (playerId != null) {
      await _channel.invokeMethod('play', {'playerId': playerId});
    }
  }

  @override
  Future<void> pause(int textureId) async {
    final playerId = _textureIdToPlayerId[textureId];
    if (playerId != null) {
      await _channel.invokeMethod('pause', {'playerId': playerId});
    }
  }

  @override
  Future<void> seekTo(int textureId, Duration position) async {
    final playerId = _textureIdToPlayerId[textureId];
    if (playerId != null) {
      await _channel.invokeMethod('seekTo', {
        'playerId': playerId,
        'position': position.inMilliseconds,
      });
    }
  }

  @override
  Future<void> setVolume(int textureId, double volume) async {
    final playerId = _textureIdToPlayerId[textureId];
    if (playerId != null) {
      await _channel.invokeMethod('setVolume', {
        'playerId': playerId,
        'volume': volume,
      });
    }
  }

  @override
  Future<void> setPlaybackSpeed(int textureId, double speed) async {
    final playerId = _textureIdToPlayerId[textureId];
    if (playerId != null) {
      await _channel.invokeMethod('setPlaybackSpeed', {
        'playerId': playerId,
        'speed': speed,
      });
    }
  }

  @override
  Future<void> setLooping(int textureId, bool looping) async {
    final playerId = _textureIdToPlayerId[textureId];
    if (playerId != null) {
      await _channel.invokeMethod('setLooping', {
        'playerId': playerId,
        'looping': looping,
      });
    }
  }

  @override
  Future<HaloVideoPlayerValue> getValue(int textureId) async {
    final playerId = _textureIdToPlayerId[textureId];
    if (playerId != null) {
      final result = await _channel.invokeMethod('getValue', {
        'playerId': playerId,
      });
      return _parseValue(result as Map);
    }
    return const HaloVideoPlayerValue();
  }

  @override
  Future<int?> getTextureId(int playerId) async {
    final result = await _channel.invokeMethod('getTextureId', {
      'playerId': playerId,
    });
    final textureId = result as int?;
    if (textureId != null) {
      _textureIdToPlayerId[textureId] = playerId;
    }
    return textureId;
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
}
