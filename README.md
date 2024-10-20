### Route Tester
Warning: this is still in progress. Writing this simple cli route tester for a uni class where I have to build a complex web 
app and api beacuse I don't want to use bloatware like Insomnia.

#### Quick-start
```bash
$ g++ -Wall -Wextra -o main main.cpp -lcurl
$ ./main <your_test.json>
```

#### Credits
This project uses the MIT license and uses `json.hpp` from `https://github.com/nlohmann/json`
to parse json files. Maybe I'll rewrite a basic version of it for myself, but I'm using now
just to get something working.
