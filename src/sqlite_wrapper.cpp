#include <algorithm>
#include <string.h>
#include "log.h"
#include "sqlite_wrapper.h"

SqliteWrapper::SqliteWrapper(const std::string &path) {
    int ret = 0;
    if ((ret = sqlite3_open(path.c_str(), &db)) != SQLITE_OK)
    {
        TB_LOG_ERROR("Can't open database: %s", sqlite3_errmsg(db));
        goto end;
    }
    if (db == nullptr)
    {
        TB_LOG_ERROR("Can't open database, unexpected NULL db handler");
        ret = -EINVAL;
        goto end;
    }
    TB_LOG_DEBUG("DB Opened: %s", path.c_str());
    db_ok = true;
end:
    return;
}

SqliteWrapper::~SqliteWrapper() {
    if (db != nullptr) {
        TB_LOG_DEBUG("DB Closed");
        sqlite3_close(db);
    }
}

int SqliteWrapper::create_table(const std::string &table_name,
        const std::string &sql_part)
{
    std::unique_lock<std::mutex> lock(_mutex);
    std::string sql_str = "CREATE TABLE if not exists " + table_name +
        " (" + sql_part + ");";
    char *err_msg = NULL;
    int ret = 0;

    TB_LOG_DEBUG("sql: %s", sql_str.c_str());
    ret = sqlite3_exec(db, sql_str.c_str(), 0, 0, &err_msg);
    if (ret != SQLITE_OK)
    {
        TB_LOG_ERROR("create table err: %s", err_msg);
        sqlite3_free(err_msg);
    }
    return ret;
}

bool SqliteWrapper::peek_entry(const std::string &table_name,
            const std::string &sql_part)
{
    std::unique_lock<std::mutex> lock(_mutex);
    return __peek_entry(table_name, sql_part);
}

int SqliteWrapper::insert_entry(const std::string &table_name,
        const std::string &sql_part,
        std::map<const std::string, std::vector<uint8_t>*> *blobs)
{
    std::unique_lock<std::mutex> lock(_mutex);
    return __insert_entry(table_name, sql_part, blobs);
}

int SqliteWrapper::update_entry(const std::string &table_name,
            const std::string &sql_part_update,
            const std::string &sql_part_filter,
            std::map<const std::string, std::vector<uint8_t>*> *blobs)
{
    std::unique_lock<std::mutex> lock(_mutex);
    return __update_entry(table_name, sql_part_update,
            sql_part_filter, blobs);
}

int SqliteWrapper::insert_update_entry(const std::string &table_name,
            const std::string &sql_part_insert,
            const std::string &sql_part_update,
            const std::string &sql_part_filter,
            std::map<const std::string, std::vector<uint8_t>*> *blobs)
{
    std::unique_lock<std::mutex> lock(_mutex);
    if (__peek_entry(table_name, sql_part_filter))
        return __update_entry(table_name, sql_part_update,
                sql_part_filter, blobs);
    else
        return __insert_entry(table_name, sql_part_insert, blobs);
}

int SqliteWrapper::delete_entry(const std::string &table_name,
            const std::string &sql_part)
{
    std::unique_lock<std::mutex> lock(_mutex);
    return __delete_entry(table_name, sql_part);
}

int SqliteWrapper::delete_all_entry(const std::string &table_name)
{
    std::unique_lock<std::mutex> lock(_mutex);
    return __delete_all_entry(table_name);
}

int SqliteWrapper::get_entry(std::vector<GetItem> &out,
            const std::string &table_name,
            const std::string &sql_values,
            const std::string &sql_filter)
{
    std::unique_lock<std::mutex> lock(_mutex);
    return __get_entry(out, table_name, sql_values, sql_filter);
}

bool SqliteWrapper::__peek_entry(const std::string &table_name,
            const std::string &sql_part)
{
    std::string sql_str = "SELECT * FROM " + table_name + " " +
        sql_part + ";";
    sqlite3_stmt *stmt;

    TB_LOG_DEBUG("sqlite3 prepare: %s", sql_str.c_str());
    if (sqlite3_prepare_v2(db, sql_str.c_str(), -1, &stmt,
                NULL) !=
            SQLITE_OK)
    {
        TB_LOG_ERROR("sqlite3 prepare failed");
        goto SQILTE3_PREPARE_FAILED;
    }
    if (sqlite3_step(stmt) != SQLITE_ROW)
    {
        goto SQILTE3_STEP_FAILED;
    }
    sqlite3_finalize(stmt);
    return true;

SQILTE3_STEP_FAILED:
    sqlite3_finalize(stmt);
SQILTE3_PREPARE_FAILED:
    return false;
}

int SqliteWrapper::__insert_entry(const std::string &table_name,
            const std::string &sql_part,
            std::map<const std::string, std::vector<uint8_t>*> *blobs)
{
    std::string sql_str = "INSERT INTO " + table_name + " " + sql_part + ";";

    return __exec_sql_1(sql_str, blobs);
}

int SqliteWrapper::__update_entry(const std::string &table_name,
            const std::string &sql_update,
            const std::string &sql_filter,
            std::map<const std::string, std::vector<uint8_t>*> *blobs)
{
    std::string sql_str = "UPDATE " + table_name + " SET " + sql_update + " " +
        sql_filter + ";";

    return __exec_sql_1(sql_str, blobs);
}

int SqliteWrapper::__delete_entry(const std::string &table_name,
            const std::string &sql_part)
{
    std::string sql_str = "DELETE from " + table_name + " " +
        sql_part;
    return __exec_sql_1(sql_str);
}

int SqliteWrapper::__delete_all_entry(const std::string &table_name)
{
    std::string sql_str = "DELETE from " + table_name + ";";
    return __exec_sql_1(sql_str);
}

int SqliteWrapper::__exec_sql_1(const std::string &sql_str,
            std::map<const std::string, std::vector<uint8_t>*> *blobs)
{
    sqlite3_stmt *stmt;
    TB_LOG_DEBUG("sqlite3 prepare: %s", sql_str.c_str());
    int ret;
    if (sqlite3_prepare_v2(db, sql_str.c_str(), -1, &stmt,
                NULL) !=
            SQLITE_OK)
    {
        TB_LOG_ERROR("sqlite3 prepare failed");
        goto SQILTE3_PREPARE_FAILED;
    }

    if (blobs == nullptr)
        goto DONE_BLOBS;
    for(auto const& itr : *blobs) {
        auto name = itr.first;
        auto buf = itr.second;
        auto idx = sqlite3_bind_parameter_index(stmt, name.c_str());
        if (idx == 0) {
            TB_LOG_ERROR("Cannot find bind field name: %s", name.c_str());
            goto SQILTE3_BIND_FAILED;
        }
        if ((ret = sqlite3_bind_blob(stmt, idx, buf->data(),
                    buf->size(), SQLITE_STATIC)) != SQLITE_OK)
        {
            TB_LOG_ERROR("sqlite3 bind failed: %d", ret);
            goto SQILTE3_BIND_FAILED;
        }
    }
DONE_BLOBS:
    if (sqlite3_step(stmt) != SQLITE_DONE)
    {
        TB_LOG_ERROR("sqlite3 step failed");
        goto SQILTE3_STEP_FAILED;
    }
    sqlite3_finalize(stmt);
    return 0;
SQILTE3_STEP_FAILED:
SQILTE3_BIND_FAILED:
    sqlite3_finalize(stmt);
SQILTE3_PREPARE_FAILED:
    return -EAGAIN;
}

int SqliteWrapper::__get_entry(std::vector<GetItem> &out,
        const std::string &table_name,
        const std::string &sql_values,
        const std::string &sql_filter)
{
    sqlite3_stmt *stmt;
    std::string sql_str = "SELECT " + sql_values + " FROM " + table_name +
        " " + sql_filter + ";";
    int idx = 0;
    int ret = 0;

    TB_LOG_DEBUG("sqlite3 get: %s", sql_str.c_str());
    if (sqlite3_prepare_v2(db, sql_str.c_str(), -1, &stmt,
                NULL) !=
            SQLITE_OK)
    {
        TB_LOG_ERROR("sqlite3 prepare failed");
        ret = -EINVAL;
        goto SQILTE3_PREPARE_FAILED;
    }
    if (sqlite3_step(stmt) != SQLITE_ROW)
    {//The key might not found
        TB_LOG_DEBUG("sqlite3 step failed");
        ret = -ENOENT;
        goto SQILTE3_STEP_FAILED;
    }

    for(auto const &itr : out) {
        auto type = sqlite3_column_type(stmt, idx);
        auto buf = itr.buf;
        auto copy = itr.ext_copy;

        switch(type) {
            case SQLITE_INTEGER:
                if (itr.len < 8)
                    *(int *)buf = sqlite3_column_int(stmt, idx);
                else
                    *(int64_t *)buf = sqlite3_column_int64(stmt, idx);
                break;
            case SQLITE_FLOAT:
                *(double *)buf = sqlite3_column_double(stmt, idx);
                break;
            case SQLITE_TEXT:
                if (copy != nullptr) {
                    if (copy(sqlite3_column_text(stmt, idx),
                                (uint32_t)sqlite3_column_bytes(stmt, idx)) != 0) {
                        ret = -ENOMEM;
                        goto END;
                    }
                } else {
                    memcpy(buf, sqlite3_column_text(stmt, idx),
                        std::min(itr.len, (uint32_t)sqlite3_column_bytes(stmt, idx)));
                }
                break;
            case SQLITE_BLOB:
                if (copy != nullptr) {
                    if (copy(sqlite3_column_blob(stmt, idx),
                                (uint32_t)sqlite3_column_bytes(stmt, idx)) != 0) {
                        ret = -ENOMEM;
                        goto END;
                    }
                } else {
                    memcpy(buf, sqlite3_column_blob(stmt, idx),
                        std::min(itr.len, (uint32_t)sqlite3_column_bytes(stmt, idx)));
                }
                break;
            default:
                TB_LOG_ERROR("Unexpected SQL NULL type in col: %d", idx);
                ret = -EINVAL;
                goto END;
        }
        idx++;
    }
END:
    sqlite3_finalize(stmt);
    return ret;

SQILTE3_STEP_FAILED:
    sqlite3_finalize(stmt);
SQILTE3_PREPARE_FAILED:
    return ret;
}
/*
int SqliteWrapper::create_table_byjson(const std::string &para)
{
    Json::CharReaderBuilder builder;
    Json::CharReader *reader = builder.newCharReader();
    Json::Value j_root;
    JSONCPP_STRING errors;
    int ret = 0;

    if (reader == nullptr) {
        ret = -ENOMEM;
        goto end;
    }
    if (reader->parse((char *)para.data(),
                (char *)para.data() + para.size(),
                &j_root, &errors) == false)
    {
        TB_LOG_WARNING("Json parse err:%s\n", errors.c_str());
        ret = -EINVAL;
        goto end;
    }
    try {
        std::string sql_str;
        std::string table_name = j_root["table"].asString();
        char *err_msg = NULL;

        sql_str = "CREATE TABLE if not exists " + table_name + " (";

        for (uint32_t i = 0; i < j_root["fields"].size(); i++) {
            Json::Value j_field = j_root["fields"][i];

            sql_str += j_field["name"].asString() + " ";
            sql_str += j_field["type"].asString() + " ";
            if (j_field.isMember("compulsory") &&
                    j_field["compulsory"].asBool() == true)
                sql_str += "NOT NULL ";

            if (i < j_root["fields"].size() - 1)
                sql_str += ", ";
        }
        sql_str += ");";
        TB_LOG_DEBUG("sql: %s", sql_str.c_str());
        ret = sqlite3_exec(db, sql_str.c_str(), 0, 0, &err_msg);
        if (ret != SQLITE_OK)
        {
            TB_LOG_ERROR("create table err: %s", err_msg);
            free(err_msg);
        }
    } catch(std::exception &e) {
        TB_LOG_ERROR("Parse Json for Create Table Exception: %s", e.what());
        ret = -EINVAL;
    }
end:
    if (reader != nullptr)
        delete reader;
    return ret;
}
*/
