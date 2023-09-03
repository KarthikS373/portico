#ifndef _PORTICO_CONNECTION_POOL_
#define _PORTICO_CONNECTION_POOL_

#include <mysql/mysql.h>
#include <list>
#include <string>
#include <semaphore.h>

/**
 * @brief A class for managing a pool of MySQL database connections.
 */
class ConnectionPool
{
public:
    /**
     * @brief Get an available database connection from the pool.
     * @return A MySQL database connection or nullptr if the pool is empty.
     */
    MYSQL *GetConnection();

    /**
     * @brief Release a database connection back to the pool for reuse.
     * @param connection The MySQL database connection to release.
     * @return true if the connection was successfully released, false otherwise.
     */
    bool ReleaseConnection(MYSQL *connection);

    /**
     * @brief Get the number of free connections in the pool.
     * @return The number of free connections.
     */
    int GetFreeConnections();

    /**
     * @brief Initialize the connection pool with the specified parameters.
     * @param url The database server URL.
     * @param username The database username.
     * @param password The database password.
     * @param databaseName The name of the database to use.
     * @param port The database server port.
     * @param maxConnections The maximum number of connections in the pool.
     * @param enableLogging Enable or disable logging (0 for disabled, 1 for enabled).
     */
    void Initialize(const std::string &url, const std::string &username, const std::string &password,
                    const std::string &databaseName, int port, int maxConnections, int enableLogging);

    /**
     * @brief Destroy the connection pool and release all connections.
     */
    void DestroyPool();

    /**
     * @brief Singleton pattern: Get the instance of the ConnectionPool.
     * @return The ConnectionPool instance.
     */
    static ConnectionPool *GetInstance();

private:
    ConnectionPool();  // Private constructor for Singleton pattern.
    ~ConnectionPool(); // Destructor to destroy the pool.

    std::mutex mutex_; // Mutex to control access to the pool.

    int maxConnections_;     // The maximum number of connections in the pool.
    int currentConnections_; // The current number of connections in use.
    int freeConnections_;    // The current number of free connections.
    int enableLogging_;      // Logging flag (0 for disabled, 1 for enabled).

    std::string serverUrl_; // The database server URL.
    int serverPort_;        // The database server port.
    std::string username_;  // The database username.
    std::string password_;  // The database password.
    std::string dbName_;    // The name of the database to use.

    std::list<MYSQL *> connectionList_; // List of database connections.
    sem_t semaphore_;                   // Semaphore to control access to connections.

    /**
     * @brief Create and add a database connection to the pool.
     * @return true if the connection was successfully added, false otherwise.
     */
    bool CreateConnection();

    /**
     * @brief Validate the provided database connection.
     * @param connection The database connection to validate.
     * @return true if the connection is valid, false otherwise.
     */
    bool ValidateConnection(MYSQL *connection);
};

/**
 * @brief A RAII wrapper for automatic management of database connections.
 */
class ConnectionRAII
{
public:
    /**
     * @brief Construct a ConnectionRAII object and acquire a database connection.
     * @param sqlConnection Pointer to a MySQL connection (output parameter).
     * @param connectionPool The connection pool from which to acquire the connection.
     */
    ConnectionRAII(MYSQL **sqlConnection, ConnectionPool *connectionPool);

    /**
     * @brief Destructor to automatically release the acquired database connection.
     */
    ~ConnectionRAII();

private:
    MYSQL *sqlConnection_;           // Pointer to the acquired database connection.
    ConnectionPool *connectionPool_; // The connection pool used for acquiring and releasing connections.
};

#endif
