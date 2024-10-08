#pragma once

#include <cstdint>

#include <memory>

#include "milinet/imilinet.hpp"

namespace milinet {

class ConfigException : public std::runtime_error {
public:
    explicit ConfigException(const std::string& message)
        : std::runtime_error("YAML config error: " + message) {}
};

class ServiceMgr;
class MsgMgr;
class ModuleMgr;
class WorkerMgr;
class MILINET_CLASS_EXPORT Milinet : public IMilinet {
public:
    Milinet(std::string_view config_path);
    ~Milinet();

    void Init();
    void Start();

    virtual ServiceHandle CreateService(std::unique_ptr<IService> iservice) override;
    using IMilinet::CreateService;

    virtual SessionId Send(ServiceHandle target, MsgUnique msg) override;
    using IMilinet::Send;

    auto& service_mgr() { assert(service_mgr_); return *service_mgr_; }
    auto& msg_mgr() { assert(service_mgr_); return *msg_mgr_; }
    auto& module_mgr() { assert(module_mgr_); return *module_mgr_; }
    auto& worker_mgr() { assert(worker_mgr_); return *worker_mgr_; }

private:
    std::unique_ptr<ServiceMgr> service_mgr_;
    std::unique_ptr<MsgMgr> msg_mgr_;
    std::unique_ptr<ModuleMgr> module_mgr_;
    std::unique_ptr<WorkerMgr> worker_mgr_;
};

} // namespace milinet