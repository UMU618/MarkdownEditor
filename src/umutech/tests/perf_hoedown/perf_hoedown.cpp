#include <iostream>
#include <string>

#include <boost/timer/timer.hpp>

#include "third_party/hoedown/src/document.h"
#include "third_party/hoedown/src/html.h"

#include "umutech/tests/utilities/file.h"

std::string markdown2html_hoedown(const std::string& markdown) {
  const size_t kInputUnit{1024};
  const size_t kOutputUnit{64};
  const size_t kMaxNesting{16};
  const auto kRenderFlags = static_cast<hoedown_html_flags>(0);
  const auto kExtensions = static_cast<hoedown_extensions>(
      HOEDOWN_EXT_TABLES | HOEDOWN_EXT_FENCED_CODE | HOEDOWN_EXT_AUTOLINK |
      HOEDOWN_EXT_STRIKETHROUGH | HOEDOWN_EXT_NO_INTRA_EMPHASIS |
      HOEDOWN_EXT_SPACE_HEADERS);

  std::string html;
  hoedown_renderer* md_renderer =
      hoedown_html_renderer_new(kRenderFlags, kMaxNesting);
  if (md_renderer) {
    hoedown_buffer* html_buffer = hoedown_buffer_new(kOutputUnit);
    hoedown_document* document =
        hoedown_document_new(md_renderer, kExtensions, kMaxNesting);
    hoedown_document_render(document, html_buffer,
                            reinterpret_cast<const uint8_t*>(markdown.data()),
                            markdown.size());
    hoedown_document_free(document);
    html.assign(reinterpret_cast<char*>(html_buffer->data), html_buffer->size);
    hoedown_buffer_free(html_buffer);
    hoedown_html_renderer_free(md_renderer);
  }
  return html;
}

int main(int argc, const char* argv[]) {
  if (argc < 2) {
    return EXIT_FAILURE;
  }
  std::cout << "Hoedown Performance Test\n";
  std::cout << "========================\n\n";

  std::string markdown_content = read_file(argv[1]);
  std::cout << "Input file size: " << markdown_content.size() << " bytes\n\n";

  std::string html;
  {
    boost::timer::auto_cpu_timer t;
    html = markdown2html_hoedown(markdown_content);
  }

  std::cout << "\nOutput HTML size: " << html.size() << " bytes\n";
  // std::cout << html;

  return 0;
}
