#if defined(BUILD_SERVER)
#include <server.h>
#elif defined(BUILD_CLIENT)
#include <client.h>
#else
#endif

#include "../../../common/logs.h"
#include "../../../common/constants.h"
#include "../../../common/cpu_pin.h"

#if defined(BUILD_SERVER)
int start()
{
    CPUPIN::pin_to_cpu(1); // Pin server to CPU 1

    Server server(static_cast<uint16_t>(CONSTANTS::PORT));
    
    if(!server.init())
    {
        LOG_ERROR("Server initialization failed");
        return EXIT_FAILURE;
    }

    if(!server.start())
    {
        LOG_ERROR("Server start failed");
        return EXIT_FAILURE;
    }

    server.run(CONSTANTS::MAX_FDS , CONSTANTS::BUFFER_SIZE , CONSTANTS::MAX_EVENTS);

    return EXIT_SUCCESS;
}

#endif

#if defined(BUILD_CLIENT)

int start()
{
    CPUPIN::pin_to_cpu(2); // Pin client to CPU 2

    Client client(static_cast<uint16_t>(CONSTANTS::PORT));

    LOG_INFO("Starting client...");
    
    if(!client.init())
    {
        LOG_ERROR("Client initialization failed");
        return EXIT_FAILURE;
    }

    if(!client.start(CONSTANTS::ITERATIONS))
    {
        LOG_ERROR("Client start failed");
        return EXIT_FAILURE;
    }
    
    LOG_INFO("Client finished successfully");

    return EXIT_SUCCESS;
}
#endif
int main()
{
    return start();
}
