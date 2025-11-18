# Pre-Publishing Checklist

Use this checklist before publishing to pub.dev:

## Required Files

- [ ] `pubspec.yaml` - Complete with all metadata
- [ ] `LICENSE` - MIT or other open-source license
- [ ] `CHANGELOG.md` - Updated with version 0.0.1
- [ ] `README.md` - Complete documentation
- [ ] `example/` - Working example app

## pubspec.yaml Requirements

- [ ] `name` is unique and lowercase
- [ ] `description` is clear and concise
- [ ] `version` follows semantic versioning
- [ ] `homepage` points to GitHub repository
- [ ] `repository` is set (optional but recommended)
- [ ] All dependencies are listed
- [ ] SDK constraints are appropriate

## Code Quality

- [ ] Code is formatted: `dart format .`
- [ ] No linter errors: `flutter analyze`
- [ ] All tests pass: `flutter test`
- [ ] Example app runs successfully
- [ ] No TODOs in production code
- [ ] No debug print statements

## Documentation

- [ ] README.md has installation instructions
- [ ] README.md has usage examples
- [ ] README.md lists platform support
- [ ] Code has documentation comments
- [ ] CHANGELOG.md is updated

## Testing

- [ ] Unit tests exist and pass
- [ ] Example app works on at least one platform
- [ ] No runtime errors in example
- [ ] All features are tested

## Pre-Publish Verification

- [ ] `dart pub publish --dry-run` succeeds
- [ ] No warnings in dry-run output
- [ ] All files are included correctly
- [ ] No sensitive files included (.env, keys, etc.)

## GitHub (Recommended)

- [ ] Repository created on GitHub
- [ ] Code pushed to GitHub
- [ ] README.md has pub.dev badge
- [ ] Repository is public

## Final Steps

- [ ] pub.dev account created
- [ ] Google account linked
- [ ] Ready to publish!

---

## Quick Commands

```bash
# Format code
dart format .

# Analyze code
flutter analyze

# Run tests
flutter test

# Dry run
dart pub publish --dry-run

# Publish (when ready)
dart pub publish
```

---

## After Publishing

- [ ] Verify package appears on pub.dev
- [ ] Test installation: `flutter pub add halo_videoplayer`
- [ ] Update README with pub.dev badge
- [ ] Share your package!

