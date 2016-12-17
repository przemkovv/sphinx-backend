

#include "backend_app.h"

int main(int argc, char *argv[])
{
  Sphinx::Backend::BackendApp app{"Sphinx::Backend", {argv + 1, argv + argc}};
  
  return app.run();
}
