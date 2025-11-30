#ifndef MEMBERCONTAINER_H
#define MEMBERCONTAINER_H

#include "librarymember.h"
#include <vector>
#include <memory>
#include <algorithm>
#include "exceptions.h"
#include <iterator>

class MemberContainer {
private:
    std::vector<std::unique_ptr<LibraryMember>> members;
    int nextId = 1;

public:
    MemberContainer() = default;

    class Iterator {
    private:
        std::vector<std::unique_ptr<LibraryMember>>::iterator it;

    public:
        explicit Iterator(std::vector<std::unique_ptr<LibraryMember>>::iterator pIt) : it(pIt) {}
        
        Iterator& operator++() { ++it; return *this; }
        Iterator operator++(int) { Iterator tmp = *this; ++(*this); return tmp; }
        
        friend bool operator==(const Iterator& lhs, const Iterator& rhs) { return lhs.it == rhs.it; }
        friend bool operator!=(const Iterator& lhs, const Iterator& rhs) { return lhs.it != rhs.it; }
        
        LibraryMember& operator*() { return **it; }
        LibraryMember* operator->() { return it->get(); }
    };

    Iterator begin() { return Iterator(members.begin()); }
    Iterator end() { return Iterator(members.end()); }

    int generateId() { return nextId++; }
    void setNextId(int id) { nextId = id; }
    int getNextId() const { return nextId; }

    void addMember(std::unique_ptr<LibraryMember> member);
    void removeMember(int id);
    LibraryMember* findMember(int id) const;
    std::vector<LibraryMember*> getAllMembers() const;
    std::vector<LibraryMember*> getBlockedMembers() const;
    size_t size() const { return members.size(); }
    bool empty() const { return members.empty(); }
    
    // STL алгоритмы
    template<typename Predicate>
    std::vector<LibraryMember*> findMembers(Predicate pred) const {
        std::vector<LibraryMember*> result;
        std::copy_if(members.begin(), members.end(), std::back_inserter(result),
                    [&pred](const std::unique_ptr<LibraryMember>& member) {
                        return pred(member.get());
                    });
        return result;
    }
};

#endif // MEMBERCONTAINER_H

