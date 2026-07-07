# Home Indeed Installation

Home Indeed is an OBS Studio plugin. Install **OBS Studio** first, then download and run the installer for your operating system.

## Windows

1. Download the Windows installer: [home-indeed-1.1.0-windows-x64-Installer.exe](https://github.com/DeeThunder/Home-Indeed/releases/download/1.1.0/home-indeed-1.1.0-windows-x64-Installer.exe).
2. Run the `.exe` installer and follow the prompts.
3. The installer automatically detects your OBS Studio installation, places the plugin files in the correct directory, and configures the environment.
4. Restart OBS Studio after the installation is complete.

## macOS (Experimental)

> [!WARNING]
> macOS support is experimental and untested for v1.1.0.

1. Download `home-indeed-1.1.0-macos-universal.pkg` from the [Releases](https://github.com/DeeThunder/Home-Indeed/releases) page.
2. Open the `.pkg` installer and follow the prompts.
3. If macOS blocks the installation because the package is unsigned, open **System Settings > Privacy & Security**, scroll down, and click **Open Anyway**.
4. Restart OBS Studio.

## Ubuntu (Experimental)

> [!WARNING]
> Ubuntu/Linux support is experimental and untested for v1.1.0.

1. Download `home-indeed-1.1.0-x86_64-linux-gnu.deb` from the [Releases](https://github.com/DeeThunder/Home-Indeed/releases) page.
2. Install the Debian package with:
   ```bash
   sudo apt install ./home-indeed-1.1.0-x86_64-linux-gnu.deb
   ```
3. Restart OBS Studio.

## Verify

Open OBS Studio. You should see the **Home Indeed** options/docks in the OBS interface. If OBS Studio was open during installation, close it and open it again to load the plugin.
