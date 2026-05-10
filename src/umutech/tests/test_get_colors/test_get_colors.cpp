#define STRICT 1
#define VC_EXTRALEAN
#define WINVER 0x0601
#define _ATL_NO_AUTOMATIC_NAMESPACE
#define _WTL_NO_AUTOMATIC_NAMESPACE
#define _WIN32_WINNT 0x0601
#define _WIN32_IE 0x0700

#include <cassert>
#include <cstdlib>
#include <iostream>
#include <string>

#include <atlbase.h>

#include <atlapp.h>

#include <atlcrack.h>
#include <atlstr.h>
#include <atlwin.h>

#include <ExDisp.h>
#include <mshtml.h>

#include <simdutf.h>
#include <gsl/util>

#include <dry/types.h>
#include <dry/win/tstring.h>

#include "umutech/tests/utilities/file.h"

#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "oleaut32.lib")
#pragma comment(lib, "uuid.lib")

using namespace std::string_view_literals;

WTL::CAppModule _Module;

class TestWindow : public ATL::CWindowImpl<TestWindow> {
 public:
  DECLARE_WND_CLASS(L"TestGetColorsWindow")

  BEGIN_MSG_MAP_EX(TestWindow)
    MSG_WM_CREATE(OnCreate)
    MSG_WM_DESTROY(OnDestroy)
    MSG_WM_TIMER(OnTimer)
    MESSAGE_HANDLER(WM_USER + 1, OnHtmlLoaded)
  END_MSG_MAP()

  HRESULT GetColorsFromHtml(const std::string& html,
                            COLORREF& text_color,
                            COLORREF& bg_color) noexcept {
    text_color = RGB(0, 0, 0);
    bg_color = RGB(255, 255, 255);

    html_ = html;
    text_color_ = &text_color;
    bg_color_ = &bg_color;

    RECT rc{0, 0, 100, 100};
    if (!Create(nullptr, rc, L"TestGetColors", WS_OVERLAPPEDWINDOW)) {
      return E_FAIL;
    }

    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0)) {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }

    return S_OK;
  }

 private:
  LRESULT OnCreate(LPCREATESTRUCT) noexcept {
    dry::win::tstring clsid_string;
    clsid_string.resize(38);
    const CLSID& clsid = CLSID_HTMLDocument;
    int size =
        ::StringFromGUID2(clsid, clsid_string.data(),
                          gsl::narrow_cast<int>(clsid_string.size() + 1));
    if (size) {
      clsid_string.resize(size - 1);
    } else {
      clsid_string.assign(L"{25336920-03F9-11cf-8FD0-00AA00686F13}");
    }

    if (!ms_html_.Create(
            m_hWnd, rcDefault, clsid_string.c_str(),
            WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
            WS_EX_CLIENTEDGE)) {
      PostQuitMessage(1);
      return -1;
    }

    ATL::CComQIPtr<IHTMLDocument2> doc;
    HRESULT hr = ms_html_.QueryControl(&doc);
    if (FAILED(hr)) {
      PostQuitMessage(1);
      return -1;
    }

    size_t utf16_length = simdutf::utf16_length_from_utf8(html_);
    ATL::CStringW html_wide;
    size_t written = simdutf::convert_utf8_to_utf16(
        html_.data(), html_.size(),
        reinterpret_cast<char16_t*>(
            html_wide.GetBufferSetLength(gsl::narrow_cast<int>(utf16_length))));
    if (written < utf16_length) {
      html_wide.ReleaseBufferSetLength(gsl::narrow_cast<int>(written));
    }

    SAFEARRAY* safe_array = ::SafeArrayCreateVector(VT_VARIANT, 0, 1);
    if (!safe_array) {
      PostQuitMessage(1);
      return -1;
    }

    VARIANT* var{};
    if (hr = ::SafeArrayAccessData(safe_array, (LPVOID*)&var);
        SUCCEEDED(hr) && var) {
      ::VariantInit(var);
      if (var->bstrVal = html_wide.AllocSysString(); var->bstrVal) {
        var->vt = VT_BSTR;
        ::SafeArrayUnaccessData(safe_array);
        hr = doc->write(safe_array);
        doc->close();
      }
    }
    ::SafeArrayDestroy(safe_array);

    if (FAILED(hr)) {
      PostQuitMessage(1);
      return -1;
    }

    PostMessage(WM_USER + 1);
    return 0;
  }

  void OnDestroy() noexcept { PostQuitMessage(0); }

  LRESULT OnHtmlLoaded(UINT, WPARAM, LPARAM, BOOL&) noexcept {
    SetTimer(1, 100, nullptr);
    return 0;
  }

  void OnTimer(UINT_PTR id) noexcept {
    KillTimer(id);

    ATL::CComQIPtr<IHTMLDocument2> doc;
    HRESULT hr = ms_html_.QueryControl(&doc);
    if (FAILED(hr) || !doc) {
      SetTimer(1, 50, nullptr);
      return;
    }

    ATL::CComBSTR ready_state;
    if (FAILED(doc->get_readyState(&ready_state)) || !ready_state) {
      SetTimer(1, 50, nullptr);
      return;
    }

    if (wcscmp(ready_state, L"complete") != 0) {
      SetTimer(1, 50, nullptr);
      return;
    }

#if 1
    ATL::CComPtr<IHTMLElement> body;
    if (SUCCEEDED(doc->get_body(&body)) && body) {
      ATL::CComVariant text_color_var;
      // text_color_var.vt = VT_UINT;
      if (SUCCEEDED(
              body->getAttribute(ATL::CComBSTR(L"text"), 0, &text_color_var))) {
        if (text_color_var.vt == VT_BSTR && text_color_var.bstrVal) {
          *text_color_ = ParseColor(text_color_var.bstrVal);
        }
      }

      ATL::CComVariant bg_color_var;
      if (SUCCEEDED(body->getAttribute(ATL::CComBSTR(L"bgcolor"), 0,
                                       &bg_color_var))) {
        if (bg_color_var.vt == VT_BSTR && bg_color_var.bstrVal) {
          *bg_color_ = ParseColor(bg_color_var.bstrVal);
        }
      }

      ATL::CComQIPtr<IHTMLElement2> body2;
      if (SUCCEEDED(body->QueryInterface(&body2)) && body2) {
        ATL::CComPtr<IHTMLCurrentStyle> current_style;
        if (SUCCEEDED(body2->get_currentStyle(&current_style)) &&
            current_style) {
          ATL::CComVariant css_color;
          if (SUCCEEDED(current_style->get_color(&css_color))) {
            if (css_color.vt == VT_BSTR && css_color.bstrVal) {
              *text_color_ = ParseColor(css_color.bstrVal);
            }
          }

          ATL::CComVariant css_bg_color;
          if (SUCCEEDED(current_style->get_backgroundColor(&css_bg_color))) {
            if (css_bg_color.vt == VT_BSTR && css_bg_color.bstrVal) {
              *bg_color_ = ParseColor(css_bg_color.bstrVal);
            }
          }
        }
      }
    }
#else
    ATL::CComPtr<IDisplayServices> display;
    hr = doc.QueryInterface(&display);
    if (FAILED(hr) || !display) {
      return;
    }

    ATL::CComPtr<IHTMLComputedStyle> style;
    hr = display->GetComputedStyle(nullptr, &style);
    if (FAILED(hr) || !style) {
      return;
    }
#endif
    DestroyWindow();
  }

  static COLORREF ParseColor(BSTR bstr) noexcept {
    std::wstring_view color{bstr, ::SysStringLen(bstr)};

    if (color.empty()) {
      return RGB(0, 0, 0);
    }

    std::wstring lower_case;
    lower_case.resize(color.size());
    std::transform(
        color.begin(), color.end(), lower_case.begin(),
        [](wchar_t c) { return gsl::narrow_cast<wchar_t>(std::tolower(c)); });

    if (lower_case[0] == L'#') {
      std::wstring_view hex{lower_case};
      hex.remove_prefix(1);
      if (hex.size() == 3) {
        int r = HexDigit(hex[0]);
        int g = HexDigit(hex[1]);
        int b = HexDigit(hex[2]);
        return RGB(r * 16 + r, g * 16 + g, b * 16 + b);
      } else if (hex.size() == 6) {
        int r = HexDigit(hex[0]) * 16 + HexDigit(hex[1]);
        int g = HexDigit(hex[2]) * 16 + HexDigit(hex[3]);
        int b = HexDigit(hex[4]) * 16 + HexDigit(hex[5]);
        return RGB(r, g, b);
      }
    }

    if (lower_case.starts_with(L"rgb(") && lower_case.ends_with(L')')) {
      int r = 0, g = 0, b = 0;
      if (swscanf_s(lower_case.data(), L"rgb(%d, %d, %d)", &r, &g, &b) == 3) {
        return RGB(r, g, b);
      }
    }

    static const struct {
      std::wstring_view name;
      COLORREF color;
    } kNamedColors[] = {
        {L"aliceblue"sv, RGB(240, 248, 255)},
        {L"antiquewhite"sv, RGB(250, 235, 215)},
        {L"aqua"sv, RGB(0, 255, 255)},
        {L"aquamarine"sv, RGB(127, 255, 212)},
        {L"azure"sv, RGB(240, 255, 255)},
        {L"beige"sv, RGB(245, 245, 220)},
        {L"bisque"sv, RGB(255, 228, 196)},
        {L"black"sv, RGB(0, 0, 0)},
        {L"blanchedalmond"sv, RGB(255, 235, 205)},
        {L"blue"sv, RGB(0, 0, 255)},
        {L"blueviolet"sv, RGB(138, 43, 226)},
        {L"brown"sv, RGB(165, 42, 42)},
        {L"burlywood"sv, RGB(222, 184, 135)},
        {L"cadetblue"sv, RGB(95, 158, 160)},
        {L"chartreuse"sv, RGB(127, 255, 0)},
        {L"chocolate"sv, RGB(210, 105, 30)},
        {L"coral"sv, RGB(255, 127, 80)},
        {L"cornflowerblue"sv, RGB(100, 149, 237)},
        {L"cornsilk"sv, RGB(255, 248, 220)},
        {L"crimson"sv, RGB(220, 20, 60)},
        {L"cyan"sv, RGB(0, 255, 255)},
        {L"darkblue"sv, RGB(0, 0, 139)},
        {L"darkcyan"sv, RGB(0, 139, 139)},
        {L"darkgoldenrod"sv, RGB(184, 134, 11)},
        {L"darkgray"sv, RGB(169, 169, 169)},
        {L"darkgreen"sv, RGB(0, 100, 0)},
        {L"darkgrey"sv, RGB(169, 169, 169)},
        {L"darkkhaki"sv, RGB(189, 183, 107)},
        {L"darkmagenta"sv, RGB(139, 0, 139)},
        {L"darkolivegreen"sv, RGB(85, 107, 47)},
        {L"darkorange"sv, RGB(255, 140, 0)},
        {L"darkorchid"sv, RGB(153, 50, 204)},
        {L"darkred"sv, RGB(139, 0, 0)},
        {L"darksalmon"sv, RGB(233, 150, 122)},
        {L"darkseagreen"sv, RGB(143, 188, 143)},
        {L"darkslateblue"sv, RGB(72, 61, 139)},
        {L"darkslategray"sv, RGB(47, 79, 79)},
        {L"darkslategrey"sv, RGB(47, 79, 79)},
        {L"darkturquoise"sv, RGB(0, 206, 209)},
        {L"darkviolet"sv, RGB(148, 0, 211)},
        {L"deeppink"sv, RGB(255, 20, 147)},
        {L"deepskyblue"sv, RGB(0, 191, 255)},
        {L"dimgray"sv, RGB(105, 105, 105)},
        {L"dimgrey"sv, RGB(105, 105, 105)},
        {L"dodgerblue"sv, RGB(30, 144, 255)},
        {L"firebrick"sv, RGB(178, 34, 34)},
        {L"floralwhite"sv, RGB(255, 250, 240)},
        {L"forestgreen"sv, RGB(34, 139, 34)},
        {L"fuchsia"sv, RGB(255, 0, 255)},
        {L"gainsboro"sv, RGB(220, 220, 220)},
        {L"ghostwhite"sv, RGB(248, 248, 255)},
        {L"gold"sv, RGB(255, 215, 0)},
        {L"goldenrod"sv, RGB(218, 165, 32)},
        {L"gray"sv, RGB(128, 128, 128)},
        {L"green"sv, RGB(0, 128, 0)},
        {L"greenyellow"sv, RGB(173, 255, 47)},
        {L"grey"sv, RGB(128, 128, 128)},
        {L"honeydew"sv, RGB(240, 255, 240)},
        {L"hotpink"sv, RGB(255, 105, 180)},
        {L"indianred"sv, RGB(205, 92, 92)},
        {L"indigo"sv, RGB(75, 0, 130)},
        {L"ivory"sv, RGB(255, 255, 240)},
        {L"khaki"sv, RGB(240, 230, 140)},
        {L"lavender"sv, RGB(230, 230, 250)},
        {L"lavenderblush"sv, RGB(255, 240, 245)},
        {L"lawngreen"sv, RGB(124, 252, 0)},
        {L"lemonchiffon"sv, RGB(255, 250, 205)},
        {L"lightblue"sv, RGB(173, 216, 230)},
        {L"lightcoral"sv, RGB(240, 128, 128)},
        {L"lightcyan"sv, RGB(224, 255, 255)},
        {L"lightgoldenrodyellow"sv, RGB(250, 250, 210)},
        {L"lightgray"sv, RGB(211, 211, 211)},
        {L"lightgreen"sv, RGB(144, 238, 144)},
        {L"lightgrey"sv, RGB(211, 211, 211)},
        {L"lightpink"sv, RGB(255, 182, 193)},
        {L"lightsalmon"sv, RGB(255, 160, 122)},
        {L"lightseagreen"sv, RGB(32, 178, 170)},
        {L"lightskyblue"sv, RGB(135, 206, 250)},
        {L"lightslategray"sv, RGB(119, 136, 153)},
        {L"lightslategrey"sv, RGB(119, 136, 153)},
        {L"lightsteelblue"sv, RGB(176, 196, 222)},
        {L"lightyellow"sv, RGB(255, 255, 224)},
        {L"lime"sv, RGB(0, 255, 0)},
        {L"limegreen"sv, RGB(50, 205, 50)},
        {L"linen"sv, RGB(250, 240, 230)},
        {L"magenta"sv, RGB(255, 0, 255)},
        {L"maroon"sv, RGB(128, 0, 0)},
        {L"mediumaquamarine"sv, RGB(102, 205, 170)},
        {L"mediumblue"sv, RGB(0, 0, 205)},
        {L"mediumorchid"sv, RGB(186, 85, 211)},
        {L"mediumpurple"sv, RGB(147, 112, 219)},
        {L"mediumseagreen"sv, RGB(60, 179, 113)},
        {L"mediumslateblue"sv, RGB(123, 104, 238)},
        {L"mediumspringgreen"sv, RGB(0, 250, 154)},
        {L"mediumturquoise"sv, RGB(72, 209, 204)},
        {L"mediumvioletred"sv, RGB(199, 21, 133)},
        {L"midnightblue"sv, RGB(25, 25, 112)},
        {L"mintcream"sv, RGB(245, 255, 250)},
        {L"mistyrose"sv, RGB(255, 228, 225)},
        {L"moccasin"sv, RGB(255, 228, 181)},
        {L"navajowhite"sv, RGB(255, 222, 173)},
        {L"navy"sv, RGB(0, 0, 128)},
        {L"oldlace"sv, RGB(253, 245, 230)},
        {L"olive"sv, RGB(128, 128, 0)},
        {L"olivedrab"sv, RGB(107, 142, 35)},
        {L"orange"sv, RGB(255, 165, 0)},
        {L"orangered"sv, RGB(255, 69, 0)},
        {L"orchid"sv, RGB(218, 112, 214)},
        {L"palegoldenrod"sv, RGB(238, 232, 170)},
        {L"palegreen"sv, RGB(152, 251, 152)},
        {L"paleturquoise"sv, RGB(175, 238, 238)},
        {L"palevioletred"sv, RGB(219, 112, 147)},
        {L"papayawhip"sv, RGB(255, 239, 213)},
        {L"peachpuff"sv, RGB(255, 218, 185)},
        {L"peru"sv, RGB(205, 133, 63)},
        {L"pink"sv, RGB(255, 192, 203)},
        {L"plum"sv, RGB(221, 160, 221)},
        {L"powderblue"sv, RGB(176, 224, 230)},
        {L"purple"sv, RGB(128, 0, 128)},
        {L"rebeccapurple"sv, RGB(102, 51, 153)},
        {L"red"sv, RGB(255, 0, 0)},
        {L"rosybrown"sv, RGB(188, 143, 143)},
        {L"royalblue"sv, RGB(65, 105, 225)},
        {L"saddlebrown"sv, RGB(139, 69, 19)},
        {L"salmon"sv, RGB(250, 128, 114)},
        {L"sandybrown"sv, RGB(244, 164, 96)},
        {L"seagreen"sv, RGB(46, 139, 87)},
        {L"seashell"sv, RGB(255, 245, 238)},
        {L"sienna"sv, RGB(160, 82, 45)},
        {L"silver"sv, RGB(192, 192, 192)},
        {L"skyblue"sv, RGB(135, 206, 235)},
        {L"slateblue"sv, RGB(106, 90, 205)},
        {L"slategray"sv, RGB(112, 128, 144)},
        {L"slategrey"sv, RGB(112, 128, 144)},
        {L"snow"sv, RGB(255, 250, 250)},
        {L"springgreen"sv, RGB(0, 255, 127)},
        {L"steelblue"sv, RGB(70, 130, 180)},
        {L"tan"sv, RGB(210, 180, 140)},
        {L"teal"sv, RGB(0, 128, 128)},
        {L"thistle"sv, RGB(216, 191, 216)},
        {L"tomato"sv, RGB(255, 99, 71)},
        {L"turquoise"sv, RGB(64, 224, 208)},
        {L"violet"sv, RGB(238, 130, 238)},
        {L"wheat"sv, RGB(245, 222, 179)},
        {L"white"sv, RGB(255, 255, 255)},
        {L"whitesmoke"sv, RGB(245, 245, 245)},
        {L"yellow"sv, RGB(255, 255, 0)},
        {L"yellowgreen"sv, RGB(154, 205, 50)},
    };
#if _DEBUG
    for (std::size_t i = 1; i < std::size(kNamedColors); ++i) {
      assert(kNamedColors[i - 1].name < kNamedColors[i].name &&
             "Named colors must be sorted for binary search");
    }
#endif
    for (const auto& named : kNamedColors) {
      int e = lower_case.compare(named.name);
      if (0 < e) {
        continue;
      }
      if (0 == e) {
        return named.color;
      }
      assert(!"Unknown color!");
      break;
    }

    return RGB(0, 0, 0);
  }

  static int HexDigit(wchar_t ch) noexcept {
    if (ch >= L'0' && ch <= L'9')
      return ch - L'0';
    if (ch >= L'a' && ch <= L'f')
      return ch - L'a' + 10;
    return 0;
  }

 private:
  ATL::CAxWindow ms_html_;
  std::string html_;
  COLORREF* text_color_{};
  COLORREF* bg_color_{};
};

int main(int argc, const char* argv[]) {
  if (argc < 2) {
    std::cerr << "Usage: test_get_colors <html_file>\n";
    return EXIT_FAILURE;
  }

  std::string html = read_file(argv[1]);
  if (html.empty()) {
    return EXIT_FAILURE;
  }

  HRESULT hr = ::CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
  if (FAILED(hr)) {
    std::cerr << "Failed to initialize COM: 0x" << std::hex << hr << '\n';
    return EXIT_FAILURE;
  }
  gsl::final_action uninit_com{[] { ::CoUninitialize(); }};

  hr = _Module.Init(nullptr, ::GetModuleHandle(nullptr));
  if (FAILED(hr)) {
    std::cerr << "Failed to initialize ATL module: 0x" << std::hex << hr
              << '\n';
    return EXIT_FAILURE;
  }
  gsl::final_action uninit_module{[] { _Module.Term(); }};

  COLORREF text_color{};
  COLORREF bg_color{};

  TestWindow window;
  hr = window.GetColorsFromHtml(html, text_color, bg_color);
  if (FAILED(hr)) {
    std::cerr << "Failed to get colors: 0x" << std::hex << hr << '\n';
    return EXIT_FAILURE;
  }

  std::cout << "TextColor: " << (int)GetRValue(text_color) << ", "
            << (int)GetGValue(text_color) << ", " << (int)GetBValue(text_color)
            << '\n';
  std::cout << "BgColor: " << (int)GetRValue(bg_color) << ", "
            << (int)GetGValue(bg_color) << ", " << (int)GetBValue(bg_color)
            << '\n';

  return 0;
}
