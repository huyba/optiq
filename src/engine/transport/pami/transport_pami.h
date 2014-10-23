#ifndef OPTIQ_TRANSPORT_PAMI_H
#define OPTIQ_TRANSPORT_PAMI_H

#ifdef __bgq__
#include <spi/include/kernel/location.h>
#include <spi/include/kernel/process.h>
#include <firmware/include/personality.h>
#endif

#include "../transport_interface.h"

extern struct transport_interface transport_pami;

#endif
