#include <iostream>
#include <string>

#include <gsl/util>

#include <boost/timer/timer.hpp>

#include <cmark-gfm-core-extensions.h>
#include <cmark-gfm.h>

#include "umutech/tests/utilities/file.h"

std::string markdown2html_cmark_gfm(const std::string& markdown) {
  cmark_gfm_core_extensions_ensure_registered();

  int parser_option{CMARK_OPT_DEFAULT};
  int render_option{CMARK_OPT_UNSAFE};

  std::string html;
  cmark_parser* parser = cmark_parser_new(parser_option);
  if (!parser) {
    return html;
  }
  gsl::final_action free_parser{[parser] { cmark_parser_free(parser); }};

  if (auto table_ext = cmark_find_syntax_extension("footnotes")) {
    cmark_parser_attach_syntax_extension(parser, table_ext);
  }
  if (auto table_ext = cmark_find_syntax_extension("strikethrough")) {
    cmark_parser_attach_syntax_extension(parser, table_ext);
  }
  if (auto table_ext = cmark_find_syntax_extension("table")) {
    cmark_parser_attach_syntax_extension(parser, table_ext);
  }
  if (auto tasklist_ext = cmark_find_syntax_extension("tasklist")) {
    cmark_parser_attach_syntax_extension(parser, tasklist_ext);
  }

  cmark_parser_feed(parser, markdown.data(), markdown.size());

  cmark_node* document = cmark_parser_finish(parser);
  if (!document) {
    return html;
  }
  gsl::final_action free_document{[document] { cmark_node_free(document); }};

  cmark_llist* extensions = cmark_parser_get_syntax_extensions(parser);
  if (char* html_output =
          cmark_render_html(document, render_option, extensions)) {
    html.assign(html_output);
  }
  return html;
}

int main(int argc, const char* argv[]) {
  if (argc < 2) {
    return EXIT_FAILURE;
  }
  std::cout << "CMark-GFM Performance Test\n";
  std::cout << "==========================\n\n";

  std::string markdown_content = read_file(argv[1]);
  std::cout << "Input file size: " << markdown_content.size() << " bytes\n\n";

  std::string html;
  {
    boost::timer::auto_cpu_timer t;
    html = markdown2html_cmark_gfm(markdown_content);
  }

  std::cout << "\nOutput HTML size: " << html.size() << " bytes\n";
  // std::cout << html;

  return 0;
}
