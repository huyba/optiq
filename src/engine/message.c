#include <vector>
#include <stdlib.h>

#include "message.h"

struct optiq_message* get_send_message(vector<struct optiq_message *> *messages)
{
    struct optiq_message *message = NULL;

    if ((*messages).size() > 0) {
        message = (*messages).back();
        (*messages).pop_back();
    } else {
        message = (struct optiq_message *)malloc(sizeof(struct optiq_message));
    }

    return message;
}
