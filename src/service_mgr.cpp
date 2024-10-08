#include "milinet/service_mgr.h"

#include "milinet/service.h"

namespace milinet {

ServiceMgr::ServiceMgr(Milinet* milinet) 
    : milinet_(milinet) {}

ServiceMgr::~ServiceMgr() = default;

ServiceHandle ServiceMgr::AddService(std::unique_ptr<IService> iservice) {
    decltype(services_)::iterator iter;
    auto service = std::make_unique<Service>(std::move(iservice));
    {
        std::lock_guard guard(services_mutex_);
        services_.emplace_back(std::move(service));
        iter = --services_.end();
    }
    auto handle = ServiceHandle(iter);
    handle.service_ptr()->iservice().set_service_handle(handle);
    return handle;
}

void ServiceMgr::RemoveService(ServiceHandle handle) {
    {
        std::lock_guard guard(services_mutex_);
        services_.erase(handle.iter());
    }
}

void ServiceMgr::PushService(Service* service) {
    bool has_push = false;
    {
        std::lock_guard guard(service_queue_mutex_);
        if (!service->in_queue()){
            service_queue_.emplace(service);
            service->set_in_queue(true);
            has_push = true;
        }
    }
    if (has_push) {
        service_queue_cv_.notify_one();
    }
}

Service& ServiceMgr::PopService() {
    std::unique_lock guard(service_queue_mutex_);
    while (service_queue_.empty()) {
        service_queue_cv_.wait(guard);
    }
    // std::cout << "wake up" << std::endl;
    auto* service = service_queue_.front();
    assert(service);
    service_queue_.pop();
    service->set_in_queue(false);
    return *service;
}

SessionId ServiceMgr::Send(ServiceHandle target, MsgUnique msg) {
    auto id = msg->session_id();
    auto service = target.service_ptr();
    service->PushMsg(std::move(msg));
    PushService(service);
    return id;
}

} // namespace milinet