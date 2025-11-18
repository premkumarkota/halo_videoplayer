import 'package:flutter/services.dart';
import 'package:flutter_test/flutter_test.dart';
import 'package:halo_videoplayer/halo_videoplayer_method_channel.dart';
import 'package:halo_videoplayer/halo_videoplayer_platform_interface.dart';

void main() {
  TestWidgetsFlutterBinding.ensureInitialized();

  MethodChannelHaloVideoplayer platform = MethodChannelHaloVideoplayer();
  const MethodChannel channel = MethodChannel('halo_videoplayer');

  setUp(() {
    TestDefaultBinaryMessengerBinding.instance.defaultBinaryMessenger
        .setMockMethodCallHandler(channel, (MethodCall methodCall) async {
          return null;
        });
  });

  tearDown(() {
    TestDefaultBinaryMessengerBinding.instance.defaultBinaryMessenger
        .setMockMethodCallHandler(channel, null);
  });

  test('MethodChannelHaloVideoplayer instance creation', () {
    expect(platform, isNotNull);
    expect(platform, isInstanceOf<HaloVideoplayerPlatform>());
  });
}
