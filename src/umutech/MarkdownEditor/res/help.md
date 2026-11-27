# UMUTech.MarkdownEditor

## 1. Features

- Multiple Markdown renderers: `cmark-gfm`, `hoextdown`, `lowdown`, `MD4C`.
- Only files encoded in UTF-8 are accepted.
- Line endings are always LF. CRLF will be replaced with LF. This means that if the current format is CRLF, a * flag will be added on tab header upon opening, indicating that the document has been modified.
- Can't open files larger then 4MB.

## 2. Command Line Options

| Name | Type | Option Values | Description |
|:-|:-|:-|:-|
| action | string | reg, unreg | reg: Register file association<br> unreg: Unregister file association |
| help | bool | | Show help page. |
| license | bool | | Show license page. |
| mode | string | preview | preview: Hide editor when open files. |
| singleton | bool | | In singleton mode, any new instances opened later will activate this instance and then exit. |

eg:

- `MarkdownEditor.exe --mode=preview --singleton`
- `MarkdownEditor.exe --help`
