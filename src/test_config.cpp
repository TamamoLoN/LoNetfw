#include "config/config.h"
#include "yaml-cpp/yaml.h"

class Person
{
  public:
    Person(const std::string name, const int age, const std::string gender = "default",
           float score = 0.0f)
        : m_name(name), m_age(age), m_gender(gender), m_score(score)
    {
    }
    std::string print() const
    {
        std::stringstream ss;
        ss << "[Person: " << m_name << ", " << m_age << ", " << m_gender << ", " << m_score << "]";
        return ss.str();
    }
    std::string m_name;
    int m_age;
    std::string m_gender;
    float m_score;
};

template <> class lon::util::LexicalCast<Person, std::string>
{
  public:
    Person operator()(const std::string &source) const
    {
        YAML::Node node = YAML::Load(source);
        return Person(node["name"].as<std::string>(), node["age"].as<int>(),
                      node["gender"].as<std::string>(), node["score"].as<float>());
    }
};

template <> class lon::util::LexicalCast<std::string, Person>
{
  public:
    std::string operator()(const Person &source) const
    {
        YAML::Node node;
        node["name"]   = source.m_name;
        node["age"]    = source.m_age;
        node["gender"] = source.m_gender;
        node["score"]  = source.m_score;
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

int main(int argc, char const *argv[])
{
    auto config = std::make_shared<lon::config::Config>();
    lon::config::ConfigData<int>::Ptr data1 =
        std::make_shared<lon::config::ConfigData<int>>("test.port", 123, "testtest");

    lon::config::ConfigData<float>::Ptr data2 =
        std::make_shared<lon::config::ConfigData<float>>("test.value", 3.1415f, "testtest");

    auto data3 =
        lon::config::Config::setData("test.vec", std::vector<int>({1, 2, 3, 4}), "test.vec");

    auto data4 =
        lon::config::Config::setData("test.list", std::list<int>({1, 2, 3, 4}), "test.list");

    auto data5 = lon::config::Config::setData("test.set", std::set<int>({1, 2, 3, 4}), "test.set");
    auto data6 = lon::config::Config::setData("test.uset", std::unordered_set<int>({1, 2, 3, 4}),
                                              "test.uset");

    auto data7 = lon::config::Config::setData(
        "test.map", std::map<std::string, int>({{"one", 1}, {"two", 2}}), "test.map");

    auto data8 = lon::config::Config::setData(
        "test.umap", std::unordered_map<std::string, int>({{"one", 1}, {"two", 2}}), "test.umap");

    auto data9 = lon::config::Config::setData("test.umap", 1.123, "test.umap");

    auto data10 = std::make_shared<lon::config::ConfigData<int>>("test.value", 114, "testtest");

    auto data11 =
        lon::config::Config::setData("class.person", Person("yqh", 22, "male", 100.0f), "yqh");

    auto data12 = lon::config::Config::setData(
        "class.map", std::map<std::string, Person>({{"mmd", Person("yqh", 22, "male", 100.0f)}}),
        "yqh");

    auto data13 = lon::config::Config::setData("test.bool", false, "test.bool");

    auto data14 = lon::config::Config::setData(
        "class.map_vec",
        std::map<std::string, std::vector<Person>>({{"mmd", {Person("yqh", 22, "male", 100.0f)}}}),
        "yqh");

    config->setData(data1);
    config->setData("test.value", 3.1415f, "testtest");
    config->setData(data3);
    config->setData(data10);

    // auto vec = lon::config::Config::getData<std::vector<int>>("test.vec")->getData();
    // for (auto &v : vec)
    // {
    //     LON_INFO(LON_LOG_ROOT) << v;
    // }

    auto res = config->getData<int>("test");
    if (res != nullptr)
    {
        LON_INFO(LON_LOG_ROOT) << res->toString();
    }

    YAML::Node root = YAML::LoadFile("/home/idriver/yqh/git/LoNetfw/.config/log.yaml");
    auto log_node   = root["logs"];
    // for (const auto &node : log_node)
    // {
    //     LON_DEBUG(LON_LOG_ROOT) << node["name"].as<std::string>();
    // }
    // LON_FATAL(LON_LOG_ROOT) << root;
    // lon::util::printYamlString(root);

    LON_INFO(LON_LOG_ROOT) << "before: " << data1->getData();
    LON_INFO(LON_LOG_ROOT) << "before: " << data2->toString();
    LON_INFO(LON_LOG_ROOT) << "before: " << data3->toString();
    LON_INFO(LON_LOG_ROOT) << "before: " << data4->toString();
    LON_INFO(LON_LOG_ROOT) << "before: " << data5->toString();
    LON_INFO(LON_LOG_ROOT) << "before: " << data6->toString();
    LON_INFO(LON_LOG_ROOT) << "before: " << data7->toString();
    LON_INFO(LON_LOG_ROOT) << "before: " << data8->toString();
    LON_INFO(LON_LOG_ROOT) << "before: " << data11->toString();
    LON_INFO(LON_LOG_ROOT) << "before: " << data12->toString();
    LON_INFO(LON_LOG_ROOT) << "before: " << data13->toString();
    LON_INFO(LON_LOG_ROOT) << "before: " << data14->toString();
    // config->parseFromYaml("/home/idriver/yqh/git/LoNetfw/.config/log.yaml");
    lon::config::Config::parseFromYaml(root);

    LON_INFO(LON_LOG_ROOT) << "after: " << data1->getData();
    LON_INFO(LON_LOG_ROOT) << "after: " << data2->toString();
    LON_INFO(LON_LOG_ROOT) << "after: " << data3->toString();
    LON_INFO(LON_LOG_ROOT) << "after: " << data4->toString();
    LON_INFO(LON_LOG_ROOT) << "after: " << data5->toString();
    LON_INFO(LON_LOG_ROOT) << "after: " << data6->toString();
    LON_INFO(LON_LOG_ROOT) << "after: " << data7->toString();
    LON_INFO(LON_LOG_ROOT) << "after: " << data8->toString();
    LON_INFO(LON_LOG_ROOT) << "after: " << data11->toString();
    LON_INFO(LON_LOG_ROOT) << "after: " << data12->toString();
    LON_INFO(LON_LOG_ROOT) << "after: " << data13->toString();
    LON_INFO(LON_LOG_ROOT) << "after: " << data14->toString();

    return 0;
}
