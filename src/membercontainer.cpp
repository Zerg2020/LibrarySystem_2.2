#include "membercontainer.h"
#include "exceptions.h"

void MemberContainer::addMember(std::unique_ptr<LibraryMember> member) {
    if (findMember(member->getId()) != nullptr) {
        throw DuplicateException("Абонент с ID " + std::to_string(member->getId()) + " уже существует");
    }
    members.push_back(std::move(member));
}

void MemberContainer::removeMember(int id) {
    auto it = std::find_if(members.begin(), members.end(),
                          [id](const std::unique_ptr<LibraryMember>& member) {
                              return member->getId() == id;
                          });
    if (it != members.end()) {
        members.erase(it);
    } else {
        throw NotFoundException("Абонент с ID " + std::to_string(id));
    }
}

LibraryMember* MemberContainer::findMember(int id) const {
    auto it = std::find_if(members.begin(), members.end(),
                          [id](const std::unique_ptr<LibraryMember>& member) {
                              return member->getId() == id;
                          });
    return (it != members.end()) ? it->get() : nullptr;
}

std::vector<LibraryMember*> MemberContainer::getAllMembers() const {
    std::vector<LibraryMember*> result;
    for (const auto& member : members) {
        result.push_back(member.get());
    }
    return result;
}

std::vector<LibraryMember*> MemberContainer::getBlockedMembers() const {
    std::vector<LibraryMember*> result;
    for (const auto& member : members) {
        if (member->getIsBlocked()) {
            result.push_back(member.get());
        }
    }
    return result;
}

