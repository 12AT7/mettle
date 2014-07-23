#include <mettle.hpp>
#include <mettle/file_runner.hpp>
using namespace mettle;

struct recording_logger : log::test_logger {
  void start_run() {
    called = "start_run";
  }
  void end_run() {
    called = "end_run";
  }

  void start_suite(const std::vector<std::string> &actual_suites) {
    called = "start_suite";
    suites = actual_suites;
  }
  void end_suite(const std::vector<std::string> &actual_suites) {
    called = "end_suite";
    suites = actual_suites;
  }

  void start_test(const log::test_name &actual_test) {
    called = "start_test";
    test = actual_test;
  }
  void passed_test(const log::test_name &actual_test,
                   log::test_output &actual_output) {
    called = "passed_test";
    test = actual_test;
    stdout = actual_output.stdout.str();
    stderr = actual_output.stderr.str();
  }
  void failed_test(const log::test_name &actual_test,
                   const std::string &actual_message,
                   log::test_output &actual_output) {
    called = "failed_test";
    test = actual_test;
    message = actual_message;
    stdout = actual_output.stdout.str();
    stderr = actual_output.stderr.str();
  }
  void skipped_test(const log::test_name &actual_test) {
    called = "skipped_test";
    test = actual_test;
  }

  std::string called;
  std::vector<std::string> suites;
  log::test_name test;
  std::string message, stdout, stderr;
};

struct fixture {
  fixture() : child(stream) {}

  std::stringstream stream;
  recording_logger parent;
  log::child child;
};

suite<fixture> test_child("test child logger", [](auto &_) {

  _.test("start_run()", [](fixture &f) {
    f.child.start_run();
    detail::pipe_to_logger(f.parent, f.stream);

    // Shouldn't be called, since we ignore start_run and end_run.
    expect(f.parent.called, equal_to(""));
  });

  _.test("end_run()", [](fixture &f) {
    f.child.end_run();
    detail::pipe_to_logger(f.parent, f.stream);

    // Shouldn't be called, since we ignore start_run and end_run.
    expect(f.parent.called, equal_to(""));
  });

  _.test("start_suite()", [](fixture &f) {
    std::vector<std::string> suites = {"suite", "subsuite"};
    f.child.start_suite(suites);
    detail::pipe_to_logger(f.parent, f.stream);

    expect(f.parent.called, equal_to("start_suite"));
    expect(f.parent.suites, equal_to(suites));
  });

  _.test("end_suite()", [](fixture &f) {
    std::vector<std::string> suites = {"suite", "subsuite"};
    f.child.end_suite(suites);
    detail::pipe_to_logger(f.parent, f.stream);

    expect(f.parent.called, equal_to("end_suite"));
    expect(f.parent.suites, equal_to(suites));
  });

  _.test("start_test()", [](fixture &f) {
    log::test_name test = {{"suite", "subsuite"}, "test", 1};
    f.child.start_test(test);
    detail::pipe_to_logger(f.parent, f.stream);

    expect(f.parent.called, equal_to("start_test"));
    expect(f.parent.test, equal_to(test));
  });

  _.test("passed_test()", [](fixture &f) {
    log::test_name test = {{"suite", "subsuite"}, "test", 1};
    log::test_output output;
    output.stdout << "stdout";
    output.stderr << "stderr";

    f.child.passed_test(test, output);
    detail::pipe_to_logger(f.parent, f.stream);

    expect(f.parent.called, equal_to("passed_test"));
    expect(f.parent.test, equal_to(test));
    expect(f.parent.stdout, equal_to( output.stdout.str() ));
    expect(f.parent.stderr, equal_to( output.stderr.str() ));
  });

  _.test("failed_test()", [](fixture &f) {
    log::test_name test = {{"suite", "subsuite"}, "test", 1};
    std::string message = "failure";
    log::test_output output;
    output.stdout << "stdout";
    output.stderr << "stderr";

    f.child.failed_test(test, message, output);
    detail::pipe_to_logger(f.parent, f.stream);

    expect(f.parent.called, equal_to("failed_test"));
    expect(f.parent.test, equal_to(test));
    expect(f.parent.message, equal_to(message));
    expect(f.parent.stdout, equal_to( output.stdout.str() ));
    expect(f.parent.stderr, equal_to( output.stderr.str() ));
  });

  _.test("skipped_test()", [](fixture &f) {
    log::test_name test = {{"suite", "subsuite"}, "test", 1};
    f.child.skipped_test(test);
    detail::pipe_to_logger(f.parent, f.stream);

    expect(f.parent.called, equal_to("skipped_test"));
    expect(f.parent.test, equal_to(test));
  });

});
