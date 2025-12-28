# Adding "Max App Only (3.9MB)" Partition Scheme to Arduino IDE for ESP32

# NO NEED TO ADD A NEW PARITION ON PLATFORMIO
# This is for Arduino IDE only

This guide explains how to add a custom partition scheme to your Arduino IDE for ESP32 development, specifically one that maximizes application space to approximately 3.9MB.

**Prerequisites:**
* You have an ESP32 board support package installed in your Arduino IDE (this guide refers to version `2.0.17` as an example; paths might vary slightly for other versions).
* You have a custom partition file named `max_app_only.csv` with the following content:
    ```csv
    # Name,   Type, SubType, Offset,  Size,   Flags
    nvs,      data, nvs,     0x9000,  0x6000,
    phy_init, data, phy,     0xf000,  0x1000,
    factory,  app,  factory, 0x10000, 0x3F0000,
    ```

---

## Step 1: Modify `boards.txt` to Add the Menu Option

This step adds the new partition scheme to the Arduino IDE's "Partition Scheme" menu.

**A. Locate your `boards.txt` file:**

* **Windows:**
    ```
    C:\Users\{YourUsername}\AppData\Local\Arduino15\packages\esp32\hardware\esp32\{esp32_core_version}\boards.txt
    ```
    (Replace `{YourUsername}` and `{esp32_core_version}` e.g., `2.0.17`)

* **macOS:**
    1.  Open Finder.
    2.  Press `Shift + Command + .` (period) to show hidden files.
    3.  Navigate to:
        ```
        /Users/{YourUsername}/Library/Arduino15/packages/esp32/hardware/esp32/{esp32_core_version}/boards.txt
        ```
        (Replace `{YourUsername}` and `{esp32_core_version}` e.g., `2.0.17`)

**B. Edit `boards.txt`:**

1.  Open the `boards.txt` file in a text editor.
2.  Search for the section defining the partition schemes. It usually starts with a line similar to `esp32.menu.PartitionScheme.default=Default 4MB with spiffs ...`.
3.  Add the following lines to this section to define your new scheme:

    ```ini
    esp32.menu.PartitionScheme.max_app_only=Max App Only (3.9MB App)
    esp32.menu.PartitionScheme.max_app_only.build.partitions=max_app_only
    esp32.menu.PartitionScheme.max_app_only.upload.maximum_size=4128768
    ```
4.  Save the `boards.txt` file.

---

## Step 2: Add Your Custom Partition File (`.csv`)

This step places your custom partition layout file where the Arduino IDE can find it.

**A. Locate the `partitions` folder:**

* **Windows:**
    ```
    C:\Users\{YourUsername}\AppData\Local\Arduino15\packages\esp32\hardware\esp32\{esp32_core_version}\tools\partitions\
    ```
    (Replace `{YourUsername}` and `{esp32_core_version}`)

* **macOS:**
    ```
    /Users/{YourUsername}/Library/Arduino15/packages/esp32/hardware/esp32/{esp32_core_version}/tools/partitions/
    ```
    (Replace `{YourUsername}` and `{esp32_core_version}`)

**B. Copy your `.csv` file:**

1.  Copy your `max_app_only.csv` file into this `partitions` folder.

---

## Step 3: Clear Arduino IDE Cache (If Necessary)

Sometimes, the Arduino IDE caches board information. If the new partition scheme doesn't appear after restarting, clearing the cache might help. **Caution: This may remove other cached data.**

* **Windows:** Delete the contents of or the folder itself:
    ```
    C:\Users\{YourUsername}\AppData\Roaming\arduino-ide\
    ```
    (Replace `{YourUsername}`)

* **macOS:** Delete the contents of or the folder itself:
    ```
    /Users/{YourUsername}/Library/Application Support/Arduino-ide/
    ```
    (Replace `{YourUsername}`)

---

## Step 4: Restart Arduino IDE

Close and reopen the Arduino IDE. Your new partition scheme "Max App Only (3.9MB App)" should now be available under `Tools > Partition Scheme` when an ESP32 board is selected.

---