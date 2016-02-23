/*
** Author(s):
**  - Nicolas CORNU <ncornu@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/


#include <boost/program_options.hpp>
#include <boost/function.hpp>

namespace po = boost::program_options;

#include <qi/application.hpp>
#include <qi/atomic.hpp>
#include <qi/signal.hpp>
#include <qi/perf/dataperfsuite.hpp>

qi::Atomic<int> glob(0);

void foo()
{
  ++glob;
}

void fooInt(int a)
{
  ++glob;
}

void fooConstRStr(const std::string& a)
{
  ++glob;
}

void fooStr(std::string a)
{
  ++glob;
}

void fooSevenArgs(int a1, int a2, int a3, int a4, int a5, int a6)
{
  ++glob;
}

int main(int argc, char *argv[])
{
  qi::Application app(argc, argv);

  po::options_description desc(std::string("Usage:\n ")+argv[0]+"\n");
  desc.add_options()
    ("help,h", "Print this help.");

  desc.add(qi::detail::getPerfOptions());

  po::variables_map vm;
  po::store(po::command_line_parser(argc, argv)
            .options(desc).allow_unregistered().run(), vm);
  po::notify(vm);

  if (vm.count("help")) {
    std::cout << desc << std::endl;
    return EXIT_SUCCESS;
  }

  qi::DataPerfSuite out("qimessaging", "perf_functions", qi::DataPerfSuite::OutputData_Period, vm["output"].as<std::string>());

  qi::DataPerf dp;

  // Test signals without any arguments
  dp.start("Signal_void", 100000000);
  for (unsigned int i = 0; i < 100000000; ++i) {
    foo();
  }
  dp.stop();
  out << dp;

  // Test signals with an int
  dp.start("Signal_int", 100000000);
  for (unsigned int i = 0; i < 100000000; ++i) {
    fooInt(1);
  }
  dp.stop();
  out << dp;

  // Test signals with a string of 32768 bytes
  std::string s;
  for (unsigned int i = 0; i < 65535; ++i)
    s += "a";
  dp.start("Signal_Big_String", 100000000);
  for (unsigned int i = 0; i < 100000000; ++i) {
    fooConstRStr(s);
  }
  dp.stop();
  out << dp;

  // Test signal with 10 args
  dp.start("Signal_7_int", 100000000);
  for (unsigned int i = 0; i < 100000000; ++i) {
    fooSevenArgs(1, 1, 1, 1, 1, 1);
  }
  dp.stop();
  out << dp;

  return EXIT_SUCCESS;
}
