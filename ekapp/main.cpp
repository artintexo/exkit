#include "example.h"

#include "base/context.h"
#include "base/datetime.h"

#include <cstdio>

int main(int, char **)
{
    Config config;
    config.main.name = "ekapp";
    config.main.mode = Mode::Backtest;
    config.main.startId = 3000000;
    config.simulator.histpath = "/home/user/expack/hist";
    config.simulator.start = Datetime(2019, 5, 3, 10, 0).timestamp;
    config.simulator.stop = Datetime(2019, 5, 3, 18, 50).timestamp;

    Example strategy;
    Context ctx(config, &strategy);
    ctx.Run();
    fprintf(stderr, "\n%s\n\n", GetString(ctx.GetStatistics()).c_str());

    return 0;
}
