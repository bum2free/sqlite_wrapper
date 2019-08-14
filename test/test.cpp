#include "sqlite_wrapper.h"
#include <gtest/gtest.h>
#include <gmock/gmock.h>

const std::string db_file_path = "./test.db";

class TestSqliteWrapper : public testing::Test
{
public:
    SqliteWrapper *sw = nullptr;

    void SetUp(void) {
        sw = new SqliteWrapper(db_file_path);
    }
    void TearDown(void) {
        if (sw != nullptr) {
            delete sw;
            sw = nullptr;
        }
        remove(db_file_path.c_str());
    }
};

TEST_F(TestSqliteWrapper, test_create_db)
{
    ASSERT_TRUE(sw != nullptr);
    ASSERT_TRUE(sw->is_ok());
}

TEST_F(TestSqliteWrapper, test_create_table)
{
    ASSERT_TRUE(sw != nullptr);
    ASSERT_TRUE(sw->is_ok());

    std::string sql_str = "num1 INT, str1 TEXT, data BLOB";
    std::string table_name = "dummy_1";

    ASSERT_EQ(0,sw->create_table(table_name, sql_str));
}

TEST_F(TestSqliteWrapper, test_insert_simple)
{
    ASSERT_TRUE(sw != nullptr);
    ASSERT_TRUE(sw->is_ok());

    std::string table_name = "dummy_1";

    //create table
    {
        std::string sql_str = "num1 INT, str1 TEXT";

        ASSERT_EQ(0,sw->create_table(table_name, sql_str));
    }

    //insert a simple entry
    {
        int src_num = 123456;
        std::string src_str = "hello world";
        std::string sql_str = "(num1, str1) VALUES (" + std::to_string(src_num) + ", \"" + src_str +
            "\")";
        ASSERT_EQ(0,sw->insert_entry(table_name, sql_str));
    }
}

TEST_F(TestSqliteWrapper, test_peek_simple_ok)
{
    ASSERT_TRUE(sw != nullptr);
    ASSERT_TRUE(sw->is_ok());

    std::string table_name = "dummy_1";
    int src_num = 123456;
    std::string src_str = "hello world";

    //create table
    {
        std::string sql_str = "num1 INT, str1 TEXT";

        ASSERT_EQ(0,sw->create_table(table_name, sql_str));
    }
    //expect could not be queried, as there is no entry yet
    {
        std::string sql_str = "num1 = " + std::to_string(src_num);
        ASSERT_FALSE(sw->peek_entry(table_name, sql_str));
    }
    //insert a simple entry
    {

        std::string sql_str = "(num1, str1) VALUES (" + std::to_string(src_num) + ", \"" + src_str +
            "\")";
        ASSERT_EQ(0,sw->insert_entry(table_name, sql_str));
    }

    //expect could be queried, with correct num1
    {
        std::string sql_str = "WHERE num1 = " + std::to_string(src_num);
        ASSERT_TRUE(sw->peek_entry(table_name, sql_str));
    }
    //expect could be queried, with correct num1 and correct str1
    {
        std::string sql_str = "WHERE num1 = " + std::to_string(src_num) +
            " AND str1 = \"" + src_str + "\"";
        ASSERT_TRUE(sw->peek_entry(table_name, sql_str));
    }
    //expect could be queried, with correct num1 or incorrect str1
    {
        std::string sql_str = "WHERE num1 = " + std::to_string(src_num) +
            " OR str1 = \"" + std::to_string(654321) + "\"";
        ASSERT_TRUE(sw->peek_entry(table_name, sql_str));
    }
}

TEST_F(TestSqliteWrapper, test_peek_simple_notexist)
{
    ASSERT_TRUE(sw != nullptr);
    ASSERT_TRUE(sw->is_ok());

    std::string table_name = "dummy_1";
    int src_num = 123456;
    std::string src_str = "hello world";

    //create table
    {
        std::string sql_str = "num1 INT, str1 TEXT";

        ASSERT_EQ(0,sw->create_table(table_name, sql_str));
    }
    //expect could not be queried, as there is no entry yet
    {
        std::string sql_str = "WHERE num1 = " + std::to_string(src_num);
        ASSERT_FALSE(sw->peek_entry(table_name, sql_str));
    }
    //insert a simple entry
    {

        std::string sql_str = "(num1, str1) VALUES (" + std::to_string(src_num) + ", \"" + src_str +
            "\")";
        ASSERT_EQ(0,sw->insert_entry(table_name, sql_str));
    }

    //expect could not be queried, with correct num1 and incorrect str1
    {
        std::string sql_str = "WHERE num1 = " + std::to_string(src_num) +
            " AND str1 = \"" + std::to_string(654321) + "\"";
        ASSERT_FALSE(sw->peek_entry(table_name, sql_str));
    }
}

TEST_F(TestSqliteWrapper, test_delete_simple)
{
    ASSERT_TRUE(sw != nullptr);
    ASSERT_TRUE(sw->is_ok());

    std::string table_name = "dummy_1";
    int src_num = 123456;
    std::string src_str = "hello world";

    //create table
    {
        std::string sql_str = "num1 INT, str1 TEXT";

        ASSERT_EQ(0,sw->create_table(table_name, sql_str));
    }
    //expect could not be queried, as there is no entry yet
    {
        std::string sql_str = "WHERE num1 = " + std::to_string(src_num);
        ASSERT_FALSE(sw->peek_entry(table_name, sql_str));
    }
    //insert a simple entry
    {

        std::string sql_str = "(num1, str1) VALUES (" + std::to_string(src_num) + ", \"" + src_str +
            "\")";
        ASSERT_EQ(0,sw->insert_entry(table_name, sql_str));
    }
    //expect could be queried, with correct num1
    {
        std::string sql_str = "WHERE num1 = " + std::to_string(src_num);
        ASSERT_TRUE(sw->peek_entry(table_name, sql_str));
    }
    //Delete the entry
    {
        std::string sql_str = "WHERE num1 = " + std::to_string(src_num);
        ASSERT_EQ(0, sw->delete_entry(table_name, sql_str));
    }
    //expect could not be queried
    {
        std::string sql_str = "WHERE num1 = " + std::to_string(src_num);
        ASSERT_FALSE(sw->peek_entry(table_name, sql_str));
    }
}

TEST_F(TestSqliteWrapper, test_insert_blob)
{
    ASSERT_TRUE(sw != nullptr);
    ASSERT_TRUE(sw->is_ok());

    std::string table_name = "dummy_1";
    int src_num = 123456;
    std::string src_str = "hello world";
    std::vector<uint8_t> data = {65, 66, 67, 68, 69, 70};

    //create table
    {
        std::string sql_str = "num1 INT, str1 TEXT, data1 BLOB";

        ASSERT_EQ(0,sw->create_table(table_name, sql_str));
    }
    //insert
    {
        std::string sql_str = "(num1, str1, data1) VALUES (" + std::to_string(src_num) + ", \"" + src_str +
            "\", @_p1)"; //place holder variable as _p1
        std::map<const std::string, std::vector<uint8_t>*> blobs = {
            std::make_pair("@_p1", &data)   //should have the same place holder variable name
        };
        ASSERT_EQ(0,sw->insert_entry(table_name, sql_str, &blobs));
    }
}

TEST_F(TestSqliteWrapper, test_get_blob)
{
    ASSERT_TRUE(sw != nullptr);
    ASSERT_TRUE(sw->is_ok());

    std::string table_name = "dummy_1";
    int src_num = 123456;
    std::string src_str = "hello world";
    std::vector<uint8_t> src_data = {65, 66, 67, 68, 69, 70};

    //create table
    {
        std::string sql_str = "num1 INT, str1 TEXT, data1 BLOB";

        ASSERT_EQ(0,sw->create_table(table_name, sql_str));
    }
    //insert
    {
        std::string sql_str = "(num1, str1, data1) VALUES (" + std::to_string(src_num) + ", \"" + src_str +
            "\", @_p1)"; //place holder variable as _p1
        std::map<const std::string, std::vector<uint8_t>*> blobs = {
            std::make_pair("@_p1", &src_data)   //should have the same place holder variable name
        };
        ASSERT_EQ(0,sw->insert_entry(table_name, sql_str, &blobs));
    }
    //get
    {
        std::string sql_values = "num1, data1";
        std::string sql_filter = "WHERE num1 = " + std::to_string(src_num);
        int out_num;
        std::vector<uint8_t> out_buf;
        out_buf.resize(16);

        std::vector<SqliteWrapper::GetItem> out = {
            SqliteWrapper::GetItem(&out_num, 0),
            SqliteWrapper::GetItem(out_buf.data(), out_buf.size())
        };

        ASSERT_EQ(0, sw->get_entry(out, table_name, sql_values, sql_filter));

        //check result
        ASSERT_EQ(src_num, out_num);
        ASSERT_EQ(0, memcmp(src_data.data(), out_buf.data(), src_data.size()));
    }
}

TEST_F(TestSqliteWrapper, test_update_blob)
{
    ASSERT_TRUE(sw != nullptr);
    ASSERT_TRUE(sw->is_ok());

    std::string table_name = "dummy_1";
    int src_num = 123456;
    std::string src_str = "hello world";
    std::vector<uint8_t> src_data = {65, 66, 67, 68, 69, 70};

    //create table
    {
        std::string sql_str = "num1 INT, str1 TEXT, data1 BLOB";

        ASSERT_EQ(0,sw->create_table(table_name, sql_str));
    }
    //first insert_update, should be insert operation
    {
        std::string sql_str = "(num1, str1, data1) VALUES (" + std::to_string(src_num) + ", \"" + src_str +
            "\", @_p1)"; //place holder variable as _p1

        std::map<const std::string, std::vector<uint8_t>*> blobs = {
            std::make_pair("@_p1", &src_data)   //should have the same place holder variable name
        };
        ASSERT_EQ(0,sw->insert_entry(table_name, sql_str, &blobs));
    }
    //first get
    {
        std::string sql_values = "num1, data1";
        std::string sql_filter = "WHERE num1 = " + std::to_string(src_num);
        int out_num;
        std::vector<uint8_t> out_buf;
        out_buf.resize(16);

        std::vector<SqliteWrapper::GetItem> out = {
            SqliteWrapper::GetItem(&out_num, 0),
            SqliteWrapper::GetItem(out_buf.data(), out_buf.size())
        };

        ASSERT_EQ(0, sw->get_entry(out, table_name, sql_values, sql_filter));

        //check result
        ASSERT_EQ(src_num, out_num);
        ASSERT_EQ(0, memcmp(src_data.data(), out_buf.data(), src_data.size()));
    }
    //second insert_update, should be update operation
    src_data = {31, 32, 33, 34};
    {
        std::string sql_str = "num1 = " + std::to_string(src_num) +
            ", data1 = @_p1"; //place holder variable as _p1
        std::string sql_filter = "WHERE num1 = " + std::to_string(src_num);

        std::map<const std::string, std::vector<uint8_t>*> blobs = {
            std::make_pair("@_p1", &src_data)   //should have the same place holder variable name
        };
        ASSERT_EQ(0,sw->update_entry(table_name, sql_str, sql_filter, &blobs));
    }
    //second get
    {
        std::string sql_values = "num1, data1";
        std::string sql_filter = "WHERE num1 = " + std::to_string(src_num);
        int out_num;
        std::vector<uint8_t> out_buf;
        out_buf.resize(16);

        std::vector<SqliteWrapper::GetItem> out = {
            SqliteWrapper::GetItem(&out_num, 0),
            SqliteWrapper::GetItem(out_buf.data(), out_buf.size())
        };

        ASSERT_EQ(0, sw->get_entry(out, table_name, sql_values, sql_filter));

        //check result
        ASSERT_EQ(src_num, out_num);
        ASSERT_EQ(0, memcmp(src_data.data(), out_buf.data(), src_data.size()));
    }
}

TEST_F(TestSqliteWrapper, test_insert_update_ok)
{
    ASSERT_TRUE(sw != nullptr);
    ASSERT_TRUE(sw->is_ok());

    std::string table_name = "dummy_1";
    int src_num = 123456;
    int old_src_num = src_num;
    std::string src_str = "hello world";

    //create table
    {
        std::string sql_str = "num1 INT, str1 TEXT";

        ASSERT_EQ(0,sw->create_table(table_name, sql_str));
    }
    //expect could not be queried, as there is no entry yet
    {
        std::string sql_str = "num1 = " + std::to_string(src_num);
        ASSERT_FALSE(sw->peek_entry(table_name, sql_str));
    }
    //insert a simple entry
    {

        std::string sql_str_insert = "(num1, str1) VALUES (" +
            std::to_string(src_num) + ", \"" + src_str + "\")";
        std::string sql_str_update = "num1 = " + std::to_string(src_num) +
            ", str1 = \"" + src_str + "\"";
        std::string sql_str_filter = "WHERE num1 = " + std::to_string(src_num);
        ASSERT_EQ(0,sw->insert_update_entry(table_name, sql_str_insert,
                    sql_str_update, sql_str_filter));
    }
    //expect could be queried
    {
        std::string sql_str = "WHERE num1 = " + std::to_string(src_num);
        ASSERT_TRUE(sw->peek_entry(table_name, sql_str));
    }
    {
        src_num = 654321;
        std::string sql_str_insert = "(num1, str1) VALUES (" +
            std::to_string(src_num) + ", \"" + src_str + "\")";
        std::string sql_str_update = "num1 = " + std::to_string(src_num) +
            ", str1 = \"" + src_str + "\"";
        std::string sql_str_filter = "WHERE num1 = " + std::to_string(old_src_num);
        ASSERT_EQ(0,sw->insert_update_entry(table_name, sql_str_insert,
                    sql_str_update, sql_str_filter));
    }
    //expect could be queried, with modified value
    {
        std::string sql_str = "WHERE num1 = " + std::to_string(src_num);
        ASSERT_TRUE(sw->peek_entry(table_name, sql_str));
    }
    //expect could not be queried, with old value
    {
        std::string sql_str = "WHERE num1 = " + std::to_string(old_src_num);
        ASSERT_FALSE(sw->peek_entry(table_name, sql_str));
    }
}

TEST_F(TestSqliteWrapper, test_get_int64)
{
    ASSERT_TRUE(sw != nullptr);
    ASSERT_TRUE(sw->is_ok());

    std::string table_name = "dummy_1";
    int src_num = 123456;
    int64_t dummy_num = 1565578818000; //a dummy long int

    //create table
    {
        std::string sql_str = "num1 INT, num2 INT";

        ASSERT_EQ(0,sw->create_table(table_name, sql_str));
    }
    //insert
    {
        std::string sql_str = "(num1, num2) VALUES (" + std::to_string(src_num) + ", " +
            std::to_string(dummy_num) +
            ")";
        ASSERT_EQ(0,sw->insert_entry(table_name, sql_str));
    }
    //get
    {
        std::string sql_values = "num2";
        std::string sql_filter = "WHERE num1 = " + std::to_string(src_num);
        int64_t out_dummy_num = 0;

        std::vector<SqliteWrapper::GetItem> out = {
            SqliteWrapper::GetItem(&out_dummy_num, sizeof(out_dummy_num)),
        };

        ASSERT_EQ(0, sw->get_entry(out, table_name, sql_values, sql_filter));

        //check result
        ASSERT_EQ(dummy_num, out_dummy_num);
    }
}

TEST_F(TestSqliteWrapper, test_get_blob_dynamic_alloc)
{
    ASSERT_TRUE(sw != nullptr);
    ASSERT_TRUE(sw->is_ok());

    std::string table_name = "dummy_1";
    int src_num = 123456;
    std::string src_str = "hello world";
    std::vector<uint8_t> src_data = {65, 66, 67, 68, 69, 70};

    //create table
    {
        std::string sql_str = "num1 INT, str1 TEXT, data1 BLOB";

        ASSERT_EQ(0,sw->create_table(table_name, sql_str));
    }
    //insert
    {
        std::string sql_str = "(num1, str1, data1) VALUES (" + std::to_string(src_num) + ", \"" + src_str +
            "\", @_p1)"; //place holder variable as _p1
        std::map<const std::string, std::vector<uint8_t>*> blobs = {
            std::make_pair("@_p1", &src_data)   //should have the same place
        };
        ASSERT_EQ(0,sw->insert_entry(table_name, sql_str, &blobs));
    }
    //get
    {
        std::string sql_values = "num1, data1";
        std::string sql_filter = "WHERE num1 = " + std::to_string(src_num);
        int out_num;
        std::vector<uint8_t> out_buf;
        auto copy = [&out_buf](const void* src, uint32_t size) {
            out_buf.resize(size);
            memcpy(out_buf.data(), src, size);
            return 0;
        };

        std::vector<SqliteWrapper::GetItem> out = {
            SqliteWrapper::GetItem(&out_num, 0),
            SqliteWrapper::GetItem(nullptr, 0, copy)
        };

        ASSERT_EQ(0, sw->get_entry(out, table_name, sql_values, sql_filter));

        //check result
        ASSERT_EQ(src_num, out_num);
        ASSERT_EQ(0, memcmp(src_data.data(), out_buf.data(), src_data.size()));
    }
}
//place holder
/*
TEST_F(TestSqliteWrapper, test_dummy)
{
    ASSERT_TRUE(sw != nullptr);
    ASSERT_TRUE(sw->is_ok());

    std::string table_name = "dummy_1";

    //create table
    {
        std::string sql_str = "num1 INT, num2 INT, num3 INT";

        ASSERT_EQ(0,sw->create_table(table_name, sql_str));
    }
    //try update empty entry
    {
        std::string sql_str = "num3 = 99";
        std::string sql_filter = "num3 > 1";
        ASSERT_EQ(0,sw->update_entry(table_name, sql_str, sql_filter));
    }
}
*/
