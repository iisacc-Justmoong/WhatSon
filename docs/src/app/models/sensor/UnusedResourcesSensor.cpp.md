# `src/app/models/sensor/UnusedResourcesSensor.cpp`

## Responsibility
Implements the unused-resource scan over unpacked hub files.

## Scan Strategy
1. Validate that `hubPath` points at an unpacked `.wshub` directory.
2. Enumerate every `.wsresource` package under each hub resource root.
3. Return package descriptors for the package inventory. Note body source is not scanned.

## Returned Entry Shape
- `resourcePath`
- `packageDirectoryPath`
- `packageName`
- `resourceId`
- `assetPath`
- `assetAbsolutePath`
- `annotationPath`
- `annotationAbsolutePath`
- `bucket`
- `type`
- `format`
- `metadataValid`
- `metadataError`

## Error Handling
- Invalid hub roots and unreadable note body files set `lastError` and clear the unused-resource list.
- Broken `resource.xml` metadata does not hide a package from the sensor. The implementation falls back to package-path
  inference so damaged packages still surface as unused resources instead of disappearing silently.
