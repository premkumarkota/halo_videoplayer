import 'package:flutter/material.dart';
import 'package:halo_videoplayer/halo_videoplayer.dart';

void main() {
  runApp(const MyApp());
}

class MyApp extends StatefulWidget {
  const MyApp({super.key});

  @override
  State<MyApp> createState() => _MyAppState();
}

class _MyAppState extends State<MyApp> {
  late HaloVideoPlayerController _controller;

  @override
  void initState() {
    super.initState();
    // Initialize with a network video URL
    _controller = HaloVideoPlayerController(
      dataSource: HaloVideoPlayerDataSource.network(
        'https://flutter.github.io/assets-for-api-docs/assets/videos/bee.mp4',
      ),
      autoPlay: false,
      looping: false,
    );
    _controller
        .initialize()
        .then((_) {
          setState(() {});
        })
        .catchError((error) {
          debugPrint('Error initializing video: $error');
        });
  }

  @override
  void dispose() {
    _controller.dispose();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: 'Halo Video Player Example',
      theme: ThemeData(primarySwatch: Colors.blue),
      home: Scaffold(
        appBar: AppBar(title: const Text('Halo Video Player')),
        body: Column(
          children: [
            // Video player widget
            Expanded(
              child: Center(
                child: AspectRatio(
                  aspectRatio:
                      _controller.value.aspectRatio != 0
                          ? _controller.value.aspectRatio
                          : 16 / 9,
                  child: HaloVideoPlayer(controller: _controller),
                ),
              ),
            ),
            // Controls
            Padding(
              padding: const EdgeInsets.all(16.0),
              child: Column(
                children: [
                  // Play/Pause button
                  Row(
                    mainAxisAlignment: MainAxisAlignment.center,
                    children: [
                      IconButton(
                        icon: Icon(
                          _controller.value.isPlaying
                              ? Icons.pause
                              : Icons.play_arrow,
                        ),
                        onPressed: () {
                          if (_controller.value.isPlaying) {
                            _controller.pause();
                          } else {
                            _controller.play();
                          }
                          setState(() {});
                        },
                        iconSize: 48,
                      ),
                    ],
                  ),
                  // Position slider
                  ValueListenableBuilder<HaloVideoPlayerValue>(
                    valueListenable: _controller,
                    builder: (context, value, child) {
                      final position = value.position;
                      final duration = value.duration;
                      return Column(
                        children: [
                          Slider(
                            value:
                                duration.inMilliseconds > 0
                                    ? position.inMilliseconds.toDouble()
                                    : 0.0,
                            min: 0.0,
                            max:
                                duration.inMilliseconds > 0
                                    ? duration.inMilliseconds.toDouble()
                                    : 1.0,
                            onChanged: (double value) {
                              _controller.seekTo(
                                Duration(milliseconds: value.toInt()),
                              );
                            },
                          ),
                          Padding(
                            padding: const EdgeInsets.symmetric(
                              horizontal: 16.0,
                            ),
                            child: Row(
                              mainAxisAlignment: MainAxisAlignment.spaceBetween,
                              children: [
                                Text(_formatDuration(position)),
                                Text(_formatDuration(duration)),
                              ],
                            ),
                          ),
                        ],
                      );
                    },
                  ),
                  // Volume control
                  Row(
                    children: [
                      const Text('Volume: '),
                      Expanded(
                        child: Slider(
                          value: _controller.volume,
                          min: 0.0,
                          max: 1.0,
                          onChanged: (double value) {
                            _controller.setVolume(value);
                            setState(() {});
                          },
                        ),
                      ),
                    ],
                  ),
                  // Playback speed control
                  Row(
                    children: [
                      const Text('Speed: '),
                      Expanded(
                        child: Slider(
                          value: _controller.playbackSpeed,
                          min: 0.25,
                          max: 2.0,
                          divisions: 7,
                          label: '${_controller.playbackSpeed}x',
                          onChanged: (double value) {
                            _controller.setPlaybackSpeed(value);
                            setState(() {});
                          },
                        ),
                      ),
                    ],
                  ),
                  // Loop toggle
                  Row(
                    mainAxisAlignment: MainAxisAlignment.center,
                    children: [
                      const Text('Loop: '),
                      Switch(
                        value: _controller.looping,
                        onChanged: (bool value) {
                          _controller.setLooping(value);
                          setState(() {});
                        },
                      ),
                    ],
                  ),
                ],
              ),
            ),
          ],
        ),
      ),
    );
  }

  String _formatDuration(Duration duration) {
    String twoDigits(int n) => n.toString().padLeft(2, '0');
    final hours = duration.inHours;
    final minutes = duration.inMinutes.remainder(60);
    final seconds = duration.inSeconds.remainder(60);
    if (hours > 0) {
      return '${twoDigits(hours)}:${twoDigits(minutes)}:${twoDigits(seconds)}';
    }
    return '${twoDigits(minutes)}:${twoDigits(seconds)}';
  }
}
