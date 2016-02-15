#pragma once

#include "util/pool.hh"

#include <boost/range/iterator_range_core.hpp>

#include <cstddef>
#include <cstring>

namespace util {

/**Layout: design flat data structures at runtime.
 * Layouts are created by attaching fields:
 *   PODField stores some type.
 *     Example: PODField<int> an_int(layout);
 *   ArrayField is an array whose length is constant across data structures
 *     Example: ArrayField<MyStruct> array(layout, 3); // array of 3 elements
 *   VectorField is a variable-length array with a vector-like interface.
 *     Example: VectorField<unsigned int> variable_length(layout);
 * 
 * Once the layout has been created, one can allocate the flat data structure:
 *   void *base = layout.Allocate(pool);
 * If there are variable-length fields, then VectorField reserves the right to
 * change the base upon vector resize.
 *
 * Then populate:
 * an_int(base) = 1;
 * array(base)[0] = MyStruct(1);
 * variable_length(base, pool).push_back(2); // Requires pool used to Allocate
 *
 * Complete example:
 * Layout layout;
 * PODField<int> an_int(layout);
 *
 * util::Pool pool;
 * void *base = layout.Allocate(pool);
 * an_int(base) = 0;
 *
 * The only external function call is Allocate; all others are used by Field
 * objects, though one could use them to write a custom Field.
 */
class Layout {
  public:
    Layout() : offsets_begin_(0), offsets_end_(0), offsets_count_(0) {
#ifdef DEBUG
      frozen_ = false;
#endif
    }

    /** Allocate the flat data structure from a pool.
     * Note that the returned void * is updated by VectorField on resize. */
    template <class Allocator> void *Allocate(Allocator &pool) {
#ifdef DEBUG
      frozen_ = true;
#endif
      void *ret = pool.Allocate(OffsetsEnd());
      // Initially everything in the variable length table is blank.
      std::memset(static_cast<uint8_t*>(ret) + OffsetsBegin(), 0, OffsetsEnd() - OffsetsBegin());
      return ret;
    }

    // Add a fixed-length field and return its beginning offset.
    // Intended to be called by PODField and ArrayField, not end-users.
    std::size_t ReserveFixed(std::size_t amount) {
#ifdef DEBUG
      assert(!frozen_);
#endif
      std::size_t ret = offsets_begin_;
      offsets_begin_ += amount;
      offsets_end_ += amount;
      return ret;
    }

    // Add a variable-length field, returning its index in the offsets table.
    // All calls to this function must use the same Size type.
    // Intended to be called by VectorField, not end-users.
    template <class Size> std::size_t ReserveVariable() {
#ifdef DEBUG
      assert(!frozen_);
#endif
      assert(offsets_end_ - offsets_begin_ == offsets_count_ * sizeof(Size));
      offsets_end_ += sizeof(Size);
      return offsets_count_++;
    }

    // Positions in the data structure where an array of offsets is stored for
    // variable-length fields.
    std::size_t OffsetsBegin() const { return offsets_begin_; }
    std::size_t OffsetsEnd() const { return offsets_end_; }

  private:
    std::size_t offsets_begin_, offsets_end_;
    std::size_t offsets_count_;

#ifdef DEBUG
    // Prevent adding fields after allocating some data structures.
    bool frozen_;
#endif
};

/**Store and access a POD in a Layout.
 * @param T the type to store.  T must be POD.
 */
template <class T> class PODField {
  public:
    /**Add this field to the layout */
    explicit PODField(Layout &layout) : offset_(layout.ReserveFixed(sizeof(T))) {}

    /**Retrieve the POD given the base pointer to the underlying memory */
    const T &operator()(const void *base_void) const {
      return *reinterpret_cast<const T*>(static_cast<const uint8_t*>(base_void) + offset_);
    }

    /**Edit the POD given the base pointer to the underlying memory */
    T &operator()(void *base_void) const {
      return *reinterpret_cast<T*>(static_cast<uint8_t*>(base_void) + offset_);
    }

  private:
    /** Offset at which the structure is stored */
    std::size_t offset_;
};

/**Store and access a fixed-length array as a field in a layout.
 * @param T the type to store in the array.  T must be POD.
 * By fixed, we mean what's stored in this field has the same length in every
 * memory record.  This could be used for score vectors in a phrase table.
 */
template <class T> class ArrayField {
  public:
    /* Attach to a layout, making space for the array storing @length elements */
    ArrayField(Layout &layout, std::size_t length)
      : begin_(layout.ReserveFixed(length * sizeof(T))), end_(begin_ + length * sizeof(T)) {}
    
    /* Get iterators to the array */
    boost::iterator_range<const T*> operator()(const void *base_void) const {
      const uint8_t *base = static_cast<const uint8_t*>(base_void);
      return boost::make_iterator_range(reinterpret_cast<const T*>(base + begin_), reinterpret_cast<const T*>(base + end_));
    }

    /* Get iterators to the array */
    boost::iterator_range<T*> operator()(void *base_void) const {
      uint8_t *base = static_cast<uint8_t*>(base_void);
      return boost::make_iterator_range(reinterpret_cast<T*>(base + begin_), reinterpret_cast<T*>(base + end_));
    }

    /* Number of records in the array (not the number of bytes */
    std::size_t size() const { return (end_ - begin_) / sizeof(T); }

  private:
    /* Offsets bounding the array in the layout */
    std::size_t begin_, end_;
};

/**Variable-size field with a vector-like API.
 * @param T is the type to store in the vector.  T must be POD.
 * @param Size is the type to use for sizing the vector.  The same Size must be
 *   used for all VectorField instances attached to the same Layout.
 *
 * Vectors must be sized in the same order they were attached to the Layout.
 * One must provide the Initialize object created when the instance was
 * allocated:
 *   FakeVector<Allocator> operator()(Initialize<Allocator>)
 * The FakeVector class can be used to 
 *
 * Elements can always be accessed by using the Initialize object or raw
 * pointer:
 *   operator()(void *)
 *   operator()(const void *)
 */
template <class T, class Size = std::size_t> class VectorField {
  public:
    /**Vector-like interface used when Initialize is provided.  This takes
     * care of calling the pool to support resizing.
     */
    template <class Allocator = util::Pool> class FakeVector {
      public:
        /** Append to the vector.  Updates the void * base pointer */
        void push_back(const T &t) {
          resize(size() + 1);
          back() = t;
        }

        /** Resize the vector.  Updates the void * base pointer */
        void resize(std::size_t to) {
          void *old_base = base_;
          std::ptrdiff_t change = (to - size()) * sizeof(T);
          if (allocator_.Continue(base_, change)) {
            ReBase(begin_, old_base);
            ReBase(end_offset_, old_base);
          }
          *end_offset_ += change;
          end_ = begin_ + to;
        }

        void clear() { resize(0); }

        bool empty() const { return begin_ == end_; }

        std::size_t size() const { return end_ - begin_; }

        const T* begin() const { return begin_; }
        T* begin() { return begin_; }

        const T* end() const { return end_; }
        T* end() { return end_; }

        const T &back() const { return *(end() - 1); }
        T &back() { return *(end() - 1); }
        
        const T &front() const { return *begin(); }
        T &front() { return *begin(); }

        const T &operator[](std::size_t index) const { return begin_[index]; }
        T &operator[](std::size_t index) { return begin_[index]; }

      private:
        friend VectorField;
        FakeVector(void *&base, boost::iterator_range<T*> it, Size *end_offset, Allocator &allocator)
          : base_(base), begin_(it.begin()), end_(it.end()), end_offset_(end_offset), allocator_(allocator) {}

        template <class P> void ReBase(P *&ptr, void *old_base) {
          ptr = reinterpret_cast<P*>(reinterpret_cast<uint8_t*>(ptr) - static_cast<uint8_t*>(old_base) + static_cast<uint8_t*>(base_));
        }

        void *&base_;

        T *begin_, *end_;
        Size *end_offset_;

        Allocator &allocator_;
    };

    /** Attach a variable-length field to a layout. */
    explicit VectorField(Layout &layout) : layout_(layout), index_(layout.ReserveVariable<Size>()) {}

    /**Access the vector (as a FakeVector) including the ability to resize.
     * The underlying pool must not have been used to allocate something else.
     * If multiple VectorField instances are attached to the same Layout, then
     *   sizing must be done in the same order as they were attached to Layout.
     * The base pointer may be changed as a result of hitting the end of the
     * allocation page.
     */
    template <class Allocator> FakeVector<Allocator> operator()(void *&base, Allocator &allocator) {
      return FakeVector<Allocator>(base, (*this)(base), &OffsetsBegin(base)[index_], allocator);
    }

    /**Read elements of the vector.*/
    boost::iterator_range<const T*> operator()(const void *base) const {
      return boost::make_iterator_range(
          reinterpret_cast<const T*>(VariableStart(base) + (index_ ? OffsetsBegin(base)[index_ - 1] : 0)),
          reinterpret_cast<const T*>(VariableStart(base) + OffsetsBegin(base)[index_]));
    }

    /**Access elements of the vector (but cannot resize).
     * Resizing must be done immediately after allocation by using
     * Initialize<Allocator>.
     */
    boost::iterator_range<T*> operator()(void *base) {
      return boost::make_iterator_range(
          reinterpret_cast<T*>(VariableStart(base) + (index_ ? OffsetsBegin(base)[index_ - 1] : 0)),
          reinterpret_cast<T*>(VariableStart(base) + OffsetsBegin(base)[index_]));
    }

  private:
    // Where the offsets array starts, given the structure base pointer.
    const Size *OffsetsBegin(const void *base_void) const {
      return reinterpret_cast<const Size*>(static_cast<const uint8_t*>(base_void) + layout_.OffsetsBegin());
    }
    Size *OffsetsBegin(void *base_void) const {
      return reinterpret_cast<Size*>(static_cast<uint8_t*>(base_void) + layout_.OffsetsBegin());
    }

    // Where the variable-length storage starts.  This is right after the
    // offsets array.
    const uint8_t *VariableStart(const void *base_void) const {
      return static_cast<const uint8_t*>(base_void) + layout_.OffsetsEnd();
    }
    uint8_t *VariableStart(void *base_void) const {
      return static_cast<uint8_t*>(base_void) + layout_.OffsetsEnd();
    }

    // layout to get the location of the offsets array (which can change after
    // this was constructed since more fixed or variable fields can be added).
    Layout &layout_;

    // The index of this field's end pointer in the offsets array.
    std::size_t index_;
};

} // namespace util