# Simple Example - How End Users Use the Plugin

## Quick Start (5 minutes)

### 1. Add to pubspec.yaml

```yaml
dependencies:
  halo_videoplayer: ^0.0.1
```

### 2. Copy this code

```dart
import 'package:flutter/material.dart';
import 'package:halo_videoplayer/halo_videoplayer.dart';

void main() => runApp(MyApp());

class MyApp extends StatelessWidget {
  @override
  Widget build(BuildContext context) {
    return MaterialApp(home: VideoPage());
  }
}

class VideoPage extends StatefulWidget {
  @override
  _VideoPageState createState() => _VideoPageState();
}

class _VideoPageState extends State<VideoPage> {
  late HaloVideoPlayerController _controller;

  @override
  void initState() {
    super.initState();
    
    // Create controller with video URL
    _controller = HaloVideoPlayerController(
      dataSource: HaloVideoPlayerDataSource.network(
        'https://flutter.github.io/assets-for-api-docs/assets/videos/bee.mp4',
      ),
    );
    
    // Initialize
    _controller.initialize();
  }

  @override
  void dispose() {
    _controller.dispose();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      body: Center(
        child: AspectRatio(
          aspectRatio: 16 / 9,
          child: HaloVideoPlayer(controller: _controller),
        ),
      ),
      floatingActionButton: FloatingActionButton(
        onPressed: () {
          _controller.value.isPlaying 
              ? _controller.pause() 
              : _controller.play();
          setState(() {});
        },
        child: Icon(_controller.value.isPlaying 
            ? Icons.pause 
            : Icons.play_arrow),
      ),
    );
  }
}
```

### 3. Run it!

```bash
flutter run
```

---

## What Happens Behind the Scenes?

1. **You create** `HaloVideoPlayerController` with a video URL
2. **You call** `initialize()` - the plugin detects your platform
3. **On Android/iOS/macOS/Web**: Uses `video_player` package
4. **On Windows**: Uses Windows Media Foundation (native)
5. **On Linux**: Uses GStreamer (native)
6. **You display** the video using `HaloVideoPlayer` widget
7. **You control** it with simple methods like `play()`, `pause()`, `seekTo()`

**You don't need to worry about platform differences - the plugin handles everything!**

---

## Common Use Cases

### Play a video from internet
```dart
_controller = HaloVideoPlayerController(
  dataSource: HaloVideoPlayerDataSource.network('https://example.com/video.mp4'),
);
```

### Play a local file
```dart
_controller = HaloVideoPlayerController(
  dataSource: HaloVideoPlayerDataSource.file('/path/to/video.mp4'),
);
```

### Play from app assets
```dart
_controller = HaloVideoPlayerController(
  dataSource: HaloVideoPlayerDataSource.asset('videos/intro.mp4'),
);
```

### Auto-play and loop
```dart
_controller = HaloVideoPlayerController(
  dataSource: HaloVideoPlayerDataSource.network('https://example.com/video.mp4'),
  autoPlay: true,
  looping: true,
);
```

---

## That's It!

The plugin works the same way on all platforms. Just use the simple API and it handles the rest!

