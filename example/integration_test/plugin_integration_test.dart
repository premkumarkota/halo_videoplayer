// This is a basic Flutter integration test.
//
// Since integration tests run in a full Flutter application, they can interact
// with the host side of a plugin implementation, unlike Dart unit tests.
//
// For more information about Flutter integration tests, please see
// https://flutter.dev/to/integration-testing

import 'package:flutter_test/flutter_test.dart';
import 'package:integration_test/integration_test.dart';

import 'package:halo_videoplayer/halo_videoplayer.dart';

void main() {
  IntegrationTestWidgetsFlutterBinding.ensureInitialized();

  testWidgets('HaloVideoPlayerController initialization test', (
    WidgetTester tester,
  ) async {
    final controller = HaloVideoPlayerController(
      dataSource: HaloVideoPlayerDataSource.network(
        'https://flutter.github.io/assets-for-api-docs/assets/videos/bee.mp4',
      ),
    );

    // Test that controller can be created
    expect(controller, isNotNull);
    expect(controller.dataSource.isNetwork, true);

    // Clean up
    await controller.dispose();
  });
}
