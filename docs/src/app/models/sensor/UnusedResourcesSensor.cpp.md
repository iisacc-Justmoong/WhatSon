# `src/app/models/sensor/UnusedResourcesSensor.cpp`

## Responsibility
Implements the unused-resource scan over unpacked hub files.

## Scan Strategy
1. Validate that `hubPath` points at an unpacked `.wshub` directory.
2. Traverse note `.wsnbody` files, including hidden system roots such as `.wscontents`, while still skipping unrelated
   hidden descendants.
3. Extract every embedded `<resource ... />` path from RAW note source.
4. Enumerate every `.wsresource` package under each hub resource root.
5. Return package descriptors whose relative resource path is absent from the embedded-resource lookup.

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
