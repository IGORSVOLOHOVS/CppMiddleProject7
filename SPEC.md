# Спецификация требований: AsyncHttpProxy

Восстановлена по коду коммита `40d9bae` (2025-09-02). Описывает реализованное
поведение, а не замысел задания курса.

<!-- jira-sync:start -->
Jira: KAN-140

## Tasks

This list is one ticket in Jira project KAN, [KAN-140](https://igorsvolohovs.atlassian.net/browse/KAN-140). Each line is a checkbox there: tick it in Jira and the sync ticks it here; add a checkbox there and it is appended here. Done items are the features described in section 3; open items are what the project declares but the code does not do yet.

- [x] Запуск и приём соединений (section 3.1) <!-- spec-12d9152358 -->
- [x] Разбор заголовков (section 3.2) <!-- spec-23ced5df9b -->
- [x] Сессия проксирования (section 3.3) <!-- spec-0f5e84f0bc -->
- [ ] Учёт байтов тела, прочитанных вместе с заголовками ответа (declared, not implemented) <!-- spec-c7106864c2 -->

<!-- jira-sync:end -->

## 1. Назначение и границы

Асинхронный однопоточный HTTP-прокси на Boost.Asio с корутинами (`src/main.cpp`), учебное
задание 7-го спринта «Мидл разработчик С++» (`README.md:9`).

**Входит:** приём TCP-соединений, разбор заголовков `Host` и `Content-Length`, пересылка
одного запроса и одного ответа на соединение.

**Не входит (в коде отсутствует):** HTTPS/`CONNECT`, keep-alive, chunked-кодирование,
кеширование, аутентификация, журналирование в файл, конфигурационные файлы.

## 2. Контекст

| Участник | Роль |
|---|---|
| HTTP-клиент | Подключается к прокси, отправляет запрос с заголовком `Host` (пример — `wget`, `README.md:51`) |
| Целевой HTTP-сервер | Адрес берётся из `Host`; подключение по TCP (`src/main.cpp:51-53`) |
| DNS-резолвер ОС | `tcp::resolver::async_resolve` (`src/main.cpp:52`) |

Среда выполнения: Linux-контейнер из `.devcontainer/Dockerfile` (образ Ubuntu по хешу,
`devcontainer.json:2` называет его «Ubuntu25.04 with GCC15»), один поток `io_service`
(`src/main.cpp:134`).

## 3. Функциональные требования

### 3.1 Запуск и приём соединений

- **FR-1.** Система должна при числе аргументов, отличном от одного, выводить в stderr
  `Usage: proxy_server <listen_port>` и завершаться с кодом 1.
  Source: `src/main.cpp:129-133`. Verification: no test.
- **FR-2.** Система должна слушать TCP-порт из `argv[1]` на всех IPv4-адресах.
  Source: `src/main.cpp:97-99,135`. Verification: no test.
- **FR-3.** Система должна обрабатывать каждое принятое соединение отдельной корутиной и
  продолжать приём после каждого `async_accept`, включая неуспешный.
  Source: `src/main.cpp:106-119`. Verification: no test.

### 3.2 Разбор заголовков

- **FR-4.** Система должна перебирать строки заголовков после стартовой строки до первой
  пустой строки, разделяя имя и значение по первому `": "` и пропуская строки без него.
  Source: `src/headers.cpp:11-33`. Verification: косвенно — тесты `findHostPort.*`, `findContentLength.*`.
- **FR-5.** Система должна из заголовка `Host` вида `host:port` выделять хост и порт по
  первому `:`. Source: `src/headers.cpp:38-43`. Verification: `findHostPort.Simple`.
- **FR-6.** Система должна при `Host` без `:` использовать порт `"80"`.
  Source: `src/headers.cpp:44-45`. Verification: `findHostPort.NoPort`.
- **FR-7.** Система должна возвращать пустые хост и порт при отсутствии `Host`.
  Source: `src/headers.cpp:35-50`. Verification: `findHostPort.NoHost`.
- **FR-8.** Система должна возвращать значение `Content-Length` как `size_t`, если оно
  разбирается `std::from_chars` без ошибки, и «нет значения» при отсутствии заголовка,
  нечисловом значении или переполнении.
  Source: `src/headers.cpp:52-64`. Verification: `findContentLength.Simple`,
  `findContentLength.NoContentLength`, `findContentLength.InvalidValue`,
  `findContentLength.TooLargeValue`.

### 3.3 Сессия проксирования

- **FR-9.** Система должна читать данные клиента до последовательности `\r\n\r\n`.
  Source: `src/main.cpp:30,38`. Verification: no test.
- **FR-10.** Система должна при пустом хосте печатать в stdout `Could not find host in request`
  и закрывать соединение. Source: `src/main.cpp:46-48,87-89`. Verification: no test.
- **FR-11.** Система должна разрешать хост и порт и подключаться к первому полученному адресу.
  Source: `src/main.cpp:51-53`. Verification: no test.
- **FR-12.** Система должна отправлять серверу всё содержимое буфера, прочитанного от клиента.
  Source: `src/main.cpp:55`. Verification: no test.
- **FR-13.** Система должна читать ответ сервера до `\r\n\r\n` и отправлять клиенту всё
  содержимое буфера ответа. Source: `src/main.cpp:57-65`. Verification: no test.
- **FR-14.** Система должна при наличии `Content-Length` в ответе далее пересылать данные
  сервера клиенту блоками до 8192 байт, пока счётчик `Content-Length` не исчерпан.
  Source: `src/main.cpp:67-78`. Verification: no test.
- **FR-15.** Система должна при исключении в сессии печатать `Session error: <текст>` в
  stderr и stdout. Source: `src/main.cpp:81-85`. Verification: no test.
- **FR-16.** Система должна по окончании сессии выполнять `shutdown` и `close` клиентского и
  серверного сокетов, игнорируя ошибки. Source: `src/main.cpp:87-91`. Verification: no test.

### 3.4 Заявлено, но не реализовано

| Пункт | Источник | Состояние |
|---|---|---|
| Учёт байтов тела, прочитанных вместе с заголовками ответа | `src/main.cpp:65-76` | Не реализовано: счётчик начинается с полного `Content-Length` (см. §9) |

Явных TODO или обещаний README без кода нет.

## 4. Интерфейсы

**CLI**

| Аргумент | Тип | Обязателен | Источник |
|---|---|---|---|
| `<listen_port>` | целое, `std::atoi` → `short` | да | `src/main.cpp:97,129-135` |

**Сеть**

| Сторона | Протокол | Детали | Источник |
|---|---|---|---|
| Клиент → прокси | TCP, HTTP/1.x в открытом виде | IPv4, порт из `argv[1]` | `src/main.cpp:99` |
| Прокси → сервер | TCP | хост/порт из `Host`, по умолчанию 80 | `src/main.cpp:45-53` |

**API библиотеки** (`include/headers.h`)

| Объявление | Назначение |
|---|---|
| `struct HostPort { std::string host, port; }` | Результат разбора `Host` |
| `void iterHeaders(std::string_view, Callback&&)` | Вызов `callback(name, value)` для каждого заголовка |
| `HostPort findHostPort(std::string_view)` | FR-5…FR-7 |
| `std::optional<size_t> findContentLength(std::string_view)` | FR-8 |

**Вывод:** stdout — `Could not find host in request`, `Session error: ...`; stderr — usage,
`Session error: ...`, `Exception: ...` (`src/main.cpp:47,83-84,130-131,139`).

## 5. Данные

Постоянного хранения нет. В памяти: `boost::asio::streambuf` для заголовков запроса и
ответа, `std::array<char, 8192>` для тела (`src/main.cpp:37,57,70`). Заголовки
обрабатываются как `std::string_view` без копирования (`src/headers.cpp:19-22`).

## 6. Нефункциональные требования

- **NFR-1.** Стандарт C++26 обязателен, расширения компилятора выключены. Source: `CMakeLists.txt:4-6`.
- **NFR-2.** Для GCC/Clang предупреждения являются ошибками (`-Wall -Werror -Wfatal-errors`).
  Source: `CMakeLists.txt:9-12`.
- **NFR-3.** Обработка ведётся в одном потоке `io_service` с подсказкой параллелизма 1.
  Source: `src/main.cpp:134-136`.
- **NFR-4.** Форматирование по `.clang-format` (LLVM, ширина 120, отступ 4). Source: `.clang-format`.

## 7. Ограничения и зависимости

| Зависимость | Версия | Источник |
|---|---|---|
| CMake | ≥ 3.31.6 | `CMakeLists.txt:1` |
| Компилятор | g++-15 в контейнере; профиль Conan: gcc 15, `gnu26`, `libstdc++11` | `.devcontainer/Dockerfile:5,10`, `.devcontainer/default` |
| Boost (Asio, Beast) | не закреплена; `libboost-all-dev` | `CMakeLists.txt:20`, `.devcontainer/Dockerfile:6` |
| GoogleTest | не закреплена; `libgtest-dev` | `CMakeLists.txt:21`, `.devcontainer/Dockerfile:6` |

Lock-файлов нет; `conanfile` отсутствует, профиль `.devcontainer/default` ни на что не ссылается.

## 8. Приёмка

1. Собрать: `mkdir build && cd build && cmake .. && make` (`README.md:37-40`).
2. Запустить `./AsyncHttpProxy_tests` (`README.md:57-60`) — 7 тестов должны пройти (FR-4…FR-8).
3. Выполнить сценарий из `README.md:44-52`: `wget` через прокси на порт 5555 должен получить
   ответ с телом 4096 байт (FR-9…FR-14). Из-за дефекта, описанного в §9, этот шаг может
   зависнуть или завершиться `Session error`, если тело придёт вместе с заголовками.
4. Запуск без аргументов — сообщение usage и код 1 (FR-1).

Ни один из шагов при подготовке документа не выполнялся.

## 9. Открытые вопросы

- Должен ли прокси поддерживать keep-alive, тело запроса клиента, chunked-ответы, `CONNECT`?
  Текст задания спринта в репозитории отсутствует.
- Ожидаемое поведение, когда часть тела ответа прочитана вместе с заголовками: по коду
  (`src/main.cpp:65-76`) эти байты не вычитаются из `Content-Length`, что может приводить к
  ожиданию лишних байт или к переполнению `bytes_to_transfer`. Ошибка это или допущение
  задания — не установлено, запуском не проверено.
- Требования к производительности и числу одновременных соединений в файлах не указаны.
- Зачем в репозитории профиль Conan (`.devcontainer/default`) без `conanfile`.
- Почему `src/main.cpp` входит и в библиотеку, и в исполняемый файл (`CMakeLists.txt:26-29,39-41`).
- Версия Boost: используется `io_service` (`src/main.cpp:18`); по сведениям автора документа
  (не проверено), это устаревший псевдоним `io_context`, удалённый в новых версиях Boost.
