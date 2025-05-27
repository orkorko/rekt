#include <stdio.h>

const char* help =
  "Usage: rekt [src] [--options]\n\n"
  "Options: \n"
  "  --output [path]  path for compiled binary\n"
  "  --help           display this information\n";

int main(int argc, char **argv)
{
  if (argc == 0) {
    printf("%s", help);
    return 0;
  }

  return 0;
}
