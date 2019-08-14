#ifndef __SQLITE_WRAPPER_H__
#define __SQLITE_WRAPPER_H__

#include <functional>
#include <map>
#include <mutex>
#include <sqlite3.h>
#include <string>
#include <vector>

class SqliteWrapper {
public:

    SqliteWrapper(const std::string &path);
    ~SqliteWrapper();
    /*
     * create_table: expects fields part only sql statement:
     * e.g.: field1 INT,...
     *
     * The function will construct the complete sql statement, like the
     * following:
     *
     * "CREATE TABLE if not exist <table_name> (field1 INT,...);"
     */
    int create_table(const std::string &table_name,
            const std::string &sql_part);

    /*
     * peek_entry: expects condition part only sql statemeent
     *
     * e.g.: "WHERE name = xxx AND value = xxx"
     *
     * The function will construct the complete sql statement for querying,
     * like the followling:
     *
     * "SELECT * from <table_name> WHERE name = xxx AND value = xxx;"
     * 
     */
    bool peek_entry(const std::string &table_name,
            const std::string &sql_part);

    /*
     * insert_entry: expect part sql statement in the following format:
     *
     * e.g.: (name1, name2, ...) VALUES (value1, value2, ...)
     *
     * The function will construt the complete sql statement:
     *
     * INSERT INTO <table_name> (name1, name2, ...) VALUES (value1, value2, ...);
     *
     * Blob parameters can be provided, which will be bind by the function
     */
    int insert_entry(const std::string &table_name,
            const std::string &sql_part,
            std::map<const std::string, std::vector<uint8_t>*> *blobs = nullptr);
    /*
     * update_entry: expect part sql statement in the following format:
     *
     * e.g.: WHERE name1 = value1, name2 = value2
     *
     * Expect filter sql statement in the follwoing format:
     * e.g.: "name1 = value1"
     * The function will construt the complete sql statement:
     *
     * UPDATE <table_name> SET name1 = value1, name2 = value2 WHERE name1 =
     * value1
     *
     * Blob parameters can be provided, which will be bind by the function
     */
    int update_entry(const std::string &table_name,
            const std::string &sql_part_update,
            const std::string &sql_part_filter,
            std::map<const std::string, std::vector<uint8_t>*> *blobs = nullptr);

    int insert_update_entry(const std::string &table_name,
            const std::string &sql_part_insert,
            const std::string &sql_part_update,
            const std::string &sql_part_filter,
            std::map<const std::string, std::vector<uint8_t>*> *blobs = nullptr);
    /*
     * delete_entry: expects condition part only sql statemeent
     *
     * e.g.: "WHERE name = xxx AND value = xxx"
     *
     * The function will construct the complete sql statement for querying,
     * like the followling:
     *
     * "DELETE * from <table_name> WHERE name = xxx AND value = xxx;"
     *
     */
    int delete_entry(const std::string &table_name,
            const std::string &sql_part);
    int delete_all_entry(const std::string &table_name);
    class GetItem{
        public:
            void *buf;  // data pointer
            uint32_t len;   //only valid if type is blob, text
            std::function<int(const void*, uint32_t)> ext_copy = nullptr;
        GetItem(void *buf, uint32_t len,
                std::function<int(const void*, uint32_t)> _ext_copy = nullptr) :
                buf(buf), len(len), ext_copy(_ext_copy){}
    };
    /*
     * get_entry: get an entry from db
     *
     * out: an array of structure for output
     *
     * sql_values: expect the format: field1, field2, ....
     *
     * sql_filter: expect the format: e.g.: field1 = value1 AND field2 = value2...
     */
    int get_entry(std::vector<GetItem> &out,
            const std::string &table_name,
            const std::string &sql_values,
            const std::string &sql_filter);
    bool is_ok(void) {
        return db_ok;
    }
private:
    bool __peek_entry(const std::string &table_name,
            const std::string &sql_part);
    int __insert_entry(const std::string &table_name,
            const std::string &sql_part,
            std::map<const std::string, std::vector<uint8_t>*> *blobs = nullptr);
    int __update_entry(const std::string &table_name,
            const std::string &sql_update,
            const std::string &sql_filter,
            std::map<const std::string, std::vector<uint8_t>*> *blobs = nullptr);
    int __delete_entry(const std::string &table_name,
            const std::string &sql_part);
    int __delete_all_entry(const std::string &table_name);
    int __exec_sql_1(const std::string &sql_str,
            std::map<const std::string, std::vector<uint8_t>*> *blobs = nullptr);
    int __get_entry(std::vector<GetItem> &out,
            const std::string &table_name,
            const std::string &sql_values,
            const std::string &sql_filter);
    sqlite3 *db = nullptr;
    bool db_ok = false;
    std::mutex _mutex;
};


#if 0
    /*
     * Create table: expect stringify json parameter
     * {
     *   "table": <table_name>,
     *   "fields": 
     *   [
     *      {
     *          "name": <field_name>,
     *          "type": <field type>,
     *          "compulsory": <true|false>
     *      },
     *      {
     *        ...
     *      }
     *   ]
     * }
     */
    int create_table_byjson(const std::string &para);
    /*
     * peak_entry: expect stringify json parameter
     * {
     *   "table": <table_name>,
     *   "condition": <and|or>
     *   "fields" :
     *   [
     *      {
     *          "name": <field_name>,
     *          "condition": "= | != | < | > | <= | >= |",
     *          "value": <field_value>
     *      },
     *      {
     *          ...
     *      }
     *   ]
     * }
     */
    int peek_entry_byjson(const std::string &para);
    /*
     * insert_entry: expect stringify json parameter
     * {
     *   "table": <table_name>,
     *   "update": <true|false>
     *   "fields":
     *   [
     *      {
     *          "name": <field name>,
     *          "value": <field_value>,
     *          "blob_index": <>    //optional, if value is not exists
     *      },
     *   ]
     * }
     */
    int insert_entry(const std::string &para);
    int remove_entry(const std::string &para);
#endif

#endif
