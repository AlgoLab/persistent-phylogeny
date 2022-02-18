/** 
 * 
 * @file main.cpp
 * 
 * @author Simone Paolo Mottadelli
 * 
 */

#include <boost/program_options.hpp>
#include "rbgraph.hpp"
#include "functions.hpp"

int main(int argc, const char* argv[]) {
 // declare the vector of input files
  std::vector<std::string> files;

  // initialize options menu
  boost::program_options::options_description general_options(
      "Usage: ppp [OPTION...] FILE..."
      "\n\n"
      "Compute the PPP algorithm on the matrices in input. Note that the algorithm will be executed on the maximal reducible graphs generated from the input matrices."
      "\n\n"
      "Options");

  general_options.add_options()
      // option: help message
      ("help,h", "Display this message.\n")
      // option: verbose, print information on the ongoing operations
      ("verbose,v", boost::program_options::bool_switch(&logging::enabled),
       "Display the operations performed by the program.\n");
      // option: testpy, test reduce output with a python script

  // initialize hidden options (not shown in --help)
  boost::program_options::options_description hidden_options;
  // option: input files
  hidden_options.add_options()(
      "files", boost::program_options::value<std::vector<std::string>>(&files));

  // initialize positional options
  boost::program_options::positional_options_description positional_options;
  // add input files to the positional options
  positional_options.add("files", -1);

  // initialize options
  boost::program_options::options_description cmdline_options;
  // add the options menu and the hidden options to the available options
  cmdline_options.add(general_options).add(hidden_options);

  // initialize the variables map
  boost::program_options::variables_map vm;

  try {
    // parse the options given in input, and check them against positional and
    // cmdline options: store the result in vm
    boost::program_options::store(
        boost::program_options::command_line_parser(argc, argv)
            .positional(positional_options)
            .options(cmdline_options)
            .run(),
        vm);

    boost::program_options::notify(vm);
  } catch (const std::exception& e) {
    // error while parsing the options given in input
    std::cerr << "Error: " << e.what() << "." << std::endl
              << "Try '" << argv[0] << " --help' for more information."
              << std::endl;

    return 1;
  }

  if (vm.count("help")) {
    // help option specified
    std::cerr << general_options << std::endl;

    return 1;
  }

  if (!vm.count("files")) {
    // no input files specified
    std::cerr << "Error: No input file specified." << std::endl
              << "Try '" << argv[0] << " --help' for more information."
              << std::endl;

    return 1;
  }

  std::cout << "\n[INFO] This program executes the PPP algorithm on the matrices specified in input. Please note that the algorithm will be executed on the maximal reducible graphs generated from the input matrices. Future extensions of the program will allow to process general graphs too." << std::endl;

  std::cout << "[INFO] Starting..." << std::endl;
  if (files.size() > 1) {
    std::cout << "[INFO] Running PPP on " << files.size() << " files." << std::endl;
  }

  size_t count_file = 0;
  for (const auto& file : files) {

    count_file++;

    std::cout << "[INFO] Processing  \"" << file << "\"..." << std::endl;

    if (logging::enabled) {
      // verbosity enabled
      std::cout << "Reading the matrix from the file..." << std::endl;
    }

    RBGraph g;

    try {
      read_graph(file, g);
    } catch(std::runtime_error e) {
      std::cout << e.what() << std::endl;
      exit(0);
    }

    if (logging::enabled) {
      // verbosity enabled
      std::cout << "[INFO] Extracting the maximal reducible graph..."  << std::endl;
    }

    //RBGraph gm;
    //maximal_reducible_graph(g, gm, false);

    if (logging::enabled) {
      // verbosity enabled
      std::cout << "[INFO] Executing the PPP algorithm on the extracted maximal reducible graph..." << std::endl;
    }

    bool successfully_reduced = false;
    std::list<SignedCharacter> realized_characters;

    try {
      // realized_characters = ppp_maximal_reducible_graphs(gm); // also uncomment lines 121 e 122 
      realized_characters = ppp_maximal_reducible_graphs(g);
      successfully_reduced = true;
    } catch(...) {
      successfully_reduced = false;
    }

    if (!successfully_reduced) {
      std::cout << "[NO] The graph can not be reduced!" << std::endl;
    } else {
      std::cout << "[OK] The graph has been successfully reduced! The realized characters are: ";
      std::cout << "<";
      for (SignedCharacter sc : realized_characters)
        std::cout << "(" << sc << ") ";
      std::cout << ">" << std::endl;
    }

    std::cout << "[INFO] Processing  \"" << file << "\"... DONE!\n" << std::endl;
  }

  std::cout << "[INFO] Finished processing the input files. Terminated successfully." << std::endl;

  return 0;
}
