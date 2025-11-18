# Step-by-Step Guide: Publishing to pub.dev

## Prerequisites Checklist

Before publishing, make sure you have:

- [ ] A Google account (for pub.dev)
- [ ] Flutter SDK installed and working
- [ ] Dart SDK installed
- [ ] Git installed (for version control)
- [ ] Package is complete and tested

---

## Step 1: Create/Update pubspec.yaml

### 1.1 Update Package Metadata

Your `pubspec.yaml` should have:

```yaml
name: halo_videoplayer  # Must be unique on pub.dev
description: "A unified video player plugin that works across all platforms (Android, iOS, macOS, Windows, Linux, Web) with support for all standard video formats."
version: 0.0.1  # Start with 0.0.1 for initial release
homepage: https://github.com/yourusername/halo_videoplayer  # Your GitHub repo
repository: https://github.com/yourusername/halo_videoplayer  # Optional but recommended
issue_tracker: https://github.com/yourusername/halo_videoplayer/issues  # Optional
```

### 1.2 Important Requirements

- **Name**: Must be lowercase with underscores, unique on pub.dev
- **Description**: Should be clear and concise (max 60 characters recommended)
- **Version**: Follow semantic versioning (MAJOR.MINOR.PATCH)
- **Homepage**: Should point to your repository

---

## Step 2: Add License File

pub.dev requires a LICENSE file. Common options:

### Option A: MIT License (Recommended)

Create `LICENSE` file in root directory:

```
MIT License

Copyright (c) 2024 [Your Name]

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
```

### Option B: Other Licenses

You can use Apache 2.0, BSD, or other open-source licenses. See: https://choosealicense.com/

---

## Step 3: Update CHANGELOG.md

Create/update `CHANGELOG.md` in root directory:

```markdown
## 0.0.1

* Initial release
* Support for Android, iOS, macOS, Windows, Linux, and Web
* Network, file, and asset video sources
* Play, pause, seek, volume, speed, and looping controls
* Unified API across all platforms
```

**Important**: The version in CHANGELOG.md must match pubspec.yaml version.

---

## Step 4: Update README.md

Make sure your README.md includes:

- [ ] Clear description
- [ ] Installation instructions
- [ ] Usage examples
- [ ] Platform support information
- [ ] License information

Your README.md looks good! Just make sure it's complete.

---

## Step 5: Verify Package Quality

### 5.1 Run Package Analyzer

```bash
# Install pub tools if not already installed
dart pub global activate pana

# Analyze your package
pana --no-warning
```

Fix any issues reported.

### 5.2 Run Tests

```bash
# Run all tests
flutter test

# Run example app to verify it works
cd example
flutter run
```

### 5.3 Check for Common Issues

- [ ] No TODOs in code
- [ ] All imports are used
- [ ] No linter errors: `flutter analyze`
- [ ] Example app works
- [ ] Documentation is complete

---

## Step 6: Create GitHub Repository (Recommended)

### 6.1 Create Repository

1. Go to https://github.com/new
2. Create a new repository named `halo_videoplayer`
3. **Don't** initialize with README (you already have one)
4. Click "Create repository"

### 6.2 Push Your Code

```bash
# Initialize git if not already done
git init

# Add all files
git add .

# Commit
git commit -m "Initial release: v0.0.1"

# Add remote (replace YOUR_USERNAME)
git remote add origin https://github.com/YOUR_USERNAME/halo_videoplayer.git

# Push
git branch -M main
git push -u origin main
```

### 6.3 Update pubspec.yaml with Repository URL

```yaml
homepage: https://github.com/YOUR_USERNAME/halo_videoplayer
repository: https://github.com/YOUR_USERNAME/halo_videoplayer
```

---

## Step 7: Prepare for Publishing

### 7.1 Verify Package Structure

Your package should have:

```
halo_videoplayer/
├── lib/
│   ├── halo_videoplayer.dart
│   ├── halo_videoplayer_platform_interface.dart
│   └── ...
├── example/
│   └── lib/
│       └── main.dart
├── test/
│   └── ...
├── pubspec.yaml
├── README.md
├── CHANGELOG.md
└── LICENSE
```

### 7.2 Final Checks

```bash
# Format code
dart format .

# Analyze code
flutter analyze

# Run tests
flutter test

# Verify pubspec.yaml
dart pub publish --dry-run
```

The `--dry-run` flag shows what would be published without actually publishing.

---

## Step 8: Create pub.dev Account

### 8.1 Sign In to pub.dev

1. Go to https://pub.dev
2. Click "Sign in" (top right)
3. Sign in with your Google account
4. Authorize pub.dev to access your Google account

### 8.2 Verify Email (if required)

Check your email and verify your account if needed.

---

## Step 9: Publish the Package

### 9.1 Final Verification

```bash
# This shows what will be published
dart pub publish --dry-run
```

Review the output carefully. Make sure:
- All files are included
- No sensitive files are included
- Version is correct
- Description is correct

### 9.2 Publish

```bash
# Publish to pub.dev
dart pub publish
```

**Important**: This is permanent! Once published, you cannot delete the version (only deprecate it).

### 9.3 What Happens Next

1. pub.dev validates your package
2. If successful, package is published
3. You'll see a success message with the package URL
4. Package appears on pub.dev within a few minutes

---

## Step 10: Verify Publication

### 10.1 Check Package Page

Visit: `https://pub.dev/packages/halo_videoplayer`

Your package should appear with:
- Description
- Version
- Documentation
- Example code
- Platform support badges

### 10.2 Test Installation

Create a test project:

```bash
# Create test project
flutter create test_app
cd test_app

# Add your package
flutter pub add halo_videoplayer

# Verify it works
flutter pub get
```

---

## Step 11: Post-Publishing

### 11.1 Add Badge to README

Add this to your README.md:

```markdown
[![pub package](https://img.shields.io/pub/v/halo_videoplayer.svg)](https://pub.dev/packages/halo_videoplayer)
```

### 11.2 Share Your Package

- Share on Flutter communities
- Add to your portfolio
- Write a blog post (optional)

---

## Updating Your Package

When you want to publish a new version:

### 1. Update Version

In `pubspec.yaml`:
```yaml
version: 0.0.2  # Increment version
```

### 2. Update CHANGELOG.md

```markdown
## 0.0.2

* Fixed bug with video initialization
* Added support for custom headers
* Improved error handling

## 0.0.1

* Initial release
```

### 3. Commit Changes

```bash
git add .
git commit -m "Release v0.0.2"
git tag v0.0.2
git push origin main --tags
```

### 4. Publish

```bash
dart pub publish
```

---

## Common Issues and Solutions

### Issue: "Package name already taken"

**Solution**: Choose a different name in pubspec.yaml

### Issue: "Version already exists"

**Solution**: Increment version number in pubspec.yaml

### Issue: "Missing LICENSE file"

**Solution**: Create LICENSE file (see Step 2)

### Issue: "CHANGELOG.md version mismatch"

**Solution**: Ensure CHANGELOG.md version matches pubspec.yaml

### Issue: "Package score too low"

**Solution**: 
- Add more documentation
- Add more tests
- Fix linter warnings
- Add example app

---

## Package Score Tips

pub.dev gives your package a score. Improve it by:

- ✅ Complete README.md
- ✅ Complete CHANGELOG.md
- ✅ Add example app
- ✅ Add tests
- ✅ Fix all linter warnings
- ✅ Add documentation comments
- ✅ Follow Dart style guide
- ✅ Add repository URL

---

## Summary Checklist

Before publishing, ensure:

- [ ] `pubspec.yaml` is complete and correct
- [ ] `LICENSE` file exists
- [ ] `CHANGELOG.md` exists and matches version
- [ ] `README.md` is complete
- [ ] Code is formatted (`dart format .`)
- [ ] No linter errors (`flutter analyze`)
- [ ] Tests pass (`flutter test`)
- [ ] Example app works
- [ ] `dart pub publish --dry-run` shows no errors
- [ ] GitHub repository is set up (recommended)
- [ ] pub.dev account is created

---

## Quick Reference Commands

```bash
# Format code
dart format .

# Analyze code
flutter analyze

# Run tests
flutter test

# Dry run (see what would be published)
dart pub publish --dry-run

# Publish
dart pub publish

# Check package score
pana --no-warning
```

---

## Need Help?

- pub.dev documentation: https://dart.dev/tools/pub/publishing
- Flutter plugin guide: https://flutter.dev/docs/development/packages-and-plugins/developing-packages
- pub.dev support: https://pub.dev/help

Good luck with your publication! 🚀

