#include "error.h"
#include "system.h"

void node::error::internal::fatal()
{
	system::restartAfter1min();
}
