#include <boost/program_options.hpp>

#include "qicli.hpp"
#include <qi/jsoncodec.hpp>

namespace po = boost::program_options;

std::map<std::string, SubCmd> subCmdMap;

void init()
{
  subCmdMap["info"] = &subCmd_info;
  subCmdMap["call"] = &subCmd_call;
  subCmdMap["post"] = &subCmd_post;
  subCmdMap["watch"] = &subCmd_watch;
  subCmdMap["get"] = &subCmd_get;
  subCmdMap["set"] = &subCmd_set;
  subCmdMap["trace"] = &subCmd_trace;
  subCmdMap["top"] = &subCmd_top;
  subCmdMap["log-view"] = &subCmd_logView;
  subCmdMap["log-send"] = &subCmd_logSend;
  subCmdMap["mod"] = &subCmd_mod;
}

int                 main(int argc, char **argv)
{
  qi::ApplicationSession app(argc, argv);
  int                    subCmdArgc = 0;
  char                   **subCmdArgv = nullptr;
  SubCmd                 subCmd = 0;

  init();
  for (int i = 0; i < argc; ++i)
  {
    std::map<std::string, SubCmd>::const_iterator it = subCmdMap.find(argv[i]);
    if (it != subCmdMap.end())
    {
      subCmdArgc = argc - i;
      subCmdArgv = &argv[i];
      subCmd = it->second;
      argc = i + 1;
      break;
    }
  }
  po::options_description desc("Usage: qicli [OPTIONS] SUBCMD [-h] [OPTIONS] [ARGS]");

  desc.add_options()
      ("help,h", "Print this help message and exit")
      ;

  po::positional_options_description positionalOptions;
  positionalOptions.add("", 1);
  po::variables_map vm;

  try {
    po::store(po::command_line_parser(argc, argv).options(desc).positional(positionalOptions).run(), vm);
    po::notify(vm);
  } catch (po::error &e) {
    printError(e.what());
    return false;
  }

  if (vm.count("help") || (subCmdArgc == 0 && argc > 1))
  {
    showHelp(desc);
    return 0;
  }

  if (subCmdArgc == 0)
  {
    subCmd = subCmdMap["info"];
  }

  int ret;
  try {
    ret = subCmd(subCmdArgc, subCmdArgv, app);
  } catch (const std::exception& e)
  {
    printError(e.what());
    return 1;
  } catch (...) {
    printError("unknown error catched");
    return 1;
  }
  return ret;
}
