#include <brex/expand.h>

#include <cassert>
#include <utility>  // move

namespace brex {

// class Expander
// --------------

Expander::~Expander() {
}

std::ostream& operator<<(std::ostream& stream, const Expander& expander) {
    expander.printCurrent(stream);
    return stream;
}

// class String
// ------------

String::String(const std::string& value)
: value(value) {
}

AdvanceResult String::advance() {
    return AdvanceResult::CARRY;
}

void String::printCurrent(std::ostream& stream) const {
    stream << value;
}

// class Sequence
// --------------

void Sequence::appendChild(std::unique_ptr<Expander> child) {
    children.push_back(std::move(child));
}

AdvanceResult Sequence::advance() {
    auto iter = children.rbegin();

    if (iter == children.rend()) {
        // An empty sequence always has its initial value.
        return AdvanceResult::CARRY;
    }

    AdvanceResult result;

    do {
        assert(*iter);  // each `unique_ptr<Expander>` is never null
        Expander& child = **iter;

        result = child.advance();
        ++iter;
    } while (result == AdvanceResult::CARRY && iter != children.rend());

    return result;
}

void Sequence::printCurrent(std::ostream& stream) const {
    for (const auto& child : children) {
        assert(child);
        child->printCurrent(stream);
    }
}

// class Alternation
// -----------------

Expander& Alternation::currentChild() const {
    assert(currentIndex >= 0);

    const auto& childPointer = children[currentIndex];
    assert(childPointer);  // none of the `unique_ptr<Expander>` is null

    return *childPointer;
}

Alternation::Alternation()
// `-1` is the special value meaning "there are no children."
: currentIndex(-1) {
}

void Alternation::appendChild(std::unique_ptr<Expander> child) {
    children.push_back(std::move(child));

    if (currentIndex == -1) {
        currentIndex = 0;
    }
}

AdvanceResult Alternation::advance() {
    if (currentIndex == -1) {
        // Empty alternations are against the specification, but that's handled
        // in the parser.  `class Alternation` tolerates having no children.
        // An alternation with no children is always in its initial (empty)
        // state, so `advance()` always returns `AdvanceResult::CARRY`.
        return AdvanceResult::CARRY;
    }

    if (currentChild().advance() == AdvanceResult::NO_CARRY) {
        return AdvanceResult::NO_CARRY;
    }

    // Advancing the current child carried over, so we either need to go to the
    // next child, or if we're already at the last child, go back to the first
    // and carry.
    if (++currentIndex == int(children.size())) {
        currentIndex = 0;
        return AdvanceResult::CARRY;
    }
    else {
        return AdvanceResult::NO_CARRY;  // next child, no carry over
    }
}

void Alternation::printCurrent(std::ostream& stream) const {
    if (currentIndex == -1) {
        // Empty alternations are against the specification, but that's handled
        // in the parser.  `class Alternation` tolerates having no children.
        return;
    }

    currentChild().printCurrent(stream);
}

// free functions
// --------------

std::unique_ptr<Expander> expander(const ParseTreeNode& root) {
    // TODO
    (void)root;
    return nullptr;  // TODO: stub
}

void expand(std::ostream&      stream,
            Expander&          expander,
            const std::string& separator) {
    stream << expander;

    while (expander.advance() == AdvanceResult::NO_CARRY) {
        stream << separator << expander;
    }
}

}  // namespace brex