#include <iostream>
#include <sstream>
#include <string>

#include <boost/timer/timer.hpp>

#include <md4c-html.h>

#include "umutech/tests/utilities/file.h"

std::string markdown2html_md4c(const std::string& markdown) {
  std::ostringstream ss;

  auto renderer = [](const MD_CHAR* text, MD_SIZE size, void* userdata) {
    auto ss = static_cast<std::ostringstream*>(userdata);
    *ss << std::string_view(text, size);
  };

  unsigned int parser_flags =
      MD_FLAG_TABLES | MD_FLAG_STRIKETHROUGH | MD_FLAG_PERMISSIVEURLAUTOLINKS |
      MD_FLAG_PERMISSIVEEMAILAUTOLINKS | MD_FLAG_PERMISSIVEWWWAUTOLINKS;

  unsigned int renderer_flags = 0;

  md_html(markdown.c_str(), static_cast<MD_SIZE>(markdown.size()), renderer,
          &ss, parser_flags, renderer_flags);

  return std::move(ss).str();
}

int main(int argc, const char* argv[]) {
  if (argc < 2) {
    return EXIT_FAILURE;
  }
  std::cout << "MD4C Performance Test\n";
  std::cout << "=====================\n\n";

  std::string markdown_content = read_file(argv[1]);
  std::cout << "Input file size: " << markdown_content.size() << " bytes\n\n";

  std::string html;
  {
    boost::timer::auto_cpu_timer t;
    html = markdown2html_md4c(markdown_content);
  }
  std::cout << "\nOutput HTML size: " << html.size() << " bytes\n";
  // std::cout << html;

  return 0;
}
