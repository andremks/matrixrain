#include <algorithm>
#include <chrono>
#include <csignal>
#include <cstring>
#include <iostream>
#include <random>
#include <string>
#include <sys/ioctl.h>
#include <termios.h>
#include <thread>
#include <unistd.h>
#include <vector>

namespace {
volatile std::sig_atomic_t resized = 0;
void on_resize(int) { resized = 1; }

struct Size {
  int w = 80, h = 24;
};
Size terminal_size() {
  winsize ws{};
  if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0 && ws.ws_col && ws.ws_row)
    return {static_cast<int>(ws.ws_col), static_cast<int>(ws.ws_row)};
  return {};
}

class RawMode {
  termios old{};
  bool enabled = false;

public:
  RawMode() {
    if (tcgetattr(STDIN_FILENO, &old) != 0)
      return;
    termios raw = old;
    raw.c_lflag &= static_cast<unsigned>(~(ICANON | ECHO | ISIG));
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 0;
    enabled = tcsetattr(STDIN_FILENO, TCSANOW, &raw) == 0;
    if (enabled)
      std::cout << "\033[?1049h\033[2J\033[H\033[?25l" << std::flush;
  }
  ~RawMode() {
    if (enabled) {
      std::cout << "\033[0m\033[?25h\033[2J\033[H\033[?1049l" << std::flush;
      tcsetattr(STDIN_FILENO, TCSANOW, &old);
    }
  }
  bool ok() const { return enabled; }
};

struct Rain {
  double head = -1;
  double speed = .5;
  int length = 8, delay = 0;
  bool active = false;
};

char random_char(std::mt19937 &gen) {
  static constexpr char set[] =
      "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
  return set[std::uniform_int_distribution<int>(0, sizeof(set) - 2)(gen)];
}

void reset_rain(Rain &r, int height, std::mt19937 &gen, bool first) {
  r.speed = std::uniform_real_distribution<double>(.25, .95)(gen);
  r.length =
      std::uniform_int_distribution<int>(5, std::max(6, height / 2))(gen);
  r.delay =
      std::uniform_int_distribution<int>(first ? 0 : 5, first ? 40 : 50)(gen);
  r.head = -std::uniform_int_distribution<int>(0, height)(gen);
  r.active = false;
}

void open_terminal(const char *exe) {
  const char *names[] = {"gnome-terminal", "mate-terminal", "xfce4-terminal",
                         "konsole"};
  for (const char *name : names) {
    if (!std::strcmp(name, "konsole"))
      execlp(name, name, "--maximize", "-e", exe, "--rain", nullptr);
    else
      execlp(name, name, "--maximize", "--", exe, "--rain", nullptr);
  }
  std::cerr << "Nenhum emulador de terminal compatível foi encontrado.\n";
  _exit(127);
}

int run_matrix() {
  RawMode terminal;
  if (!terminal.ok())
    return 1;
  std::signal(SIGWINCH, on_resize);

  std::mt19937 gen(std::random_device{}());
  Size s = terminal_size();
  std::vector<Rain> rain(s.w);
  std::vector<std::string> chars(s.w);

  auto initialize = [&] {
    s = terminal_size();
    rain.resize(s.w);
    chars.resize(s.w);
    for (int x = 0; x < s.w; ++x) {
      reset_rain(rain[x], s.h, gen, true);
      chars[x].resize(std::max(s.h, rain[x].length + 2));
      for (char &c : chars[x])
        c = random_char(gen);
    }
    std::cout << "\033[2J\033[H";
  };
  initialize();

  bool running = true;
  auto next_frame = std::chrono::steady_clock::now();
  constexpr auto frame_time = std::chrono::microseconds(76667); // 60 FPS

  while (running) {
    char key;
    if (read(STDIN_FILENO, &key, 1) == 1 &&
        (key == 27 || key == 'q' || key == 'Q'))
      running = false;
    if (!running)
      break;
    if (resized) {
      resized = 0;
      initialize();
    }

    std::string frame;
    frame.reserve(static_cast<size_t>(s.w) * s.h * 3);
    frame += "\033[H";
    for (int y = 0; y < s.h; ++y) {
      for (int x = 0; x < s.w; ++x) {
        const Rain &r = rain[x];
        const int distance = static_cast<int>(r.head) - y;
        if (r.active && distance >= 0 && distance < r.length) {
          int color = distance == 0 ? 231 : std::max(22, 118 - distance * 7);
          frame += "\033[38;5;" + std::to_string(color) + "m";
          frame += chars[x][(y + static_cast<int>(r.head)) % chars[x].size()];
        } else
          frame += ' ';
      }
      if (y + 1 < s.h)
        frame += '\n';
    }
    frame += "\033[0m";
    std::cout << frame << std::flush;

    for (int x = 0; x < s.w; ++x) {
      Rain &r = rain[x];
      if (r.delay > 0) {
        --r.delay;
        continue;
      }
      r.active = true;
      r.head += r.speed;
      if (r.head - r.length > s.h)
        reset_rain(r, s.h, gen, false);
      if ((gen() & 7u) == 0)
        chars[x][gen() % chars[x].size()] = random_char(gen);
    }

    next_frame += frame_time;
    std::this_thread::sleep_until(next_frame);
    if (std::chrono::steady_clock::now() >
        next_frame + std::chrono::milliseconds(200))
      next_frame = std::chrono::steady_clock::now();
  }
  return 0;
}
} // namespace

int main(int argc, char **argv) {
  if (argc > 1 && !std::strcmp(argv[1], "--rain"))
    return run_matrix();
  pid_t child = fork();
  if (child == 0)
    open_terminal(argv[0]);
  if (child < 0) {
    std::perror("fork");
    return 1;
  }
  return 0;
}