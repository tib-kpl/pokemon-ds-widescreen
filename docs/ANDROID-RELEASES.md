# Android releases of this fork

Every push to `main` builds an ARM64 APK for Android 7 or later and publishes
`v0.2.N`, where N is the workflow run number. Android uses version code
20000 + N, so later releases install over earlier releases without losing data.
Rerunning a workflow replaces the files of that run's release without changing its version.

The application is named **Pokemon DS FR** and installs separately from upstream
(`io.github.tibkpl.pokemonds`). Select the folder containing your own ROMs.
The build uses public address metadata; no ROM, save, state, or game code is uploaded.

The stable signing key is stored in the `ANDROID_SIGNING_KEY` repository secret
(the JSON format used by `tools/android/apk.py`). Keep a private backup of the key:
replacing it prevents Android from installing an update over the old application.

Local build (Linux / WSL):

```sh
ANDROID_SDK=/path/to/Android/Sdk \
ANDROID_NDK_HOME=/path/to/Android/Sdk/ndk/27.2.12479018 \
SSR_ANDROID_ABIS=arm64-v8a \
SSR_ANDROID_KEY=/private/path/android-signing.json \
SSR_VERSION=0.2.1 SSR_VERSION_CODE=20001 \
bash tools/android/build_all.sh
```

The APK is `build-android-dist/pokemon-ds-wide-android.apk`. Releases also include
`SHA256SUMS.txt` for verifying the download. Local and CI builds must use the same
key to update each other.
