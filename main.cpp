#include <iostream>
#include <fstream>
#include <sstream>
#include "nlohmann/json.hpp"
using json = nlohmann::json;

class Test {
private:
  std::string id; // can be a name as well
  std::string url;
  std::string endpoint;
  std::map<std::string, std::string> params;

public:
  Test(std::string n_id, std::string n_url, std::string n_endpoint, std::map<std::string, std::string> n_params) :
    id(n_id), url(n_url), endpoint(n_endpoint), params(n_params) {}

  void print() {
    std::cout << "test id: " << id << std::endl;
    std::cout << "url: " << url << std::endl;
    std::cout << "endpoint: " << endpoint << std::endl;
    if (!params.empty()) {
      std::cout << "params: " << std::endl;
      for (const auto &p : params) {
        std::cout << "\t" << p.first << ": " << p.second << std::endl;
      }
    }
  }
};

class Tests {
private:
  std::vector<Test> tests;

public:
  Tests(std::ifstream &f) {
    std::stringstream buffer;
    buffer << f.rdbuf();

    json j = json::parse(buffer);
    for (auto &t : j) {
      std::map<std::string, std::string> n_params;
      if (t["params"] != NULL) {
        for (auto &[k, v] : t["params"].items()) {
          n_params.insert(std::make_pair(k, v));
        }
      }
      Test n_test = Test(t["id"], t["url"], t["endpoint"], n_params);
      tests.push_back(n_test);
    }

    f.close();
  }

  void print_all() {
    for (auto &t : tests) {
      t.print();
      std::cout << std::endl;
    }
  }

  void run_all() {
  }
};

int main(int argc, char *argv[]) {
  // TODO: check input args
  if (argc != 2) {
    std::cout << "[ERROR]: pass in a file path" << std::endl;
    return 0;
  }
  
  std::ifstream f(argv[1]);
  if (!f.is_open()) {
    std::cout << "[ERROR]: unable to open file" << std::endl;
    return 0;
  }

  Tests custom_tests = Tests(f);
  custom_tests.print_all();

  return 0;
}
