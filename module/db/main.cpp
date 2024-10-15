#include <iostream>

#include "million/imillion.h"
#include "million/imsg.h"

#include <yaml-cpp/yaml.h>

#include <sw/redis++/redis++.h>

#include <soci/soci.h>
#include <soci/mysql/soci-mysql.h>


#include <iostream>
#include <fstream>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/message.h>
#include <google/protobuf/compiler/importer.h>
#include "proto/message.pb.h"

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


    void RegisterDbProto(google::protobuf::Message msg) {
        const google::protobuf::Descriptor* const descriptor = msg.GetDescriptor();
        const google::protobuf::Reflection* const reflection = msg.GetReflection();


    }

    void QueryDbObj() {

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
            connection_ = soci::session(soci::mysql, "db=pokemon_server user=root password=You_Yu666 host=121.37.17.176");

            // ����һ���򵥵ı�
            connection_ << "CREATE TABLE IF NOT EXISTS users ("
                "id INT AUTO_INCREMENT PRIMARY KEY, "
                "name VARCHAR(100), "
                "age INT)";

            // ��������
            std::string name = "Alice";
            int age = 30;
            connection_ << "INSERT INTO users (name, age) VALUES (:name, :age)", soci::use(name), soci::use(age);

            // ��ѯ����
            soci::rowset<soci::row> rs = (connection_.prepare << "SELECT id, name, age FROM users");
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


    template <typename ProtoMsgT>
    ProtoMsgT QueryDbObj(std::string_view table) {
        ProtoMsgT msg;

        const google::protobuf::Descriptor* descriptor = msg.GetDescriptor();
        const google::protobuf::Reflection* reflection = msg.GetReflection();

        for (int i = 0; i < descriptor->field_count(); ++i) {

        }

        return msg;
    }

private:
    soci::session connection_;
};


void PrintMessageFields(const google::protobuf::Message& message) {
    // ��ȡ��Ϣ����������Descriptor�����������������ֶε�Ԫ����
    const google::protobuf::Descriptor* descriptor = message.GetDescriptor();
    const google::protobuf::Reflection* reflection = message.GetReflection();

    for (int i = 0; i < descriptor->field_count(); ++i) {
        // ��ȡÿ���ֶε�������
        const google::protobuf::FieldDescriptor* field = descriptor->field(i);

        auto type = field->type();

        // �����ֶ����Ͷ�̬��ȡ�ֶ�ֵ
        if (field->is_repeated()) {
            int field_size = reflection->FieldSize(message, field);
            std::cout << "Repeated Field: " << field->name() << " (size=" << field_size << ")" << std::endl;

            for (int j = 0; j < field_size; ++j) {
                if (field->type() == google::protobuf::FieldDescriptor::TYPE_STRING) {
                    std::cout << "  Value: " << reflection->GetRepeatedString(message, field, j) << std::endl;
                }
            }
        }
        else {
            if (field->type() == google::protobuf::FieldDescriptor::TYPE_INT32) {
                int32_t value = reflection->GetInt32(message, field);
                std::cout << "Field: " << field->name() << " (int32): " << value << std::endl;
            }
            else if (field->type() == google::protobuf::FieldDescriptor::TYPE_STRING) {
                std::string value = reflection->GetString(message, field);
                std::cout << "Field: " << field->name() << " (string): " << value << std::endl;
            }
        }
    }
}


class ErrorCollector : public google::protobuf::compiler::MultiFileErrorCollector {
public:
    void AddError(const std::string& filename, int line, int column, const std::string& message) override {
        std::cerr << "Error in " << filename << ":" << line << ":" << column << " - " << message << std::endl;
    }
};

int parseProtoFile()
{
    // ׼�����ú��ļ�ϵͳ
    google::protobuf::compiler::DiskSourceTree sourceTree;
    // ����ǰ·��ӳ��Ϊ��Ŀ��Ŀ¼ �� project_root �����Ǹ����֣����������Ҫ�ĺϷ�����.
    // sourceTree.MapPath("data", ".");
    sourceTree.MapPath("", "C:/Users/yuyu/Desktop/projects/milinet/module/db/proto/");
    // ���ö�̬������.
    google::protobuf::compiler::Importer importer(&sourceTree, new ErrorCollector());
    // ��̬����protoԴ�ļ��� Դ�ļ���./source/proto/test.proto .
    auto fileDescriptor = importer.Import("message.proto");


    std::cout << fileDescriptor->message_type_count() << std::endl;
    for (auto i = 0; i < fileDescriptor->message_type_count(); i++)
    {
        auto descriptor = fileDescriptor->message_type(i);

        std::cout << descriptor->name() << " " << descriptor->field_count() << " " << descriptor->nested_type_count() << std::endl;

        auto descriptor1 = descriptor->containing_type();

        if (descriptor1)
        {
            std::cout << descriptor1->name() << std::endl;
        }
    }

    std::cout << fileDescriptor->name() << std::endl;


    auto descriptor = fileDescriptor->message_type(1);
    for (auto i = 0; i < descriptor->field_count(); i++)
    {
        auto fieldDes = descriptor->field(i);
        google::protobuf::SourceLocation outLocation;
        if (fieldDes->GetSourceLocation(&outLocation))
        {
            printf("%s: %d %d %d %d\nleading_comments:%s\ntrailing_comments:%s\n",
                fieldDes->full_name().c_str(),
                outLocation.start_line, outLocation.start_column, outLocation.end_line, outLocation.end_column,
                outLocation.leading_comments.c_str(), outLocation.trailing_comments.c_str());
            for (auto comment : outLocation.leading_detached_comments)
            {
                printf("leading_detached_comments:%s\n", comment.c_str());
            }
        }
        else
        {
            std::cout << "fail" << std::endl;
        }
    }

#if 0
    // ���ڿ��Դӱ���������ȡ���͵�������Ϣ.
    auto descriptor1 = importer.pool()->FindMessageTypeByName("T.Test.InMsg");

    // ����һ����̬����Ϣ����.
    google::protobuf::DynamicMessageFactory factory;
    // ����Ϣ�����д�����һ������ԭ��.
    auto proto1 = factory.GetPrototype(descriptor1);
    // ����һ�����õ���Ϣ.
    auto message1 = proto1->New();
    // ������ͨ������ӿڸ��ֶθ�ֵ.
    auto reflection1 = message1->GetReflection();
    auto filed1 = descriptor1->FindFieldByName("id");
    reflection1->SetUInt32(message1, filed1, 1);

    // ��ӡ����
    std::cout << message1->DebugString() << std::endl;

    std::string output;
    google::protobuf::util::MessageToJsonString(*message1, &output);
    std::cout << output << std::endl;

    // ɾ����Ϣ.
    delete message1;
#endif
    return 0;
}

MILLION_FUNC_EXPORT bool MillionModuleInit(million::IMillion* imillion) {


    // ͨ��RegisterDBMsg��������SQLӳ��


    // ��ʼ�� Protobuf ��
    GOOGLE_PROTOBUF_VERIFY_VERSION;

    parseProtoFile();

    // �Ӷ������ļ��ж�ȡ MyMessage
    MyMessage message;
    message.set_id(100);
    message.set_name("222");
    message.add_tags("fake");
    message.add_tags("sd");

    // message.ParseFrom();


    // ʹ�÷��䶯̬��ӡ��Ϣ���ֶ�
    PrintMessageFields(message);

    // ���� Protobuf ��Դ
    google::protobuf::ShutdownProtobufLibrary();


    auto& config = imillion->config();
    auto cache_service_handle = imillion->NewService<CacheService>();
    auto sql_service_handle = imillion->NewService<SqlService>();

    return true;
}
