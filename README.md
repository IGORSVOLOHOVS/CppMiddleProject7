# cpp-middle-project-sprint-7 <!-- omit in toc -->

- [Getting Started](#getting-started)
- [Building the Project and Running Tests](#building-the-project-and-running-tests)
  - [Commands to Build the Project](#commands-to-build-the-project)
  - [Commands to Run the Application](#commands-to-run-the-application)
  - [Command to Run Tests](#command-to-run-tests)

Repository template for the practical task of the 7th sprint of "Middle C++ Developer".

## Getting Started

1. Click the green `Use this template` button, then `Create a new repository`.
2. Name your repository.
3. Clone the created repository using the command `git clone your-repository-name`.
4. Create a new branch using the command `git switch -c development`.
5. Open the project in `Visual Studio Code`.
6. Press `F1` and open the project in a dev container using the command `Dev Containers: Reopen in Container`.

## Building the Project and Running Tests

This repository uses several tools:

- **cmake** — a build system generator for C and C++. It allows you to create projects that can be compiled on various platforms and with different compilers. More about cmake:
  - https://dzen.ru/a/ZzZGUm-4o0u-IQlb
  - https://neerc.ifmo.ru/wiki/index.php?title=CMake_Tutorial
  - https://cmake.org/cmake/help/book/mastering-cmake/cmake/Help/guide/tutorial/index.html

- **VS Code Dev Docker container** - a Docker container that contains a fully configured environment for the task. More about this functionality:
  - https://habr.com/ru/articles/822707/ - "Almost everything you wanted to know about Docker"
  - https://code.visualstudio.com/docs/devcontainers/containers - official VS Code documentation
  - https://www.youtube.com/watch?v=p9L7YFqHGk4 - "Docker container for VS Code"
  - https://www.youtube.com/watch?v=pg19Z8LL06w&t=174s&pp=ygUPRG9ja2VyY29udGFpbmVy - "Docker in 1 hour"

### Commands to Build the Project

- Create a `build` folder
- Navigate into it: `cd build`
- Run `cmake ..`
- Run `make`

### Commands to Run the Application

```bash
cd build

./AsyncHttpProxy 5555 &

python3 -c 'print("HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: 4096\r\n\r\n" + "A"*4096, end="")' | nc -l 127.0.0.1 -p 8000 &

wget -e use_proxy=yes -e http_proxy=127.0.0.1:5555 127.0.0.1:8000
```

### Command to Run Tests

```bash
cd build
./AsyncHttpProxy_tests
```
