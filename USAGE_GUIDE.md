# Halo Video Player - End User Guide

## How End Users Use This Plugin

This guide explains how to use the `halo_videoplayer` plugin in your Flutter application.

---

## Step 1: Add the Plugin to Your Project

In your `pubspec.yaml` file, add the dependency:

```yaml
dependencies:
  flutter:
    sdk: flutter
  halo_videoplayer:
    path: ../halo_videoplayer  # If using locally
    # OR
    # halo_videoplayer: ^0.0.1  # If published to pub.dev
```

Then run:
```bash
flutter pub get
```

---

## Step 2: Import the Plugin

In your Dart file where you want to use the video player:

```dart
import 'package:halo_videoplayer/halo_videoplayer.dart';
```

---

## Step 3: Basic Usage Example

Here's a complete example showing how to use the plugin:

```dart
import 'package:flutter/material.dart';
import 'package:halo_videoplayer/halo_videoplayer.dart';

void main() {
  runApp(const MyApp());
}

class MyApp extends StatelessWidget {
  const MyApp({super.key});

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: 'Video Player Demo',
      home: VideoPlayerScreen(),
    );
  }
}

class VideoPlayerScreen extends StatefulWidget {
  @override
  _VideoPlayerScreenState createState() => _VideoPlayerScreenState();
}

class _VideoPlayerScreenState extends State<VideoPlayerScreen> {
  // Create a controller for the video player
  late HaloVideoPlayerController _controller;
  bool _isInitialized = false;

  @override
  void initState() {
    super.initState();
    
    // Initialize the controller with a video source
    _controller = HaloVideoPlayerController(
      // Option 1: Network video (from URL)
      dataSource: HaloVideoPlayerDataSource.network(
        'https://flutter.github.io/assets-for-api-docs/assets/videos/bee.mp4',
      ),
      
      // Option 2: Local file
      // dataSource: HaloVideoPlayerDataSource.file('/path/to/video.mp4'),
      
      // Option 3: Asset video (from assets folder)
      // dataSource: HaloVideoPlayerDataSource.asset('videos/sample.mp4'),
      
      autoPlay: false,  // Don't auto-play when initialized
      looping: false,    // Don't loop the video
      volume: 1.0,       // Full volume (0.0 to 1.0)
    );
    
    // Initialize the video player
    _controller.initialize().then((_) {
      setState(() {
        _isInitialized = true;
      });
    }).catchError((error) {
      print('Error initializing video: $error');
    });
  }

  @override
  void dispose() {
    // Always dispose the controller when done
    _controller.dispose();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: const Text('Video Player'),
      ),
      body: Column(
        children: [
          // Display the video player
          if (_isInitialized)
            Expanded(
              child: AspectRatio(
                aspectRatio: _controller.value.aspectRatio,
                child: HaloVideoPlayer(controller: _controller),
              ),
            )
          else
            const Expanded(
              child: Center(child: CircularProgressIndicator()),
            ),
          
          // Simple play/pause button
          Padding(
            padding: const EdgeInsets.all(16.0),
            child: FloatingActionButton(
              onPressed: () {
                if (_controller.value.isPlaying) {
                  _controller.pause();
                } else {
                  _controller.play();
                }
                setState(() {}); // Update UI
              },
              child: Icon(
                _controller.value.isPlaying 
                    ? Icons.pause 
                    : Icons.play_arrow,
              ),
            ),
          ),
        ],
      ),
    );
  }
}
```

---

## Step 4: Understanding the Key Components

### 1. **HaloVideoPlayerController**
This is the main controller that manages video playback. You create it with:
- `dataSource`: Where the video comes from (network, file, or asset)
- `autoPlay`: Whether to start playing automatically
- `looping`: Whether to loop the video
- `volume`: Initial volume (0.0 to 1.0)

### 2. **HaloVideoPlayerDataSource**
This specifies the video source:

```dart
// Network video (from internet)
HaloVideoPlayerDataSource.network(
  'https://example.com/video.mp4',
  httpHeaders: {'Authorization': 'Bearer token'}, // Optional headers
)

// Local file on device
HaloVideoPlayerDataSource.file('/storage/emulated/0/Download/video.mp4')

// Asset bundled with your app
HaloVideoPlayerDataSource.asset('videos/my_video.mp4')
```

### 3. **HaloVideoPlayer Widget**
This widget displays the video. You pass the controller to it:

```dart
HaloVideoPlayer(controller: _controller)
```

### 4. **Controller Value**
The controller has a `value` property that contains:
- `isPlaying`: Whether video is currently playing
- `position`: Current playback position
- `duration`: Total video duration
- `isBuffering`: Whether video is buffering
- `aspectRatio`: Video aspect ratio
- `volume`: Current volume
- `playbackSpeed`: Current playback speed

---

## Step 5: Common Operations

### Play/Pause

```dart
// Play
await _controller.play();

// Pause
await _controller.pause();

// Check if playing
bool isPlaying = _controller.value.isPlaying;
```

### Seek to Position

```dart
// Seek to 30 seconds
await _controller.seekTo(Duration(seconds: 30));

// Seek to 1 minute 15 seconds
await _controller.seekTo(Duration(minutes: 1, seconds: 15));
```

### Volume Control

```dart
// Set volume to 50%
await _controller.setVolume(0.5);

// Mute
await _controller.setVolume(0.0);

// Full volume
await _controller.setVolume(1.0);
```

### Playback Speed

```dart
// Normal speed
await _controller.setPlaybackSpeed(1.0);

// 1.5x speed
await _controller.setPlaybackSpeed(1.5);

// 0.5x speed (slow motion)
await _controller.setPlaybackSpeed(0.5);
```

### Looping

```dart
// Enable looping
await _controller.setLooping(true);

// Disable looping
await _controller.setLooping(false);
```

### Listen to Changes

```dart
// Add a listener to react to video state changes
_controller.addListener(() {
  final value = _controller.value;
  print('Position: ${value.position}');
  print('Duration: ${value.duration}');
  print('Is Playing: ${value.isPlaying}');
});
```

---

## Step 6: Complete Example with Controls

Here's a more complete example with full video controls:

```dart
import 'package:flutter/material.dart';
import 'package:halo_videoplayer/halo_videoplayer.dart';

class AdvancedVideoPlayer extends StatefulWidget {
  @override
  _AdvancedVideoPlayerState createState() => _AdvancedVideoPlayerState();
}

class _AdvancedVideoPlayerState extends State<AdvancedVideoPlayer> {
  late HaloVideoPlayerController _controller;
  bool _isInitialized = false;

  @override
  void initState() {
    super.initState();
    _controller = HaloVideoPlayerController(
      dataSource: HaloVideoPlayerDataSource.network(
        'https://commondatastorage.googleapis.com/gtv-videos-bucket/sample/BigBuckBunny.mp4',
      ),
    );
    _controller.initialize().then((_) {
      setState(() => _isInitialized = true);
    });
    
    // Listen to position updates
    _controller.addListener(_updateUI);
  }

  void _updateUI() {
    setState(() {}); // Rebuild UI when video state changes
  }

  @override
  void dispose() {
    _controller.removeListener(_updateUI);
    _controller.dispose();
    super.dispose();
  }

  String _formatDuration(Duration duration) {
    String twoDigits(int n) => n.toString().padLeft(2, '0');
    final minutes = duration.inMinutes.remainder(60);
    final seconds = duration.inSeconds.remainder(60);
    return '${twoDigits(minutes)}:${twoDigits(seconds)}';
  }

  @override
  Widget build(BuildContext context) {
    if (!_isInitialized) {
      return Scaffold(
        body: Center(child: CircularProgressIndicator()),
      );
    }

    final value = _controller.value;
    
    return Scaffold(
      appBar: AppBar(title: Text('Video Player')),
      body: Column(
        children: [
          // Video display
          AspectRatio(
            aspectRatio: value.aspectRatio,
            child: HaloVideoPlayer(controller: _controller),
          ),
          
          // Controls
          Padding(
            padding: EdgeInsets.all(16.0),
            child: Column(
              children: [
                // Play/Pause button
                IconButton(
                  icon: Icon(value.isPlaying ? Icons.pause : Icons.play_arrow),
                  iconSize: 48,
                  onPressed: () {
                    if (value.isPlaying) {
                      _controller.pause();
                    } else {
                      _controller.play();
                    }
                  },
                ),
                
                // Progress slider
                Slider(
                  value: value.duration.inMilliseconds > 0
                      ? value.position.inMilliseconds.toDouble()
                      : 0.0,
                  min: 0.0,
                  max: value.duration.inMilliseconds > 0
                      ? value.duration.inMilliseconds.toDouble()
                      : 1.0,
                  onChanged: (double position) {
                    _controller.seekTo(Duration(milliseconds: position.toInt()));
                  },
                ),
                
                // Time display
                Row(
                  mainAxisAlignment: MainAxisAlignment.spaceBetween,
                  children: [
                    Text(_formatDuration(value.position)),
                    Text(_formatDuration(value.duration)),
                  ],
                ),
                
                SizedBox(height: 16),
                
                // Volume control
                Row(
                  children: [
                    Icon(Icons.volume_up),
                    Expanded(
                      child: Slider(
                        value: value.volume,
                        min: 0.0,
                        max: 1.0,
                        onChanged: (double volume) {
                          _controller.setVolume(volume);
                        },
                      ),
                    ),
                  ],
                ),
                
                // Speed control
                Row(
                  children: [
                    Text('Speed: '),
                    Expanded(
                      child: Slider(
                        value: value.playbackSpeed,
                        min: 0.25,
                        max: 2.0,
                        divisions: 7,
                        label: '${value.playbackSpeed}x',
                        onChanged: (double speed) {
                          _controller.setPlaybackSpeed(speed);
                        },
                      ),
                    ),
                  ],
                ),
              ],
            ),
          ),
        ],
      ),
    );
  }
}
```

---

## Summary

1. **Add dependency** to `pubspec.yaml`
2. **Import** the package
3. **Create** a `HaloVideoPlayerController` with your video source
4. **Initialize** the controller
5. **Display** the video using `HaloVideoPlayer` widget
6. **Control** playback using controller methods
7. **Dispose** the controller when done

That's it! The plugin handles all the platform-specific details automatically.

