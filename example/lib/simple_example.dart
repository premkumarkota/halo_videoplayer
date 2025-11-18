// Simple Example - How End Users Use the Plugin
//
// This is a minimal example showing how to use halo_videoplayer plugin
// in a Flutter application.

import 'package:flutter/material.dart';
import 'package:halo_videoplayer/halo_videoplayer.dart';

void main() {
  runApp(const SimpleVideoApp());
}

class SimpleVideoApp extends StatelessWidget {
  const SimpleVideoApp({super.key});

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: 'Simple Video Player Example',
      theme: ThemeData(primarySwatch: Colors.blue),
      home: const SimpleVideoPlayerPage(),
    );
  }
}

class SimpleVideoPlayerPage extends StatefulWidget {
  const SimpleVideoPlayerPage({super.key});

  @override
  State<SimpleVideoPlayerPage> createState() => _SimpleVideoPlayerPageState();
}

class _SimpleVideoPlayerPageState extends State<SimpleVideoPlayerPage> {
  // Step 1: Create a controller
  late HaloVideoPlayerController _controller;
  bool _isInitialized = false;

  @override
  void initState() {
    super.initState();

    // Step 2: Initialize the controller with a video source
    _controller = HaloVideoPlayerController(
      // You can use:
      // - HaloVideoPlayerDataSource.network(url) for internet videos
      // - HaloVideoPlayerDataSource.file(path) for local files
      // - HaloVideoPlayerDataSource.asset(path) for app assets
      dataSource: HaloVideoPlayerDataSource.network(
        'https://flutter.github.io/assets-for-api-docs/assets/videos/bee.mp4',
      ),
      autoPlay: false, // Set to true to auto-play
      looping: false, // Set to true to loop
      volume: 1.0, // Volume from 0.0 to 1.0
    );

    // Step 3: Initialize the video player
    _controller
        .initialize()
        .then((_) {
          setState(() {
            _isInitialized = true;
          });
        })
        .catchError((error) {
          debugPrint('Error: $error');
        });
  }

  @override
  void dispose() {
    // Step 4: Always dispose the controller when done
    _controller.dispose();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(title: const Text('Simple Video Player')),
      body: Column(
        children: [
          // Step 5: Display the video player
          if (_isInitialized)
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
            )
          else
            const Expanded(child: Center(child: CircularProgressIndicator())),

          // Step 6: Add controls
          Padding(
            padding: const EdgeInsets.all(16.0),
            child: Column(
              children: [
                // Play/Pause button
                IconButton(
                  icon: Icon(
                    _controller.value.isPlaying
                        ? Icons.pause
                        : Icons.play_arrow,
                    size: 48,
                  ),
                  onPressed: () {
                    if (_controller.value.isPlaying) {
                      _controller.pause();
                    } else {
                      _controller.play();
                    }
                    setState(() {});
                  },
                ),

                // Show current position and duration
                Text(
                  '${_formatTime(_controller.value.position)} / '
                  '${_formatTime(_controller.value.duration)}',
                  style: Theme.of(context).textTheme.bodyMedium,
                ),
              ],
            ),
          ),
        ],
      ),
    );
  }

  String _formatTime(Duration duration) {
    String twoDigits(int n) => n.toString().padLeft(2, '0');
    final minutes = duration.inMinutes.remainder(60);
    final seconds = duration.inSeconds.remainder(60);
    return '${twoDigits(minutes)}:${twoDigits(seconds)}';
  }
}
