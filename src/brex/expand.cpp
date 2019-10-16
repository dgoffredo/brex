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

bool String::advance() {
    return false;
}

void String::printCurrent(std::ostream& stream) const {
    stream << value;
}

// class Sequence
// --------------

void Sequence::appendChild(std::unique_ptr<Expander> child) {
    children.push_back(std::move(child));
}

bool Sequence::advance() {
    auto iter = children.rbegin();

    if (iter == children.rend()) {
        return false;  // an empty sequence always returns to its initial value
    }

    // `carry` is whether to carry on incrementing the next most significant
    // child.  It's a little confusing, since `advance()` returns `false` when
    // "yes, carry is true," so there's a double negation going on here.
    // Watch out.
    bool carry;

    do {
        assert(*iter);  // each `unique_ptr<Expander>` is never null
        Expander& child = **iter;

        carry = !child.advance();
        ++iter;
    } while (carry && iter != children.rend());

    return !carry;
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

bool Alternation::advance() {
    if (currentIndex == -1) {
        // Empty alternations are against the specification, but that's handled
        // in the parser.  `class Alternation` tolerates having no children.
        // An alternation with no children is always in its initial (empty)
        // state, so `advance()` always returns `false`.
        return false;
    }

    if (currentChild().advance()) {
        // No carry over, so just return `true`.
        return true;
    }

    // Advancing the current child carried over, so we either need to go to the
    // next child, or if we're already at the last child, go back to the first
    // and carry.
    if (++currentIndex == int(children.size())) {
        currentIndex = 0;
        return false;  // carry over
    }
    else {
        return true;  // next child, no carry over
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

    while (expander.advance()) {
        stream << separator << expander;
    }
}

}  // namespace brex