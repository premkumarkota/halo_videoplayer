# How End Users Use Halo Video Player Plugin

## Overview

As an end-user (Flutter developer), you use this plugin just like any other Flutter package. The plugin handles all the complexity of different platforms (Android, iOS, Windows, Linux, macOS, Web) behind the scenes.

---

## The Simple Flow

```
┌─────────────────────────────────────────────────────────┐
│  1. Add plugin to pubspec.yaml                          │
│     dependencies:                                        │
│       halo_videoplayer: ^0.0.1                          │
└─────────────────────────────────────────────────────────┘
                        ↓
┌─────────────────────────────────────────────────────────┐
│  2. Import the package                                   │
│     import 'package:halo_videoplayer/halo_videoplayer.dart';│
└─────────────────────────────────────────────────────────┘
                        ↓
┌─────────────────────────────────────────────────────────┐
│  3. Create a controller                                  │
│     _controller = HaloVideoPlayerController(             │
│       dataSource: HaloVideoPlayerDataSource.network(...) │
│     );                                                   │
└─────────────────────────────────────────────────────────┘
                        ↓
┌─────────────────────────────────────────────────────────┐
│  4. Initialize                                           │
│     await _controller.initialize();                     │
└─────────────────────────────────────────────────────────┘
                        ↓
┌─────────────────────────────────────────────────────────┐
│  5. Display the video                                   │
│     HaloVideoPlayer(controller: _controller)            │
└─────────────────────────────────────────────────────────┘
                        ↓
┌─────────────────────────────────────────────────────────┐
│  6. Control playback                                     │
│     _controller.play()                                   │
│     _controller.pause()                                  │
│     _controller.seekTo(...)                               │
└─────────────────────────────────────────────────────────┘
```

---

## Complete Working Example

Here's a complete, copy-paste ready example:

```dart
import 'package:flutter/material.dart';
import 'package:halo_videoplayer/halo_videoplayer.dart';

void main() => runApp(MyApp());

class MyApp extends StatelessWidget {
  @override
  Widget build(BuildContext context) {
    return MaterialApp(home: VideoScreen());
  }
}

class VideoScreen extends StatefulWidget {
  @override
  _VideoScreenState createState() => _VideoScreenState();
}

class _VideoScreenState extends State<VideoScreen> {
  late HaloVideoPlayerController _controller;

  @override
  void initState() {
    super.initState();
    
    // Create controller
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

---

## Key Concepts Explained

### 1. **HaloVideoPlayerController**
- This is your main interface to control the video
- You create one controller per video
- It manages playback state, position, volume, etc.

### 2. **HaloVideoPlayerDataSource**
- Tells the plugin where to find the video
- Three types:
  - `network(url)` - Video from internet
  - `file(path)` - Video file on device
  - `asset(path)` - Video bundled with app

### 3. **HaloVideoPlayer Widget**
- The visual component that displays the video
- Just pass your controller to it
- Handles rendering automatically

### 4. **Controller Methods**
- `initialize()` - Must call this first
- `play()` - Start playback
- `pause()` - Pause playback
- `seekTo(duration)` - Jump to position
- `setVolume(0.0-1.0)` - Change volume
- `setPlaybackSpeed(0.25-2.0)` - Change speed
- `setLooping(true/false)` - Enable/disable loop
- `dispose()` - Clean up when done

### 5. **Controller Value**
- `_controller.value` gives you current state:
  - `isPlaying` - Is video playing?
  - `position` - Current position
  - `duration` - Total duration
  - `aspectRatio` - Video dimensions
  - `volume` - Current volume
  - `playbackSpeed` - Current speed

---

## Real-World Usage Patterns

### Pattern 1: Simple Video Player
```dart
// Just show a video with play/pause
_controller = HaloVideoPlayerController(
  dataSource: HaloVideoPlayerDataSource.network(url),
);
await _controller.initialize();
HaloVideoPlayer(controller: _controller)
```

### Pattern 2: Video with Custom Controls
```dart
// Create custom UI around the video
Column(
  children: [
    HaloVideoPlayer(controller: _controller),
    Row(
      children: [
        IconButton(icon: Icon(Icons.play), onPressed: () => _controller.play()),
        IconButton(icon: Icon(Icons.pause), onPressed: () => _controller.pause()),
        Slider(
          value: _controller.value.position.inSeconds.toDouble(),
          max: _controller.value.duration.inSeconds.toDouble(),
          onChanged: (v) => _controller.seekTo(Duration(seconds: v.toInt())),
        ),
      ],
    ),
  ],
)
```

### Pattern 3: Multiple Videos
```dart
// Create multiple controllers for different videos
final video1Controller = HaloVideoPlayerController(
  dataSource: HaloVideoPlayerDataSource.network(url1),
);
final video2Controller = HaloVideoPlayerController(
  dataSource: HaloVideoPlayerDataSource.network(url2),
);

await video1Controller.initialize();
await video2Controller.initialize();
```

### Pattern 4: Listening to Changes
```dart
// React to video state changes
_controller.addListener(() {
  final value = _controller.value;
  if (value.position >= value.duration) {
    // Video finished
    print('Video completed!');
  }
});
```

---

## What Happens Behind the Scenes?

When you use this plugin:

1. **You write simple code** - Just create controller and widget
2. **Plugin detects platform** - Automatically knows if you're on Android, iOS, Windows, etc.
3. **Plugin uses best player**:
   - Android/iOS/macOS/Web → Uses `video_player` package
   - Windows → Uses Windows Media Foundation (native)
   - Linux → Uses GStreamer (native)
4. **You get unified API** - Same code works everywhere!

---

## Common Questions

**Q: Do I need different code for different platforms?**  
A: No! The same code works on all platforms.

**Q: How do I handle errors?**  
A: Wrap `initialize()` in try-catch:
```dart
try {
  await _controller.initialize();
} catch (e) {
  print('Error: $e');
}
```

**Q: Can I play local files?**  
A: Yes! Use `HaloVideoPlayerDataSource.file('/path/to/video.mp4')`

**Q: Does it work offline?**  
A: Yes, if you use file or asset sources. Network sources require internet.

**Q: How do I know when video is ready?**  
A: Check `_controller.value.isInitialized`

---

## Summary

Using this plugin is simple:

1. **Add** to pubspec.yaml
2. **Import** the package  
3. **Create** a controller with video source
4. **Initialize** it
5. **Display** with HaloVideoPlayer widget
6. **Control** with simple methods
7. **Dispose** when done

That's it! The plugin handles all platform complexity for you.

