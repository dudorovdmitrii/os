#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
using namespace std;
#define RETRIES 1
#define SLEEP_SECONDS 1

int PID = 0;
struct sigaction act;

void quitHandler(int signal)
{
  execl("/bin/kill", "kill", PID, 0);
  cout << "\n";
}

int startTerminalProcess()
{
  act.sa_handler = SIG_DFL;
  act.sa_flags = SA_RESETHAND;
  sigaction(SIGINT, &act, 0);

  string command;
  getline(cin, command);

  vector<const char *> v;
  int i = 0;
  string current = "";

  for (int i = 0; i < command.size(); i++)
  {
    if (command[i] != ' ')
    {
      current.push_back(command[i]);
    }
    else
    {
      char *test = (char *)malloc(sizeof(char) * current.size());
      strcpy(test, current.data());
      v.push_back(test);
      current = "";
    }
  }

  v.push_back(current.data());
  string first = v[0];

  pid_t child_pid;
  child_pid = fork();

  switch (child_pid)
  {
  case -1:
    printf("Не удалось создать дочерний процесс!");
    break;
  case 0:
  {
    /* Мы - дочерний процесс */

    int code = execvp(first.data(), const_cast<char *const *>(v.data()));
    int retries = RETRIES;
    while (code != 0 && retries > 0)
    {
      retries--;
      cout << "Процесс завершился аварийно c кодом " << code << ". Перезапуск через " << SLEEP_SECONDS << " секунд.\n";
      sleep(SLEEP_SECONDS);
      code = execvp(first.data(), const_cast<char *const *>(v.data()));
    }
    cout << "Процесс завершился аварийно c кодом " << code << " после " << RETRIES + 1 << " попыток.\n";
    break;
  }

  default:
    /* Мы - родительский процесс */

    PID = child_pid;
    act.sa_handler = quitHandler;
    act.sa_flags = 0;
    sigaction(SIGINT, &act, 0);
    break;
  }

  if (child_pid != 0)
  {
    waitpid(child_pid, NULL, 0);
  }

  cout << "\n";
  return 0;
}

int main()
{

  int code = 0;
  while (code == 0)
  {
    cout << ">> ";
    code = startTerminalProcess();
  }

  return 0;
}
