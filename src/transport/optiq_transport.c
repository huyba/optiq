#include <stdlib.h>

#include <optiq_transport.h>

void optiq_transport_assign_service_level_to_message(struct optiq_message  message, int service_level)
{
    message.service_level = service_level;
}

void optiq_transport_add_message_to_hi_queue(struct optiq_message message, int weight)
{

}

void optiq_transport_add_message_to_low_queue(struct optiq_message message, int weight)
{

}
