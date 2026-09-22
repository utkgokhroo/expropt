#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include <string>
#include <vector>

// One entry per distinct identifier that has been assigned to at least once.
struct SymbolEntry {
    std::string name;
    std::string type;        // "int" or "float" -- fixed after the first assignment
    int firstAssignLine;
    int lastAssignLine;
};

// A flat (single-scope) symbol table. ExprOpt has no blocks, functions, or
// nested scopes, so one flat list of entries -- indexed by name -- is
// enough; there is nothing to push/pop.
class SymbolTable {
public:
    // Adds a new entry for an identifier seen for the first time.
    // Caller must check lookup(name) == nullptr first.
    void insert(const std::string& name, const std::string& type, int line);

    // Returns the entry for 'name', or nullptr if it has never been assigned.
    const SymbolEntry* lookup(const std::string& name) const;

    // Updates lastAssignLine for an identifier that already exists.
    // Caller must check lookup(name) != nullptr first. The stored type is
    // never changed here -- a variable's type is fixed at first assignment.
    void recordReassignment(const std::string& name, int line);

    // All entries, in first-seen order (handy for printing).
    const std::vector<SymbolEntry>& entries() const;

private:
    std::vector<SymbolEntry> table_;

    SymbolEntry* find(const std::string& name);
};

#endif // SYMBOL_TABLE_H
