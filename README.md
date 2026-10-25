# UMUTech.MarkdownEditor

## Usage

This is a green software that you can freely copy and use anywhere.

### 1. Command Line Options

You can open files as preview mode via command line option `--mode=preview`.

By passing the "--singleton" option, the instance can be set to the singleton mode. In this case, any new instances opened later will activate this instance and then exit.

### 2. Shortcut Keys

- Ctrl+E: Toggle Editor
- Ctrl+F4/Ctrl+W: Close Active Document
- Ctrl+Tab: Next Tab
- Shift+Ctrl+Tab: Previous Tab

### 3. Note

- Only files encoded in UTF-8 are accepted.
- Line endings are **always LF**. CRLF will be replaced with LF. This means that if the current format is CRLF, a `*` flag will be added on tab header upon opening, indicating that the document has been modified.
- Can't open files larger then 4MB.

## Snapshots

![Preview](doc/preview.png)

![About](doc/about.png)
