#include "handler.h"

HANDLER(state_reset, "") {
  editor->reset();
  return false;
}
