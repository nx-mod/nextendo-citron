# nextendo-citron (nx-mod testing-android)

nx-mod's `testing-android` fork of [citron-nextendo](https://github.com/NextendoNetwork/citron-nextendo), the
Nextendo Network emulator: the **Android** build nx-mod uses. Part of
[nextendo-testing](https://github.com/nx-mod/nextendo-testing): the whole Nextendo Network, run on a LAN.
Upstream's README is kept as [README.upstream.md](README.upstream.md).

## nx-mod changes

- **LAN play:** LDN over ldn_mitm's native wire protocol, and an Android LDN VPN tunnel for lan-play bridges;
  our own echoed datagrams dropped, failed sends logged, UDP allowed to fragment.
- **Nextendo:** `demonware.net` hosts sent to the Nextendo server (Diablo III); the account Mii synced from the
  profile page on Android; the `[Services]` config section kept on load.
- **CI:** Android APK builds for these branches.

ZeroTier support lives on the `zerotier-option` branch, not here. `testing-android` is 41 upstream commits
behind: merging upstream's `main` conflicts and is still to be resolved.

## Credits

citron-nextendo is the work of the **Nextendo Network team** — https://nextendo.network — on
[Citron](https://github.com/citron-neo/emulator). nx-mod only adds the changes above. Nextendo is awesome.
