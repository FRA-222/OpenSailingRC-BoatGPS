# M5Burner Custom Configuration Guide

The public publish feature in M5Burner's USER CUSTOM section does not support uploading separate manifest files directly. Custom configuration fields are only supported for official firmware builds, but you can use the following two workarounds to achieve custom yellow button configuration functionality:

## Option 1: Use local custom firmware (no publish required)

This method works for your own use and does not require sharing the firmware publicly:

1. Locate your local M5Burner custom firmware directory:
   - **Windows:** `C:\Users\<your username>\AppData\Roaming\M5Burner\custom`
   - **macOS:** `~/Library/Application Support/M5Burner/custom`
   - **Linux:** `~/.config/M5Burner/custom`

2. Create a new folder named after your firmware, place both your `.bin` file and a `config.json` manifest file (with your yellow button configuration fields) inside this folder

3. Restart M5Burner, and the firmware will appear directly in the USER CUSTOM list. When you click Configure for this firmware, your custom yellow button parameters will be available for editing, and values will be saved to the device's NVS as expected.

## Option 2: Embed default configurations into firmware + post-flash configuration

If you need to publish the firmware to share with others:

1. Compile your firmware with built-in default yellow button parameters
2. Add a simple configuration interface in your firmware (e.g., long-press the yellow button to enter configuration mode, use the screen to adjust action settings, or provide a web configuration page if WiFi is enabled)
3. When publishing via the USER CUSTOM publish window, only upload the `.bin` file, and note the configuration method in the firmware description.

## Important Note

If you want your custom firmware to support native M5Burner configuration fields in the public list, you need to submit your firmware to the official M5Stack firmware repository via a pull request to the UIFlow MicroPython repository, where the M5Stack team will add your configuration manifest to the official build pipeline.
