import 'package:flutter_test/flutter_test.dart';
import 'package:halo_videoplayer/halo_videoplayer.dart';
import 'package:halo_videoplayer/halo_videoplayer_platform_interface.dart';
import 'package:halo_videoplayer/halo_videoplayer_method_channel.dart';
import 'package:plugin_platform_interface/plugin_platform_interface.dart';

class MockHaloVideoplayerPlatform
    with MockPlatformInterfaceMixin
    implements HaloVideoplayerPlatform {

  @override
  Future<String?> getPlatformVersion() => Future.value('42');
}

void main() {
  final HaloVideoplayerPlatform initialPlatform = HaloVideoplayerPlatform.instance;

  test('$MethodChannelHaloVideoplayer is the default instance', () {
    expect(initialPlatform, isInstanceOf<MethodChannelHaloVideoplayer>());
  });

  test('getPlatformVersion', () async {
    HaloVideoplayer haloVideoplayerPlugin = HaloVideoplayer();
    MockHaloVideoplayerPlatform fakePlatform = MockHaloVideoplayerPlatform();
    HaloVideoplayerPlatform.instance = fakePlatform;

    expect(await haloVideoplayerPlugin.getPlatformVersion(), '42');
  });
}
