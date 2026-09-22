#include "symbol_table.h"

void SymbolTable::insert(const std::string& name, const std::string& type, int line) {
    table_.push_back(SymbolEntry{name, type, line, line});
}

const SymbolEntry* SymbolTable::lookup(const std::string& name) const {
    for (const SymbolEntry& entry : table_) {
        if (entry.name == name) return &entry;
    }
    return nullptr;
}

SymbolEntry* SymbolTable::find(const std::string& name) {
    for (SymbolEntry& entry : table_) {
        if (entry.name == name) return &entry;
    }
    return nullptr;
}

void SymbolTable::recordReassignment(const std::string& name, int line) {
    SymbolEntry* entry = find(name);
    if (entry != nullptr) {
        entry->lastAssignLine = line;
    }
}

const std::vector<SymbolEntry>& SymbolTable::entries() const {
    return table_;
}
