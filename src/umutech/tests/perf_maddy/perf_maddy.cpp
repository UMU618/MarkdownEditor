#include <sstream>
#include <string>

#include <boost/nowide/iostream.hpp>
#include <boost/timer/timer.hpp>

#include <maddy/parser.h>

#include "umutech/tests/utilities/file.h"

namespace nw = boost::nowide;

std::string markdown2html_maddy(const std::string& markdown) {
  std::shared_ptr<maddy::ParserConfig> config =
      std::make_shared<maddy::ParserConfig>();
  config->enabledParsers &= ~maddy::types::EMPHASIZED_PARSER;
  config->enabledParsers |= maddy::types::HTML_PARSER;

  std::shared_ptr<maddy::Parser> parser =
      std::make_shared<maddy::Parser>(config);
  std::istringstream iss(markdown);
  return parser->Parse(iss);
}

int main(int argc, const char* argv[]) {
  if (argc < 2) {
    return EXIT_FAILURE;
  }
  nw::cout << "Maddy Performance Test\n";
  nw::cout << "=====================\n\n";

  std::string markdown_content = read_file(argv[1]);
  nw::cout << "Input file size: " << markdown_content.size() << " bytes\n\n";

  std::string html;
  {
    boost::timer::auto_cpu_timer t;
    html = markdown2html_maddy(markdown_content);
  }
  nw::cout << "\nOutput HTML size: " << html.size() << " bytes\n";
#if _DEBUG
  nw::cout << html;
#endif

  return 0;
}
