# French ROMs

This fork adds French revision 0 profiles for the same seven games as upstream.
It does not add Pearl or the Mystery Dungeon games. Use your own extracted `.nds`
files; changing a filename or editing its game code does not change its version.

| Game | French code | Save in the ROM folder's `saves` directory |
|---|---|---|
| Black | IRBF | black-fr.sav |
| White | IRAF | white-fr.sav |
| Black 2 | IREF | black2-fr.sav |
| White 2 | IRDF | white2-fr.sav |
| HeartGold | IPKF | heartgold-fr.sav |
| SoulSilver | IPGF | soulsilver-fr.sav |
| Platinum | CPUF | platinum-fr.sav |

The Android app is **Pokemon DS FR**. Install its APK from this fork's releases,
open it and select the folder containing your ROMs. French games have `(FR)` in
the list. The original English profiles remain available. French and English
versions keep separate saves, so adding one cannot replace the other's save.
To import an existing French save, first back it up, then copy it under the name
in the table above. A raw `.sav` is required, as explained in the main README.

## What changed

Recognising the French header is only the first part. The screens, touch/controller
routing and widescreen map hooks use function addresses, which differ between
versions. The French profiles were compared with the supported English ROMs.
SoulSilver and HeartGold each have 195 application/control probe addresses.
The other five games use their own function tables. The four Black/White games
also have separately matched map and heap tables for wide map loading.

Platinum's introduction detects the coloured OUI/NON button areas rather than
pixels through their lettering, so the merged view and controller answers work
with French labels. A touches OUI; B touches NON.

## Validation and limits

All seven French ROMs were exercised with the JIT in local headless runs:
startup, introductions and scene changes. The tested runs reached the overworld
in SoulSilver, HeartGold, Black, Black 2 and White 2; White reached naming, and
Platinum's touch-only introduction was checked separately. These checks are not
a full playthrough of every map, battle, minigame or endgame screen.

Black 2 and White 2 were also checked with the French patched copies supplied
for testing. This does not establish compatibility with arbitrary ROM hacks or
other revisions. When reporting a problem, include the game code/revision, the
APK version and `game.log` from `Android/data/io.github.tibkpl.pokemonds/files`.

## Reproducing the profiles

The tools read personally supplied reference and French ROMs locally. Their
output contains addresses only, never game instructions. Keep ROMs, generated
game code, saves, snapshots, APKs and build output outside version control.

For Black, White, Black 2, White 2 and Platinum:

```sh
python tools/regions/match_functions.py reference.nds french.nds english-functions.txt build/french-profile.json
```

Use IRBO, IRAO, IREO and IRDO revision 0 references with their matching function
tables, or **CPUE revision 1** with `tools/platinum/CPUE.txt`. The tool refuses
unresolved function matches and ambiguous map/heap table matches. It normalises
branch and literal-pool offsets and uses matching neighbours for small wrappers.
The build's English tables are in `build-android-data/funcs`.

For SoulSilver and HeartGold, use the SoulSilver USA reference whose hash is
recorded in `layouts/soulsilver-us.json`:

```sh
python tools/soulsilver/match_french.py french.nds build/french.json --reference soulsilver-us.nds
```

The public build uses only layouts and address profiles. No reference ROM is
needed by GitHub Actions. See [Android releases](ANDROID-RELEASES.md).
