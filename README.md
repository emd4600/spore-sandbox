# Spore Sandbox

A random test mod for higher quality graphics. The mod contains experimental features that might affect other areas of the game, so use at your own risk.

To try it, you need the [https://github.com/emd4600/Spore-ModAPI](ModAPI SDK) and the [http://davoonline.com/sporemodder/rob55rod/ModAPI/Public/](Spore ModAPI Launcher Kit). Follow these instructions:
 1. Add `SporeSandbox_data` as an external project to [https://github.com/emd4600/SporeModder-FX](SporeModder FX), and pack it.
 2. You need to compile the latest version of the ModAPI DLLs, as [explained here](https://emd4600.github.io/Spore-ModAPI/_dev_d_l_ls.html). It's recommended you backup the original ones first.
 3. Edit `SandboxProject\SandboxProject.sln`. Find this line:
 ```
 Project("{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}") = "Spore ModAPI", "..\..\Spore ModAPI\Spore ModAPI.vcxproj", "{F057451A-1413-4D68-AF56-1BF529933420}"
 ```
 and change the `..\..\Spore ModAPI\Spore ModAPI.vcxproj` path to be the path to the ModAPI project in your installation of the SDK.
 4. Open `SandboxProject\SandboxProject.sln` in Visual Studio and compile it. 

To access the test mode, type `sb2` in the cheat console from the main galaxy screen. Controls:
 - Control the creature with `WASD`, or right-clicking to the place you want to go. There's a bug in which the creature doesn't animate with WASD if you haven't click-moved it first.
 - Change the position of the sun with `Ctrl + Left Mouse Click`, dragging the mouse.  This will change the color of the sky accordingly.
 - Press `O` to disable the ozone layer in the sky. This slightly changes the color of the sky.