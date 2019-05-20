## Overview
Simple framework for trading futures via Plaza-2
* __ekapp__ - example strategy
* __eksaver__ - save market data for backtest


## Realtime config:
```cpp
Config config;
config.main.name = "ekapp";
config.main.mode = Mode::Realtime;
config.main.startId = 1000000;
config.plaza.type = "p2lrpcq";
config.plaza.host = "127.0.0.1";
config.plaza.port = "4001";
config.plaza.subs = "mq,replclient";
config.plaza.ini = "/home/user/expack/ini/ekapp.ini";
config.plaza.key = "11111111";
config.plaza.broker = "FZ00";
config.plaza.client = "J00";
```

## Backtest config:
```cpp
Config config;
config.main.name = "ekapp";
config.main.mode = Mode::Backtest;
config.main.startId = 1000000;
config.simulator.histpath = "/home/user/expack/hist";
config.simulator.start = Datetime(2019, 5, 3, 10, 0).timestamp;
config.simulator.stop = Datetime(2019, 5, 3, 18, 50).timestamp;
```
