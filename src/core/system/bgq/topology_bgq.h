#ifndef TOPOLOGY_BGQ_H
#define TOPOLOGY_BGQ_H

#ifdef __bgq__
#include <spi/include/kernel/location.h>
#include <spi/include/kernel/process.h>
#include <firmware/include/personality.h>
#endif

#include "../topology_interface.h"

extern struct topology_interface topology_bgq;

#endif
