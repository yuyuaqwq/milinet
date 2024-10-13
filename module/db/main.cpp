#include <iostream>

#include "million/imillion.h"
#include "million/imsg.h"

#include <yaml-cpp/yaml.h>

#include <sw/redis++/redis++.h>

#include <soci/soci.h>
#include <soci/mysql/soci-mysql.h>

using namespace sw::redis;

// �������
class CacheService : public million::IService {
public:
    using Base = million::IService;
    using Base::Base;

    //bool Connect() {
    //    Close();
    //    connection_ = redisAsyncConnect("127.0.0.1", 6379);
    //    if (connection_ == nullptr || connection_->err) {
    //        if (connection_) {
    //            std::cerr << "Error: " << connection_->errstr << std::endl;
    //        }
    //        else {
    //            std::cerr << "Can't allocate redis context" << std::endl;
    //        }
    //        return false;
    //    }
    //    return true;
    //}

    //void Close() {
    //    if (connection_) {
    //        redisAsyncFree(connection_);
    //        connection_ = nullptr;;
    //    }
    //}

    virtual void OnInit() override {
        try {
            // ���� Redis �������ӵ� Redis ������
            auto redis = Redis("tcp://127.0.0.1:6379");

            // ���ü�ֵ��
            redis.set("my_key", "Hello, Redis!");

            // ��ȡ����ֵ
            auto val = redis.get("my_key");
            if (val) {
                // ��������ڣ���ӡ����ֵ
                std::cout << "my_key: " << *val << std::endl;
            }
            else {
                std::cout << "my_key does not exist" << std::endl;
            }

            // ��������ֵ
            redis.incr("my_counter");

            // ��ȡ�������ֵ
            auto counter_val = redis.get("my_counter");
            if (counter_val) {
                std::cout << "my_counter: " << *counter_val << std::endl;
            }
        }
        catch (const Error& err) {
            std::cerr << "Redis error: " << err.what() << std::endl;
        }

    }

    virtual million::Task OnMsg(million::MsgUnique msg) override {
        co_return;
    }

    virtual void OnExit() override {
        // Close();
    }



private:
};

// SQL����
class SqlService : public million::IService {
public:
    using Base = million::IService;
    using Base::Base;

    virtual void OnInit() override {
        try
        {
            // ������ MySQL ���ݿ������
            soci::session sql(soci::mysql, "db=mydb user=myuser password=mypassword host=localhost");

            // ����һ���򵥵ı�
            sql << "CREATE TABLE IF NOT EXISTS users ("
                "id INT AUTO_INCREMENT PRIMARY KEY, "
                "name VARCHAR(100), "
                "age INT)";

            // ��������
            std::string name = "Alice";
            int age = 30;
            sql << "INSERT INTO users (name, age) VALUES (:name, :age)", soci::use(name), soci::use(age);

            // ��ѯ����
            soci::rowset<soci::row> rs = (sql.prepare << "SELECT id, name, age FROM users");
            for (auto it = rs.begin(); it != rs.end(); ++it)
            {
                const soci::row& row = *it;
                std::cout << "ID: " << row.get<int>(0) << ", Name: " << row.get<std::string>(1) << ", Age: " << row.get<int>(2) << std::endl;
            }
        }
        catch (const soci::mysql_soci_error& e)
        {
            std::cerr << "MySQL error: " << e.what() << std::endl;
        }
        catch (const std::exception& e)
        {
            std::cerr << "Error: " << e.what() << std::endl;
        }
    }

    virtual million::Task OnMsg(million::MsgUnique msg) override {
        co_return;
    }

    virtual void OnExit() override {
        // Close();
    }



private:

};

MILLION_FUNC_EXPORT bool MillionModuleInit(million::IMillion* imillion) {
    auto& config = imillion->config();
    auto cache_service_handle = imillion->NewService<CacheService>();
    auto sql_service_handle = imillion->NewService<SqlService>();

    return true;
}
