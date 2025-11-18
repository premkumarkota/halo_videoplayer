import 'package:flutter_test/flutter_test.dart';
import 'package:halo_videoplayer/halo_videoplayer.dart';
import 'package:halo_videoplayer/halo_videoplayer_method_channel.dart';

void main() {
  TestWidgetsFlutterBinding.ensureInitialized();

  final HaloVideoplayerPlatform initialPlatform =
      HaloVideoplayerPlatform.instance;

  test('$MethodChannelHaloVideoplayer is the default instance', () {
    expect(initialPlatform, isInstanceOf<MethodChannelHaloVideoplayer>());
  });

  test('HaloVideoPlayerController can be created', () {
    final controller = HaloVideoPlayerController(
      dataSource: HaloVideoPlayerDataSource.network(
        'https://example.com/video.mp4',
      ),
    );
    expect(controller, isNotNull);
    expect(controller.dataSource.isNetwork, true);
    expect(controller.dataSource.source, 'https://example.com/video.mp4');
  });

  test('HaloVideoPlayerController with asset source', () {
    final controller = HaloVideoPlayerController(
      dataSource: HaloVideoPlayerDataSource.asset('videos/test.mp4'),
    );
    expect(controller.dataSource.isAsset, true);
    expect(controller.dataSource.source, 'videos/test.mp4');
  });

  test('HaloVideoPlayerController with file source', () {
    final controller = HaloVideoPlayerController(
      dataSource: HaloVideoPlayerDataSource.file('/path/to/video.mp4'),
    );
    expect(controller.dataSource.isNetwork, false);
    expect(controller.dataSource.isAsset, false);
    expect(controller.dataSource.source, '/path/to/video.mp4');
  });
}
