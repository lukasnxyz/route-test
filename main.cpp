#include <iostream>
#include <fstream>
#include <sstream>
#include <curl/curl.h>
#include "nlohmann/json.hpp"

using json = nlohmann::json;

#define CURL_FAIL "CURL_FAIL"
#define INDENT_LEN 2

/*
// TODO:
class Json {
};
*/

class Logger {
public:
  static void error(std::string msg) {
    std::cout << "\e[31m[ERROR]: " << msg << "\e[0m" << std::endl;
  }

  static void info(std::string msg) {
    std::cout << "\e[34m[INFO]: " << msg << "\e[0m" << std::endl;
  }

  static void success(std::string msg) {
    std::cout << "\e[32m[SUCCESS]: " << msg << "\e[0m" << std::endl;
  }
};

/*
// TODO: GET, POST methods with curl
class Fetch {
};
*/

class Test {
private:
  std::string id; 
  std::string url;
  std::string endpoint;
  std::map<std::string, std::string> params;
  //std::string method; // (GET, POST)
  json expected_response;

  std::string compose_request() {
    std::string r;
    r.append(url);
    r.append(endpoint);
    if (!params.empty()) {
      r.append("?");
      for (const auto &p : params) {
        r.append(p.first + "=" + p.second);
        if (/*p not at end*/true) {
          r.append("&");
        }
      }
    }

    return r;
  }

  static size_t WriteCallback(void *contents, const size_t size, const size_t nmemb, std::string *output) {
    const size_t total_size = size * nmemb;
    output->append((char*)contents, total_size);
    return total_size;
  }

  std::string r_curl(std::string in_url) {
    const char *r_in_url = in_url.c_str();
    CURL* curl;
    CURLcode res;
    std::string read_buffer;

    curl = curl_easy_init();
    if (curl) {
      curl_easy_setopt(curl, CURLOPT_URL, r_in_url);

      curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
      curl_easy_setopt(curl, CURLOPT_WRITEDATA, &read_buffer);

      res = curl_easy_perform(curl);

      if (res != CURLE_OK) {
        Logger::error("curl_easy_perform() failed");
      } else {
        return read_buffer; 
      }

      curl_easy_cleanup(curl);
    }

    return CURL_FAIL;
  }

public:
  Test(std::string n_id, std::string n_url, std::string n_endpoint, std::map<std::string, std::string> n_params, json n_expected_response) :
    id(n_id), url(n_url), endpoint(n_endpoint), params(n_params), expected_response(n_expected_response) {}

  std::string get_id() { return id; }
  std::string get_endpoint() { return endpoint; }
  std::map<std::string, std::string> get_params() { return params; }
  json get_expected_response() { return expected_response; }

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

  json run() {
    const std::string r = compose_request();
    const std::string res = r_curl(r);
    if (res == CURL_FAIL) {
      Logger::error("test '" + id + "', error curling: " + url);
      return NULL;
    } 
    const json j = json::parse(res);
    if (!expected_response.empty()) {
      if (!expected_response.dump(INDENT_LEN).compare(j.dump(INDENT_LEN))) {
        Logger::success("test '" + id + "' passed");
      } else {
        Logger::error("test '" + id + "' failed");
      }
    } else {
      Logger::info("test '" + id + "' ran successfully, but no expected response to compare to (print response)");
      std::cout << j.dump(INDENT_LEN) << std::endl;
    }
    return j;
  }
};

class Tests {
private:
  std::vector<Test> tests;
  std::map<std::string, json> responses;

public:
  Tests(std::ifstream &f) {
    std::stringstream buffer;
    buffer << f.rdbuf();

    json j = json::parse(buffer);
    for (auto &t : j) {
      std::map<std::string, std::string> n_params;
      if (t["parameters"] != NULL) {
        for (const auto &[k, v] : t["parameters"].items()) {
          n_params.insert(std::make_pair(k, v));
        }
      }
      const Test n_test = Test(t["id"], t["url"], t["endpoint"], n_params, t["expected_response"]);
      tests.push_back(n_test);
    }

    f.close();
  }

  std::vector<Test> get_tests() { return tests; }
  std::map<std::string, json> get_responses() { return responses; }

  void print_all() {
    for (auto &t : tests) {
      t.print();
      std::cout << std::endl;
    }
  }

  void run_all() {
    for (auto &t : tests) {
      json j = t.run();
      responses.insert(std::make_pair(t.get_id(), j));
    }
  }

  void print_responses() {
    Logger::info("responses:");
    for (const auto &[id, js] : responses) {
      std::cout << id << ": " << js.dump(INDENT_LEN) << std::endl;
    }
  }
};

int main(int argc, char *argv[]) {
  // TODO: add proper cli command parsing
  if (argc != 2) {
    Logger::error("you must pass in a json file!");
    return 0;
  }
  
  std::ifstream f(argv[1]);
  if (!f.is_open()) {
    Logger::error("unable to open file");
    f.close();
    return 0;
  }

  Tests api_tests = Tests(f);
  api_tests.run_all();

  return 0;
}
