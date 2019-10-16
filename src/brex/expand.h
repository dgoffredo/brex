#ifndef INCLUDED_BREX_EXPAND
#define INCLUDED_BREX_EXPAND

#include <iosfwd>  // ostream&
#include <memory>  // unique_ptr
#include <string>
#include <vector>

namespace brex {

class ParseTreeNode;

// `Expander` is the abstact base class of `String`, `Sequence`, and
// `Alternation`.  An `Expander` is a stateful emitter of values expanded from
// a parsed shell brace expression.  Its interface is that of a cursor that
// iterates over all values implied by a shell brace (sub)expression.  When
// it has exhausted all of its values, `advance` returns `false`, and the
// `Expander` is restored to its initial state.
class Expander {
  public:
    virtual ~Expander();

    // Increment this object to its next value.  Return `false` if this object
    // has "rolled over" to its initial value, i.e. if the invoking code should
    // "carry over" the increment.  Return `true` if this object has not
    // "rolled over" to its initial value.
    virtual bool advance() = 0;

    // Insert into the specified `stream` the current value of this object.
    virtual void printCurrent(std::ostream& stream) const = 0;
};

// Insert into the specified `stream` the current value of the specified
// `expander`.  Return a reference providing modifiable access to `stream`.
std::ostream& operator<<(std::ostream& stream, const Expander& expander);

class String : public Expander {
    std::string value;

  public:
    // Create an object having the specified `value`.
    explicit String(const std::string& value);

    // Return `false`.  Since a string only ever has a single value, to advance
    // it is always to "roll over" to its initial value.
    bool advance() override;

    // Insert this object's value into the specified `stream`.
    void printCurrent(std::ostream& stream) const override;
};

class Sequence : public Expander {
    std::vector<std::unique_ptr<Expander>> children;

  public:
    // Add the specified `child` to the end of this object's list of children.
    void appendChild(std::unique_ptr<Expander> child);

    // Increment this object by incrementing lexicographically the sequence of
    // its children, least significant first, where the last child is the least
    // significant.  Return `false` if this lexicographical ordering has
    // "rolled over" to its initial value.  Return `true` otherwise.
    bool advance() override;

    // Insert into the specified `stream` the concatenation of the current
    // values of each of this object's children, in the order in which they
    // were added.
    void printCurrent(std::ostream& stream) const override;
};

class Alternation : public Expander {
    std::vector<std::unique_ptr<Expander>> children;
    int                                    currentIndex;

    // Return a reference providing modifiable access to the currently selected
    // child.  The behavior is undefined if this object has no children.
    Expander& currentChild() const;

  public:
    // Create an object having no children.
    Alternation();

    // Add the specified `child` to the end of this object's list of children.
    void appendChild(std::unique_ptr<Expander> child);

    // Incrementing this object by incrementing the currently selected child.
    // If doing so "rolls over," then change the selection to the following
    // child.  If there are no more children, then reset to the first child and
    // return `false`.  Otherwise, return `true`.
    bool advance() override;

    // Insert into the specified `stream` the current value of the currently
    // selected child.
    void printCurrent(std::ostream& stream) const override;
};

// Return an `Expander` assembled using the specified parse tree `root`.  The
// returned value will not be null.
std::unique_ptr<Expander> expander(const ParseTreeNode& root);

// Insert into the specified `stream` all of the values produced by the
// specified `expander`, where each inserted value is separated from the next
// by the specified `separator`.
void expand(std::ostream&      stream,
            Expander&          expander,
            const std::string& separator);

}  // namespace brex

#endif
