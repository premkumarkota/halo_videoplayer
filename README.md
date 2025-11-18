# Halo Video Player

[![pub package](https://img.shields.io/pub/v/halo_videoplayer.svg)](https://pub.dev/packages/halo_videoplayer)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

A unified video player plugin for Flutter that works across all platforms (Android, iOS, macOS, Windows, Linux, Web) with support for all standard video formats including `.mp4`, `.mov`, `.avi`, and more.

## Features

- ✅ **Cross-platform support**: Works on Android, iOS, macOS, Windows, Linux, and Web
- ✅ **Format support**: Plays all standard video formats (`.mp4`, `.mov`, `.avi`, etc.)
- ✅ **Platform-optimized**: Uses the best video player for each platform
  - Android/iOS/macOS/Web: Uses `video_player` package
  - Windows: Uses Windows Media Foundation (native implementation)
  - Linux: Uses GStreamer (native implementation)
- ✅ **Full control**: Play, pause, seek, volume, playback speed, looping
- ✅ **Easy to use**: Simple API similar to `video_player` package

## Installation

Add this to your `pubspec.yaml`:

```yaml
dependencies:
  halo_videoplayer: ^0.0.1
```

Then run:

```bash
flutter pub get
```

## Platform Setup

### Android

No additional setup required. The plugin uses `video_player` which handles Android automatically.

### iOS

No additional setup required. The plugin uses `video_player` which handles iOS automatically.

### macOS

No additional setup required. The plugin uses `video_player` which handles macOS automatically.

### Windows

The plugin uses Windows Media Foundation (WMF) which is built into Windows. No additional dependencies required.

### Linux

The plugin requires GStreamer and its development packages. Install them:

**Ubuntu/Debian:**
```bash
sudo apt-get install libgstreamer1.0-dev libgstreamer-plugins-base1.0-dev \
  gstreamer1.0-plugins-base gstreamer1.0-plugins-good \
  gstreamer1.0-plugins-bad gstreamer1.0-plugins-ugly \
  gstreamer1.0-libav
```

**Fedora:**
```bash
sudo dnf install gstreamer1-devel gstreamer1-plugins-base-devel \
  gstreamer1-plugins-good gstreamer1-plugins-bad-free \
  gstreamer1-plugins-ugly-free gstreamer1-libav
```

**Arch Linux:**
```bash
sudo pacman -S gstreamer gstreamer-vaapi gst-plugins-base \
  gst-plugins-good gst-plugins-bad gst-plugins-ugly gst-libav
```

### Web

No additional setup required. The plugin uses `video_player` which handles Web automatically.

## Usage

### Basic Example

```dart
import 'package:flutter/material.dart';
import 'package:halo_videoplayer/halo_videoplayer.dart';

class VideoPlayerExample extends StatefulWidget {
  @override
  _VideoPlayerExampleState createState() => _VideoPlayerExampleState();
}

class _VideoPlayerExampleState extends State<VideoPlayerExample> {
  late HaloVideoPlayerController _controller;

  @override
  void initState() {
    super.initState();
    _controller = HaloVideoPlayerController(
      dataSource: HaloVideoPlayerDataSource.network(
        'https://example.com/video.mp4',
      ),
      autoPlay: false,
      looping: false,
    );
    _controller.initialize().then((_) {
      setState(() {});
    });
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
          aspectRatio: _controller.value.aspectRatio,
          child: HaloVideoPlayer(controller: _controller),
        ),
      ),
      floatingActionButton: FloatingActionButton(
        onPressed: () {
          if (_controller.value.isPlaying) {
            _controller.pause();
          } else {
            _controller.play();
          }
          setState(() {});
        },
        child: Icon(
          _controller.value.isPlaying ? Icons.pause : Icons.play_arrow,
        ),
      ),
    );
  }
}
```

### Data Sources

#### Network Video

```dart
_controller = HaloVideoPlayerController(
  dataSource: HaloVideoPlayerDataSource.network(
    'https://example.com/video.mp4',
    httpHeaders: {'Authorization': 'Bearer token'},
  ),
);
```

#### File Video

```dart
_controller = HaloVideoPlayerController(
  dataSource: HaloVideoPlayerDataSource.file('/path/to/video.mp4'),
);
```

#### Asset Video

```dart
_controller = HaloVideoPlayerController(
  dataSource: HaloVideoPlayerDataSource.asset('videos/sample.mp4'),
);
```

### Controls

```dart
// Play
await _controller.play();

// Pause
await _controller.pause();

// Seek to position
await _controller.seekTo(Duration(seconds: 30));

// Set volume (0.0 to 1.0)
await _controller.setVolume(0.5);

// Set playback speed
await _controller.setPlaybackSpeed(1.5); // 1.5x speed

// Set looping
await _controller.setLooping(true);
```

### Listening to Player State

```dart
_controller.addListener(() {
  final value = _controller.value;
  print('Playing: ${value.isPlaying}');
  print('Position: ${value.position}');
  print('Duration: ${value.duration}');
  print('Buffering: ${value.isBuffering}');
});
```

## Supported Formats

### Android
- `.mp4`, `.mov`, `.avi`, `.mkv`, `.webm`, and more (via ExoPlayer)

### iOS/macOS
- `.mp4`, `.mov`, `.m4v`, and more (via AVPlayer)
- Note: `.avi` format may have limited support on iOS/macOS

### Windows
- `.mp4`, `.mov`, `.avi`, `.mkv`, `.wmv`, and more (via Windows Media Foundation)

### Linux
- `.mp4`, `.mov`, `.avi`, `.mkv`, `.webm`, and more (via GStreamer)
- Requires GStreamer plugins to be installed

### Web
- Depends on browser support (typically `.mp4`, `.webm`)

## Troubleshooting

### Linux: Video not playing

Make sure GStreamer and all required plugins are installed:

```bash
# Check if GStreamer is installed
gst-inspect-1.0 --version

# Check available plugins
gst-inspect-1.0 | grep -i "decode"
```

### Windows: Video not playing

Ensure Windows Media Foundation is available (it's built into Windows 7+). If videos still don't play, check that the video format is supported by Windows Media Foundation.

### Format not supported

If a specific format doesn't play on a platform:
- **Linux**: Install additional GStreamer plugins (bad, ugly, libav)
- **Windows**: The format may not be supported by Windows Media Foundation
- **iOS/macOS**: Check AVPlayer supported formats for your iOS/macOS version

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request.

## License

This project is licensed under the MIT License.
