#include <iostream>
#include <sstream>
#include <string>

#include <boost/timer/timer.hpp>
#include <gsl/util>

extern "C" {
#include <mkdio.h>
}

#include "umutech/tests/utilities/file.h"

std::string markdown2html_discount(const std::string& markdown) {
  MMIOT* doc =
      gfm_string(markdown.c_str(), gsl::narrow_cast<int>(markdown.size()), nullptr);
  mkd_compile(doc, nullptr);
  char* html;
  int size = mkd_document(doc, &html);
  std::string result;
  if (0 < size) {
    result.assign(html, size);
  }
  mkd_cleanup(doc);
  return result;
}

int main(int argc, const char* argv[]) {
  if (argc < 2) {
    return EXIT_FAILURE;
  }
  std::cout << "Discount Performance Test\n";
  std::cout << "=====================\n\n";

  std::string markdown_content = read_file(argv[1]);
  std::cout << "Input file size: " << markdown_content.size() << " bytes\n\n";

  std::string html;
  {
    boost::timer::auto_cpu_timer t;
    html = markdown2html_discount(markdown_content);
  }
  std::cout << "\nOutput HTML size: " << html.size() << " bytes\n";
  // std::cout << html;

  return 0;
}
