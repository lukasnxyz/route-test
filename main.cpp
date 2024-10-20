#include <iostream>
#include <fstream>
#include <sstream>
#include <curl/curl.h>
#include "nlohmann/json.hpp"
using json = nlohmann::json;

/* TODO:
 * - add cli param parsing
*/

class Logger {
private:
public:
};

class Test {
private:
  std::string id; // can be a name as well
  std::string url;
  std::string endpoint;
  std::map<std::string, std::string> params;

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
        // TODO: change error logging
        std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
      } else {
        return read_buffer; 
      }

      curl_easy_cleanup(curl);
    }

    return NULL;
  }

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

  void run(Logger logger) {
    std::string r = compose_request();
    std::cout << "testing: " << r << std::endl;
    std::string res = r_curl(r);
    std::cout << res << std::endl;
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
    Logger logger = Logger();
    for (auto &t : tests) {
      t.run(logger);
    }
  }
};

int main(int argc, char *argv[]) {
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
  custom_tests.run_all();

  return 0;
}
