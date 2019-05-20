#pragma once

#include <string>
#include <vector>

//
// forward declarations
class Context;
class Holder;
class Plaza;
class Simulator;
class IStrategy;
class IDataGate;
class IExecGate;

class LogManager;
class TimeManager;
class EventManager;
class OrderManager;
class PositionManager;
class StatManager;
class SessionManager;
class SecurityManager;

class DataWriter;
class DataReader;

class Plaza;
class DataPlaza;
class ExecPlaza;

class Simulator;
class DataSim;
class ExecSim;

class Datetime;
class Order;

struct ExecReport;
struct ExecTrade;
struct UserDeal;

struct Obook;
struct Trade;
struct Session;
struct Security;
struct Heartbeat;

//
// enums
enum class LogLevel {
    Debug,
    Info,
    Warn,
    Error
};

enum class Mode {
    Backtest,
    Paper,
    Realtime
};

enum class DataType {
    None,
    Obook,
    Trade,
    Session,
    Security,
};

enum class EventType {
    None,
    NewExecReport,
    NewExecTrade,
    NewObook,
    NewTrade,
    NewSession,
    NewSecurity,
    NewHeartbeat,
    NewEndHistFile,
    NewEndHistData,
};

enum class ExecTif {
    Day = 1,
    IOC = 2,
    FOK = 3
};

enum class ExecDir {
    Long = 1,
    Short = 2,
    Both = 3
};

enum class ExecStatus {
    AcceptReject,
    CancelReject,
    Created,
    Accepted,
    Filled,
    Cancelled,
    Rejected
};

//
// config
struct MainConfig {
    std::string name = "default";
    Mode mode = Mode::Backtest;
    int startId = 1000000;
};

struct SimulatorConfig {
    std::string histpath = "./hist";
    long start = 0;
    long stop = 0;
    bool fillOnObook = true;
    bool fillOnTrade = false;
};

struct PlazaConfig {
    std::string type = "p2lrpcq";
    std::string host = "127.0.0.1";
    std::string port = "4001";
    std::string subs = "mq,replclient";
    std::string ini = "./default.ini";
    std::string key = "11111111";
    std::string broker = "Your broker code";
    std::string client = "Your client code";
    int tradeMsgMax = 30;
    int nontradeMsgMax = 1000;
};

struct Config {
    MainConfig main;
    PlazaConfig plaza;
    SimulatorConfig simulator;
};

//
// strategy
class IStrategy {
public:
    virtual ~IStrategy() {}
    virtual void OnStart() = 0;
    virtual void OnStop() = 0;
    virtual void OnOrder(Order *) = 0;
    virtual void OnObook(Obook &) = 0;
    virtual void OnTrade(Trade &) = 0;
    virtual void OnSession(Session &) = 0;
    virtual void OnSecurity(Security &) = 0;
    virtual void OnReminder(long) = 0;
};

//
// gates
class IGate {
public:
    virtual ~IGate() {}
    virtual bool IsOpened() = 0;
    virtual bool IsClosed() = 0;
    virtual void Open() = 0;
    virtual void Close() = 0;
};

class IDataGate : public IGate {
public:
    virtual ~IDataGate() {}
    virtual void Subscribe(const Security &) = 0;
    virtual void Unsubscribe(const Security &) = 0;
};

class IExecGate : public IGate {
public:
    virtual ~IExecGate() {}
    virtual void Send(Order *) = 0;
    virtual void Cancel(Order *) = 0;
    virtual void CancelAll(const Security &, ExecDir) = 0;
};

// exec data
struct ExecReport {
    long timestamp;
    int clientId;
    long orderId;
    ExecStatus status;
};

struct ExecTrade {
    long timestamp;
    int clientId;
    long orderId;
    double price;
    int qty;
};

struct UserTrade {
    long timestamp;
    int secId;
    ExecDir dir;
    double price;
    int qty;
};

struct UserDealItem {
    long timestamp;
    double price;
};

struct UserDeal {
    int secId;
    ExecDir dir;
    int qty;
    UserDealItem open;
    UserDealItem close;
};

struct Statistics {
    double grossAll;
    double grossWin;
    double grossLoss;
    double profitFactor;
    int numAll;
    int numWin;
    int numLoss;
    double winRate;
    double avgProfit;
    double avgTime;
};

//
// market data
constexpr int EK_OBOOK_DEPTH = 5;

struct ObookItem {
    ObookItem() {}
    ObookItem(double price, int qty) : price(price), qty(qty) {}
    double price;
    int qty;
    bool operator==(const ObookItem &o) { return price == o.price && qty == o.qty; }
    bool operator!=(const ObookItem &o) { return price != o.price || qty != o.qty; }
    bool operator<(const ObookItem &o) { return price < o.price; }
    bool operator>(const ObookItem &o) { return price > o.price; }
};

struct Obook {
    long timestamp;
    int secId;
    std::vector<ObookItem> bids;
    std::vector<ObookItem> asks;
};

struct Trade {
    long timestamp;
    long moment;
    int secId;
    double price;
    int qty;
};

struct SessionItem {
    long begin;
    long end;
    bool operator==(const SessionItem &o) { return begin == o.begin && end == o.end; }
    bool operator!=(const SessionItem &o) { return begin != o.begin || end != o.end; }
};

struct Session {
    long timestamp;
    int sessId;
    int mainState;
    int interState;
    bool allowAccept;
    bool allowCancel;
    SessionItem evening;
    SessionItem before;
    SessionItem inter;
    SessionItem after;
    bool operator==(const Session &o)
    {
        return sessId == o.sessId &&
               mainState == o.mainState &&
               interState == o.interState &&
               allowAccept == o.allowAccept &&
               allowCancel == o.allowCancel &&
               evening == o.evening &&
               before == o.before &&
               inter == o.inter &&
               after == o.after;
    }
    bool operator!=(const Session &o) { return (*this == o) ? false : true; }
};

struct Security {
    long timestamp;
    int sessId;
    int secId;
    int mainState;
    bool allowAccept;
    bool allowCancel;
    std::string symbol;
    int roundTo;
    double minStep;
    double minPrice;
    double maxPrice;
    bool operator==(const Security &o)
    {
        return sessId == o.sessId &&
               secId == o.secId &&
               mainState == o.mainState &&
               allowAccept == o.allowAccept &&
               allowCancel == o.allowCancel &&
               symbol == o.symbol &&
               roundTo == o.roundTo &&
               minStep == o.minStep &&
               minPrice == o.minPrice &&
               maxPrice == o.maxPrice;
    }
    bool operator!=(const Security &o) { return (*this == o) ? false : true; }
};

struct Heartbeat {
    long timestamp;
};

//
// events
class Event {
public:
    Event(EventType type) : type(type) {}
    virtual ~Event() {}
    EventType type;
};

class NewExecReport : public Event {
public:
    NewExecReport() : Event(EventType::NewExecReport) {}
    ExecReport report;
};

class NewExecTrade : public Event {
public:
    NewExecTrade() : Event(EventType::NewExecTrade) {}
    ExecTrade trade;
};

class NewObook : public Event {
public:
    NewObook() : Event(EventType::NewObook) {}
    Obook obook;
};

class NewTrade : public Event {
public:
    NewTrade() : Event(EventType::NewTrade) {}
    Trade trade;
};

class NewSession : public Event {
public:
    NewSession() : Event(EventType::NewSession) {}
    Session session;
};

class NewSecurity : public Event {
public:
    NewSecurity() : Event(EventType::NewSecurity) {}
    Security security;
};

class NewHeartbeat : public Event {
public:
    NewHeartbeat() : Event(EventType::NewHeartbeat) {}
    Heartbeat heartbeat;
};

class NewEndHistFile : public Event {
public:
    NewEndHistFile() : Event(EventType::NewEndHistFile) {}
    std::string file;
};

class NewEndHistData : public Event {
public:
    NewEndHistData() : Event(EventType::NewEndHistData) {}
};

// functions
long GetSystemTime();
long GetContextTime(Holder *hld);
std::vector<std::string> Split(const std::string &str, char delim);
bool CheckObook(const Obook &o);
double Round(double value, int roundTo);

std::string GetString(const Datetime &dt);
std::string GetString(const Statistics &s);
std::string GetString(const Order &o);
std::string GetString(const Obook &o);
std::string GetString(const Trade &t);
std::string GetString(const Session &s);
std::string GetString(const Security &s);
std::string GetString(const ExecStatus &s);
std::string GetString(const ExecDir &d);
std::string GetString(const ExecTif &t);
