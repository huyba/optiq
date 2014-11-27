#include "util.h"
#include "flow.h"

using namespace std;

int get_next_dest_from_flow(const struct optiq_flow *flow, int current_ep)
{
    for (int i = 0; i < (*flow).arcs.size(); i++) {
        if ((*flow).arcs[i].ep1 == current_ep) {
            return (*flow).arcs[i].ep2;
        }
    }
    return -1;
}
