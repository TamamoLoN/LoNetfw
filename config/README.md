# LoNetfw 配置系统架构设计

## 1. 系统概述
配置系统提供灵活的类型安全配置管理，支持：
- 多种数据类型（基础类型/STL容器/自定义类型）
- YAML配置文件解析
- 动态配置更新
- 配置变更回调

## 2. 核心组件

### 2.1 Config (config.h/config.cpp)
- 配置管理核心类
- 提供配置项的注册、访问接口
- 管理配置数据集合

### 2.2 ConfigData (configdata.h/configdata.cpp)
- 配置数据模板类
- 封装配置值、名称和描述
- 支持类型安全的配置访问

### 2.3 ConfigInitter (configinit.h)
- 配置初始化工具类
- 提供配置数据的便捷注册方式

## 3. 功能特性

### 3.1 支持的数据类型
```cpp
// 基础类型
Config::setData("test.port", 8080, "服务端口");

// STL容器
Config::setData("test.vec", std::vector<int>{1,2,3}, "测试向量");

// 自定义类型（需特化LexicalCast）
Config::setData("person", Person("Alice",25), "用户信息");
```

### 3.2 自定义类型支持
```cpp
template <>
class LexicalCast<Person, std::string> {
    Person operator()(const std::string& v) {
        YAML::Node node = YAML::Load(v);
        return Person(
            node["name"].as<std::string>(),
            node["age"].as<int>()
        );
    }
};
```

### 3.3 配置变更监听
```cpp
auto data = Config::setData("threshold", 0.5f, "敏感度阈值");
data->addConfigDataChangeCB([](float oldVal, float newVal){
    // 处理配置变更
});
```

## 4. 使用示例

### 4.1 基础使用
```cpp
// 设置配置
Config::setData("server.port", 8080, "服务端口");

// 获取配置
int port = Config::getData<int>("server.port")->getData();
```

### 4.2 从YAML加载
```cpp
// 加载配置文件
Config::parseFromYaml("config.yaml");

// 访问配置
auto dbConfig = Config::getData<std::string>("database.url");
```

## 5. 设计优势
- 类型安全：编译期类型检查
- 扩展性强：支持自定义类型
- 线程安全：内置同步机制
- 高效：基于模板的零成本抽象