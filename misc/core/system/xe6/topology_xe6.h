#ifndef OPTIQ_TOPOLOGY_XE6_H
#define OPTIQ_TOPOLOGY_XE6_H

//#ifndef __CRAYXE
//#define __CRAYXE
//#endif

#ifdef __CRAYXE
#include <pmi.h>
#include <rca_lib.h>
#endif

#include "../topology_interface.h"

extern struct topology_interface topology_xe6;

#endif
