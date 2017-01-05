#include "sqlite3.hpp"

#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlError>
#include <QtSql/QSqlQuery>
#include <QVariant>

#include <cassert>
#include <cstring>
#include <cstdio>
#include <chrono>
#include <experimental/optional>

#include <mbgl/util/logging.hpp>
#include <mbgl/util/string.hpp>

namespace mapbox {
namespace sqlite {

class DatabaseImpl {
public:
    DatabaseImpl(const char* filename, int flags) : db(QSqlDatabase::addDatabase("QSQLITE")) {
        QString connectOptions;
        if (flags & OpenFlag::ReadOnly) {
            connectOptions.append("QSQLITE_OPEN_READONLY");
        }
        if (flags & OpenFlag::SharedCache) {
            if (connectOptions.isEmpty())
                connectOptions.append("QSQLITE_ENABLE_SHARED_CACHE");
            else
                connectOptions.append(";QSQLITE_ENABLE_SHARED_CACHE");
        }
        db.setConnectOptions(connectOptions);
        db.setDatabaseName(QString(filename));

        if (!db.open()) {
            QSqlError lastError = db.lastError();
            throw Exception { lastError.type(), lastError.text().toStdString() };
        }
    }

    ~DatabaseImpl() {
        db.close();
    }

    QSqlDatabase db;
};

class StatementImpl {
public:
    StatementImpl(const QString& sql, const QSqlDatabase& db) : query(sql, db) {
        checkError();
    }

    ~StatementImpl() {
        query.clear();
    }

    void checkError() {
        QSqlError lastError = query.lastError();
        if (lastError.type() != QSqlError::NoError) {
            throw Exception { lastError.type(), lastError.text().toStdString() };
        }
    }

    QSqlQuery query;
};

template <typename T>
using optional = std::experimental::optional<T>;


Database::Database(const std::string& file, int flags)
        : impl(std::make_unique<DatabaseImpl>(file.c_str(), flags)) {
}

Database::Database(Database &&other)
        : impl(std::move(other.impl)) {
}

Database &Database::operator=(Database &&other) {
    std::swap(impl, other.impl);
    return *this;
}

Database::~Database() {
}

Database::operator bool() const {
    return impl.operator bool();
}

void Database::setBusyTimeout(std::chrono::milliseconds timeout) {
    assert(impl);
    std::string timeoutStr = mbgl::util::toString(timeout.count());
    QString connectOptions = impl->db.connectOptions();
    if (connectOptions.isEmpty())
        connectOptions.append("QSQLITE_BUSY_TIMEOUT=").append(QString::fromStdString(timeoutStr));
    else
        connectOptions.append(";QSQLITE_BUSY_TIMEOUT=").append(QString::fromStdString(timeoutStr));

    if (impl->db.isOpen()) {
        impl->db.close();
    }
    impl->db.setConnectOptions(connectOptions);
    if (!impl->db.open()) {
        QSqlError lastError = impl->db.lastError();
        throw Exception { lastError.type(), lastError.text().toStdString() };
    }
}

void Database::exec(const std::string &sql) {
    assert(impl);
    QSqlQuery query = impl->db.exec(QString::fromStdString(sql));
    QSqlError lastError = query.lastError();
    if (lastError.type() != QSqlError::NoError) {
        throw Exception { lastError.type(), lastError.text().toStdString() };
    }
}

Statement Database::prepare(const char *query) {
    assert(impl);
    return Statement(this, query);
}

Statement::Statement(Database *db, const char *sql)
        : impl(std::make_unique<StatementImpl>(QString(sql), db->impl->db)) {
}

Statement::Statement(Statement &&other) {
    *this = std::move(other);
}

Statement &Statement::operator=(Statement &&other) {
    std::swap(impl, other.impl);
    return *this;
}

Statement::~Statement() {
}

Statement::operator bool() const {
    return impl.operator bool();
}

template void Statement::bind(int, std::nullptr_t);
template void Statement::bind(int, int8_t);
template void Statement::bind(int, int16_t);
template void Statement::bind(int, int32_t);
template void Statement::bind(int, int64_t);
template void Statement::bind(int, uint8_t);
template void Statement::bind(int, uint16_t);
template void Statement::bind(int, uint32_t);
template void Statement::bind(int, double);
template void Statement::bind(int, bool);

template <typename T>
void Statement::bind(int offset, T value) {
    assert(impl);
    impl->query.bindValue(offset, QVariant::fromValue<T>(value));
    impl->checkError();
}

template <>
void Statement::bind(int offset, std::chrono::time_point<std::chrono::system_clock, std::chrono::seconds> value) {
    return bind(offset, std::chrono::system_clock::to_time_t(value));
}

template <>
void Statement::bind(int offset, optional<std::chrono::time_point<std::chrono::system_clock, std::chrono::seconds>> value) {
    if (value)
        return bind(offset, *value);
    return bind(offset, nullptr);
}

template <>
void Statement::bind(int offset, optional<std::string> value) {
    if (value)
        return bind(offset, QString::fromStdString(*value));
    return bind(offset, nullptr);
}

void Statement::bind(int offset, const char * value, std::size_t length, bool retain) {
    assert(impl);
    if (length > std::numeric_limits<int>::max()) {
        throw std::range_error("value too long for sqlite3_bind_text");
    }

    // TODO
    (void)offset;
    (void)value;
    (void)length;
    (void)retain;
}

void Statement::bind(int offset, const std::string& value, bool retain) {
    bind(offset, value.data(), value.size(), retain);
}

void Statement::bindBlob(int offset, const void * value, std::size_t length, bool retain) {
    assert(impl);
    if (length > std::numeric_limits<int>::max()) {
        throw std::range_error("value too long for sqlite3_bind_text");
    }

    // TODO
    (void)offset;
    (void)value;
    (void)length;
    (void)retain;
}

void Statement::bindBlob(int offset, const std::vector<uint8_t>& value, bool retain) {
    bindBlob(offset, value.data(), value.size(), retain);
}

bool Statement::run() {
    assert(impl);
    // TODO
    return false;
}

template int8_t Statement::get(int);
template int16_t Statement::get(int);
template int32_t Statement::get(int);
template int64_t Statement::get(int);
template uint8_t Statement::get(int);
template uint16_t Statement::get(int);
template uint32_t Statement::get(int);
template double Statement::get(int);
template bool Statement::get(int);
template std::vector<uint8_t> Statement::get(int);

template <typename T> T Statement::get(int offset) {
    assert(impl);
    QVariant value = impl->query.boundValue(offset);
    impl->checkError();
    return value.value<T>();
}

template <> std::chrono::time_point<std::chrono::system_clock, std::chrono::seconds> Statement::get(int offset) {
    assert(impl);
    QVariant value = impl->query.boundValue(offset);
    impl->checkError();
    return std::chrono::time_point_cast<std::chrono::seconds>(
        std::chrono::system_clock::from_time_t(value.value<std::time_t>()));
}

template <> optional<int64_t> Statement::get(int offset) {
    assert(impl);
    QVariant value = impl->query.boundValue(offset);
    impl->checkError();
    if (value.isNull())
        return optional<int64_t>();
    return optional<int64_t>(value.value<int64_t>());
}

template <> optional<double> Statement::get(int offset) {
    assert(impl);
    QVariant value = impl->query.boundValue(offset);
    impl->checkError();
    if (value.isNull())
        return optional<double>();
    return optional<double>(value.value<double>());
}

template <> std::string Statement::get(int offset) {
    assert(impl);
    QVariant value = impl->query.boundValue(offset);
    impl->checkError();
    return value.toString().toStdString();
}

template <> optional<std::string> Statement::get(int offset) {
    assert(impl);
    QVariant value = impl->query.boundValue(offset);
    impl->checkError();
    if (value.isNull())
        return optional<std::string>();
    return optional<std::string>(value.toString().toStdString());
}

template <>
optional<std::chrono::time_point<std::chrono::system_clock, std::chrono::seconds>> Statement::get(int offset) {
    assert(impl);
    QVariant value = impl->query.boundValue(offset);
    impl->checkError();
    if (value.isNull())
        return optional<std::chrono::time_point<std::chrono::system_clock, std::chrono::seconds>>();
    return optional<std::chrono::time_point<std::chrono::system_clock, std::chrono::seconds>>(
            std::chrono::time_point_cast<std::chrono::seconds>(
                std::chrono::system_clock::from_time_t(value.value<std::time_t>())));
}

void Statement::reset() {
    assert(impl);
    impl->query.clear();
}

void Statement::clearBindings() {
    assert(impl);
    // FIXME: Find a better way of doing this.
    impl->query.clear();
}

int64_t Statement::lastInsertRowId() const {
    assert(impl);
    return impl->query.lastInsertId().value<int64_t>();
}

uint64_t Statement::changes() const {
    assert(impl);
    return impl->query.numRowsAffected();
}

Transaction::Transaction(Database& db_, Mode mode)
        : db(db_) {
    switch (mode) {
    case Deferred:
        db.exec("BEGIN DEFERRED TRANSACTION");
        break;
    case Immediate:
        db.exec("BEGIN IMMEDIATE TRANSACTION");
        break;
    case Exclusive:
        db.exec("BEGIN EXCLUSIVE TRANSACTION");
        break;
    }
}

Transaction::~Transaction() {
    if (needRollback) {
        try {
            rollback();
        } catch (...) {
            // Ignore failed rollbacks in destructor.
        }
    }
}

void Transaction::commit() {
    needRollback = false;
    db.exec("COMMIT TRANSACTION");
}

void Transaction::rollback() {
    needRollback = false;
    db.exec("ROLLBACK TRANSACTION");
}

} // namespace sqlite
} // namespace mapbox
