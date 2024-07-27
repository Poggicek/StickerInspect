# Sticker Inspect

A simple tool that exposes a GUI allowing you to inspect workshop stickers on weapons.

## Setup

Download the latest release from the [releases page](https://github.com/Poggicek/StickerInspect/releases) and extract both `Launcher.exe` and `StickerInspect.dll` inside the same folder.

Change your launch options to include `-insecure` and launch the game in **tools** mode.

When the game loads, launch `Launcher.exe` which will automatically load the dll and show a new window.

## Usage

Whenever you start the game you have to inspect any sticker so that it internally creates a new items_game definition, then you press the `Refresh` button in the tool to load all the stickers.

Use [CS2Inspects website](https://cs2inspects.com/) to create an inspect link which includes any stickers on any position.

## Can I get VAC banned?

TLDR; no. While Sticker inspect does act as a cheat in the sense that it injects itself into the game to read and write memory it follows the same principles as [HLAE](https://github.com/advancedfx/advancedfx/wiki/FAQ#can-hlae-get-me-vac-banned), the launcher prevents the tool from loading if the game isn't in explicitly in both -insecure and -tools. You cannot get VAC banned for using this tool if you aren't connecting to VAC secured servers, which should be impossible with -insecure. If you are still unsure about the safety of this tool you can use a throwaway steam account.