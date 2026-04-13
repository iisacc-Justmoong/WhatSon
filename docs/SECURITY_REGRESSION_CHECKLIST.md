# Security Regression Checklist

This repository does not operate automated or scripted tests. Use this checklist as manual review documentation when
inspecting trial/auth and resource-import changes.

## Trial/Auth
- Plain `QSettings` value `register/authenticated=true` must not unlock a trial build by itself.
- A signed authenticated record created on one machine must not verify on a different machine with a different integrity secret.
- Legacy plain trial identity values in `QSettings` must migrate into the secure store and be removed from `QSettings`.
- Missing or unavailable secure-store signing material must fail closed for trial authentication and signed install-date verification.

## Trial Install Date
- A tampered signed install-date record must not extend the trial window.
- A valid legacy secure-store install date must migrate into the signed `QSettings` format once signing material is available.

## Resources Import
- If package creation fails mid-import, all created `.wsresource` packages from that turn must be removed.
- If the runtime refresh callback fails after `Resources.wsresources` is written, the importer must restore the previous `Resources.wsresources` contents and remove created packages.
- Editor drag/drop imports must keep using the same rollback path as menu imports; the drag entrypoint must not append
  `Resources.wsresources` or create `.wsresource` packages through a bypass path.

## Hub Mutation Path
- Hub mutations must not create or refresh `.whatson/write-lease.json`; the repository no longer uses a write-lease side channel.
