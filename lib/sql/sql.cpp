#include <mysql/mysql.h>
#include <list>
#include <string>
#include <semaphore.h>
#include <mutex>

#include "sql.h"

using namespace std;

ConnectionPool::ConnectionPool()
{
    maxConnections_ = 0;
    currentConnections_ = 0;
    freeConnections_ = 0;
    enableLogging_ = 0; // Default to logging disabled.
}

ConnectionPool *ConnectionPool::GetInstance()
{
    static ConnectionPool connPool;
    return &connPool;
}

void ConnectionPool::Initialize(const std::string &url, const std::string &username, const std::string &password,
                                const std::string &databaseName, int port, int maxConnections, int enableLogging)
{
    serverUrl_ = url;
    serverPort_ = port;
    username_ = username;
    password_ = password;
    dbName_ = databaseName;
    maxConnections_ = maxConnections;
    enableLogging_ = enableLogging;

    for (int i = 0; i < maxConnections; i++)
    {
        if (CreateConnection())
        {
            freeConnections_++;
        }
    }

    sem_init(&semaphore_, 0, freeConnections_);
}

MYSQL *ConnectionPool::GetConnection()
{
    MYSQL *con = nullptr;

    if (sem_wait(&semaphore_) == 0)
    {
        std::unique_lock<std::mutex> lock(mutex_);

        if (!connectionList_.empty())
        {
            con = connectionList_.front();
            connectionList_.pop_front();
            freeConnections_--;
            currentConnections_++;

            if (!ValidateConnection(con))
            {
                con = nullptr;
                currentConnections_--;
            }
        }

        lock.unlock();
    }
    return con;
}

bool ConnectionPool::ReleaseConnection(MYSQL *connection)
{
    if (connection == nullptr)
    {
        return false;
    }

    std::unique_lock<std::mutex> lock(mutex_);

    connectionList_.push_back(connection);
    freeConnections_++;
    currentConnections_--;

    lock.unlock();

    sem_post(&semaphore_);
    return true;
}

int ConnectionPool::GetFreeConnections()
{
    return freeConnections_;
}

void ConnectionPool::DestroyPool()
{
    std::unique_lock<std::mutex> lock(mutex_);

    if (!connectionList_.empty())
    {
        for (auto it = connectionList_.begin(); it != connectionList_.end(); ++it)
        {
            MYSQL *con = *it;
            mysql_close(con);
        }
        connectionList_.clear();
    }

    lock.unlock();
    sem_destroy(&semaphore_);
}

ConnectionPool::~ConnectionPool()
{
    DestroyPool();
}

bool ConnectionPool::CreateConnection()
{
    MYSQL *con = nullptr;
    con = mysql_init(con);

    if (con == nullptr)
    {
        return false;
    }

    con = mysql_real_connect(con, serverUrl_.c_str(), username_.c_str(), password_.c_str(),
                             dbName_.c_str(), serverPort_, nullptr, 0);

    if (con == nullptr)
    {
        return false;
    }

    connectionList_.push_back(con);
    return true;
}

bool ConnectionPool::ValidateConnection(MYSQL *connection)
{
    if (mysql_ping(connection) == 0)
    {
        return true;
    }
    return false;
}

ConnectionRAII::ConnectionRAII(MYSQL **sqlConnection, ConnectionPool *connectionPool)
{
    sqlConnection_ = connectionPool->GetConnection();
    connectionPool_ = connectionPool;
}

ConnectionRAII::~ConnectionRAII()
{
    connectionPool_->ReleaseConnection(sqlConnection_);
}
