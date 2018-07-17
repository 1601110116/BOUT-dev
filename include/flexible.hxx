
/*!************************************************************************
 *
 * Wrapper for fields for different stagger locations
 *
 **************************************************************************
 * Copyright 2018
 *    B.D.Dudson, S.Farley, M.V.Umansky, X.Q.Xu, D. Schwörer
 *
 * Contact: Ben Dudson, bd512@york.ac.uk
 *
 * This file is part of BOUT++.
 *
 * BOUT++ is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * BOUT++ is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with BOUT++.  If not, see <http://www.gnu.org/licenses/>.
 *
 **************************************************************************/

/*!************************************************************************
 * This file is autogenerated - see  flexible.hxx.in.py
 **************************************************************************/

template <typename F> class Flexible;

#pragma once

#ifndef __FLEXIBLE_H__
#define __FLEXIBLE_H__

#include <bout/dataiterator.hxx>
#include <bout/deprecated.hxx>
#include <bout_types.hxx>
#include <boutexception.hxx>
#include <field2d.hxx>
#include <field3d.hxx>
#include <field_data.hxx>

const char *strLocation(CELL_LOC);
const Field2D interp_to(const Field2D &, CELL_LOC, REGION);
const Field3D interp_to(const Field3D &, CELL_LOC, REGION);

/// Template for having one field at different locations. If a Field
/// is not yet known for that location, it will be created and
/// cached. It is further possible to provide the staggered fields, if
/// it is e.g. crated from an analytical expression.
template <typename F> class Flexible : public FieldData {
  typedef unsigned int uint;

public:
  /// Constructor from a Field
  Flexible(F &main) { init(main); };
  /// Can also be constructed the same way as the Field F itself.
  /// Passes all the arguments to the constructor, and stores the
  /// created F.
  template <typename... Args> Flexible(Args... args) {
    F main(args...);
    init(main);
  }
  /// Destructor
  ~Flexible() {
    for (uint i = 0; i < num_fields; ++i) {
      if (!(fields[i] == nullptr)) {
        delete fields[i];
      }
    }
  }
  /// Get a reference to the Field at location F
  const F &get(CELL_LOC loc_) {
    // staggered->staggered interpolation is not correct due to corner guard
    // cells not being set properly, so do not allow here: either mainid is the
    // field at CELL_CENTRE, or the only other field we can ask for is at
    // CELL_CENTRE
    ASSERT1(mainid == 0 || loc_ == CELL_CENTRE);
    ASSERT1(fields[mainid] != nullptr);

    if (loc_ == CELL_DEFAULT) {
      return *fields[mainid];
    }
    uint loc = getId(loc_);
    if (!is_valid[loc]) {
      // fields[0] is the field at CELL_CENTRE
      if (fields[0] == nullptr) {
        fields[0] = new F(interp_to(*fields[mainid], CELL_CENTRE, RGN_NOBNDRY));
        is_valid[0] = true;
      } else if (!is_valid[0]) {
        *fields[0] = interp_to(*fields[mainid], CELL_CENTRE, RGN_NOBNDRY);
        is_valid[0] = true;
      }
      if (loc != mainid) {
        if (fields[loc] == nullptr) {
          fields[loc] = new F(interp_to(*fields[mainid], loc_, RGN_NOBNDRY));
        } else {
          *fields[loc] = interp_to(*fields[mainid], loc_, RGN_NOBNDRY);
        }
        is_valid[loc] = true;
      }
    }
    return *fields[loc];
  };
  /// Assignment from a Field f
  /// Use this to reset main field
  Flexible<F> &operator=(F &f) {
    reset(true);
    mainid = getId(f.getLocation());
    set(f);
    ASSERT1(fields[mainid] != nullptr);
    ASSERT1(is_valid[mainid]);
    return *this;
  }
  /// Assignment from a Field f
  /// Use this to reset main field
  Flexible<F> &operator=(F &&f) {
    reset(true);
    mainid = getId(f.getLocation());
    set(f);
    ASSERT1(fields[mainid] != nullptr);
    ASSERT1(is_valid[mainid]);
    return *this;
  }
  /// Assignment from a BoutReal
  /// This sets only the field at the mainlocation
  Flexible<F> &operator=(BoutReal d) {
    *fields[mainid] = d;
    is_valid[mainid] = true;
    reset(false);
    ASSERT1(fields[mainid] != nullptr);
    return *this;
  }
  /// Set a part of the Flexible Field.
  /// If the main field is set, then, all other fields are
  /// invalidated. If an other location is set, then, it is assumed
  /// that the this is in sync with the main field.
  void set(F &field) {
    uint loc = getId(field.getLocation());
    if (loc == mainid) {
      reset(true);
    }
    if (fields[loc] == nullptr) {
      fields[loc] = new F(field);
    } else {
      *fields[loc] = field;
    }
    is_valid[loc] = true;
  };
  // Fallback to F - return the main field
  // Note: make a copy of the main field, do not return a reference to the main
  //       field.
  //       If returning a reference, the destructor may be called on the main
  //       field when the returned field goes out of scope, deleting the main
  //       field's data. Since BOUT++ Fields copy on change anyway, this does
  //       not add a lot of memory copying.
  operator const F() {
    // return *fields[mainid];
    const F maincopy = *fields[mainid];
    return maincopy;
  };
  // DEPRECATED? - do not know which stagger location
  const BoutReal &operator()(int x, int y) { return fields[mainid]->operator()(x, y); };
  // DEPRECATED? - do not know which stagger location
  const BoutReal &operator[](const DataIterator &i) {
    return fields[mainid]->operator[](i);
  };
  // DEPRECATED? - do not know which stagger location
  virtual inline const BoutReal &operator[](const Indices &i) const override {
    return fields[mainid]->operator[](i);
  };
  // DEPRECATED? - do not know which stagger location
  virtual inline BoutReal &operator[](const Indices &i) override {
    return fields[mainid]->operator[](i);
  };
  /// Various functions needed to be FieldData compatible.
  /// We are just forwarding the the various functions to the main
  /// field.
  virtual void accept(FieldVisitor &v) override { fields[mainid]->accept(v); }
  /// Various functions needed to be FieldData compatible.
  /// We are just forwarding the the various functions to the main
  /// field.
  virtual bool isReal() const override { return fields[mainid]->isReal(); }
  /// Various functions needed to be FieldData compatible.
  /// We are just forwarding the the various functions to the main
  /// field.
  virtual bool is3D() const override { return fields[mainid]->is3D(); }
  /// Various functions needed to be FieldData compatible.
  /// We are just forwarding the the various functions to the main
  /// field.
  virtual int byteSize() const override { return fields[mainid]->byteSize(); }
  /// Various functions needed to be FieldData compatible.
  /// We are just forwarding the the various functions to the main
  /// field.
  virtual int BoutRealSize() const override { return fields[mainid]->BoutRealSize(); }
  /// Various functions needed to be FieldData compatible.
  /// We are just forwarding the the various functions to the main
  /// field.
  virtual void doneComms() override {
    fields[mainid]->doneComms();
    reset(false);
  }; // Notifies that communications done
  /// Various functions needed to be FieldData compatible.
  /// We are just forwarding the the various functions to the main
  /// field.
  virtual void applyBoundary(bool init = false) override {
    for (uint i = 0; i < num_fields; ++i) {
      if (fields[i]) {
        fields[i]->applyBoundary(init);
      }
    }
  }
  /// There is currently no support to evolve a Flexible<Field>
  /// Thus this function is currently not implemented (i.e. it throws)
  virtual void applyTDerivBoundary() override {
    throw BoutException("Flexible<F>: applyTDerivBoundary(): Not implemented");
  };
  /// Forward allocate call to the main field.
  void allocate() { fields[mainid]->allocate(); }
  Flexible<F> &operator*=(const Field3D &rhs) {
    if (mainid == getId(rhs.getLocation())) {
      fields[mainid]->operator*=(rhs);
    } else {
      throw BoutException("Trying to update a Flexible<F>, but themain location of "
                          "Flexible<F> is different to the location of the "
                          "rhs.\nFlexible<F> is at %s, but rhs is at %s",
                          strLocation(mainLocation()), strLocation(rhs.getLocation()));
    }
    reset(false);
    return *this;
  };
  Flexible<F> &operator*=(const Field2D &rhs) {
    if (mainid == getId(rhs.getLocation())) {
      fields[mainid]->operator*=(rhs);
    } else {
      throw BoutException("Trying to update a Flexible<F>, but themain location of "
                          "Flexible<F> is different to the location of the "
                          "rhs.\nFlexible<F> is at %s, but rhs is at %s",
                          strLocation(mainLocation()), strLocation(rhs.getLocation()));
    }
    reset(false);
    return *this;
  };
  // BoutReal does not has a location, so we can aplly the operation
  // to all fields. Should be faster than re-interpolating the
  // fields. Further this conserves any fields that have been set.
  Flexible<F> &operator*=(BoutReal rhs) {
    for (uint i = 0; i < num_fields; ++i) {
      if (is_valid[i]) {
        fields[i]->operator*=(rhs);
      }
    }
    return *this;
  };
  Flexible<F> &operator/=(const Field3D &rhs) {
    if (mainid == getId(rhs.getLocation())) {
      fields[mainid]->operator/=(rhs);
    } else {
      throw BoutException("Trying to update a Flexible<F>, but themain location of "
                          "Flexible<F> is different to the location of the "
                          "rhs.\nFlexible<F> is at %s, but rhs is at %s",
                          strLocation(mainLocation()), strLocation(rhs.getLocation()));
    }
    reset(false);
    return *this;
  };
  Flexible<F> &operator/=(const Field2D &rhs) {
    if (mainid == getId(rhs.getLocation())) {
      fields[mainid]->operator/=(rhs);
    } else {
      throw BoutException("Trying to update a Flexible<F>, but themain location of "
                          "Flexible<F> is different to the location of the "
                          "rhs.\nFlexible<F> is at %s, but rhs is at %s",
                          strLocation(mainLocation()), strLocation(rhs.getLocation()));
    }
    reset(false);
    return *this;
  };
  // BoutReal does not has a location, so we can aplly the operation
  // to all fields. Should be faster than re-interpolating the
  // fields. Further this conserves any fields that have been set.
  Flexible<F> &operator/=(BoutReal rhs) {
    for (uint i = 0; i < num_fields; ++i) {
      if (is_valid[i]) {
        fields[i]->operator/=(rhs);
      }
    }
    return *this;
  };
  Flexible<F> &operator+=(const Field3D &rhs) {
    if (mainid == getId(rhs.getLocation())) {
      fields[mainid]->operator+=(rhs);
    } else {
      throw BoutException("Trying to update a Flexible<F>, but themain location of "
                          "Flexible<F> is different to the location of the "
                          "rhs.\nFlexible<F> is at %s, but rhs is at %s",
                          strLocation(mainLocation()), strLocation(rhs.getLocation()));
    }
    reset(false);
    return *this;
  };
  Flexible<F> &operator+=(const Field2D &rhs) {
    if (mainid == getId(rhs.getLocation())) {
      fields[mainid]->operator+=(rhs);
    } else {
      throw BoutException("Trying to update a Flexible<F>, but themain location of "
                          "Flexible<F> is different to the location of the "
                          "rhs.\nFlexible<F> is at %s, but rhs is at %s",
                          strLocation(mainLocation()), strLocation(rhs.getLocation()));
    }
    reset(false);
    return *this;
  };
  // BoutReal does not has a location, so we can aplly the operation
  // to all fields. Should be faster than re-interpolating the
  // fields. Further this conserves any fields that have been set.
  Flexible<F> &operator+=(BoutReal rhs) {
    for (uint i = 0; i < num_fields; ++i) {
      if (is_valid[i]) {
        fields[i]->operator+=(rhs);
      }
    }
    return *this;
  };
  Flexible<F> &operator-=(const Field3D &rhs) {
    if (mainid == getId(rhs.getLocation())) {
      fields[mainid]->operator-=(rhs);
    } else {
      throw BoutException("Trying to update a Flexible<F>, but themain location of "
                          "Flexible<F> is different to the location of the "
                          "rhs.\nFlexible<F> is at %s, but rhs is at %s",
                          strLocation(mainLocation()), strLocation(rhs.getLocation()));
    }
    reset(false);
    return *this;
  };
  Flexible<F> &operator-=(const Field2D &rhs) {
    if (mainid == getId(rhs.getLocation())) {
      fields[mainid]->operator-=(rhs);
    } else {
      throw BoutException("Trying to update a Flexible<F>, but themain location of "
                          "Flexible<F> is different to the location of the "
                          "rhs.\nFlexible<F> is at %s, but rhs is at %s",
                          strLocation(mainLocation()), strLocation(rhs.getLocation()));
    }
    reset(false);
    return *this;
  };
  // BoutReal does not has a location, so we can aplly the operation
  // to all fields. Should be faster than re-interpolating the
  // fields. Further this conserves any fields that have been set.
  Flexible<F> &operator-=(BoutReal rhs) {
    for (uint i = 0; i < num_fields; ++i) {
      if (is_valid[i]) {
        fields[i]->operator-=(rhs);
      }
    }
    return *this;
  };

private:
  // Don't allow implicit copy constructor or copy assignment by declaring
  // these members here
  // This is needed since we manage resources, see the 'rule of three'
  // https://stackoverflow.com/questions/4172722/what-is-the-rule-of-three
  Flexible(const Flexible &flexible_field) = delete;
  Flexible &operator=(const Flexible flexible_field) = delete;

  // Helper function to get index of location.
  uint getId(CELL_LOC loc_) const {
    uint loc = static_cast<const uint>(loc_) - 1;
    if (loc > num_fields || loc_ == 0) {
      throw BoutException("Unexpected Fieldlocation!\n (Info: I got %d)", loc);
    }
    return loc;
  };
  // internal constructor - set all fields to nullptr etc
  void init(F &main) {
    mainid = getId(main.getLocation());
    for (uint i = 0; i < num_fields; ++i) {
      fields[i] = nullptr;
      is_valid[i] = false;
    }
    if (main.isAllocated()) {
      // input 'main' should have valid data
      fields[mainid] = new F(main);
      is_valid[mainid] = true;
    } else {
      // initialize an empty field
      fields[mainid] = new F();
      is_valid[mainid] = false;
    }
  };
  // delete the fields
  // include_main - should the main field be removed or kept?
  void reset(bool include_main) {
    for (uint i = 0; i < num_fields; i++) {
      if (i != mainid || include_main) {
#if CHECK > 2
        if (fields[i] != nullptr && fields[i]->isAllocated()) {
          fields[i]->allocate(); // make sure reference to data is unique
          for (auto j : *fields[i]) {
            (*fields[i])[j] = nan("");
          }
        }
#endif
        is_valid[i] = false;
      }
    }
  };
  // get the mainlocation
  CELL_LOC mainLocation() { return (CELL_LOC)(mainid + 1); };
  // Number of field locations we support
  static const uint num_fields = 4;
  // The pointers to the fields. Some may be null
  F *fields[num_fields];
  // Has the field been set?
  bool is_valid[num_fields];
  // The id of the mainlocation
  uint mainid;
};

template <typename F> F operator*(Flexible<F> &lhs, const F &rhs) {
  return lhs.get(rhs.getLocation()) * rhs;
};

template <typename F> F operator*(const F &lhs, Flexible<F> &rhs) {
  return lhs * rhs.get(lhs.getLocation());
};

template <typename F> F operator/(Flexible<F> &lhs, const F &rhs) {
  return lhs.get(rhs.getLocation()) / rhs;
};

template <typename F> F operator/(const F &lhs, Flexible<F> &rhs) {
  return lhs / rhs.get(lhs.getLocation());
};

template <typename F> F operator+(Flexible<F> &lhs, const F &rhs) {
  return lhs.get(rhs.getLocation()) + rhs;
};

template <typename F> F operator+(const F &lhs, Flexible<F> &rhs) {
  return lhs + rhs.get(lhs.getLocation());
};

template <typename F> F operator-(Flexible<F> &lhs, const F &rhs) {
  return lhs.get(rhs.getLocation()) - rhs;
};

template <typename F> F operator-(const F &lhs, Flexible<F> &rhs) {
  return lhs - rhs.get(lhs.getLocation());
};

inline Field3D operator*(Flexible<Field3D> &lhs, const Field2D &rhs) {
  return lhs.get(rhs.getLocation()) * rhs;
};

inline Field3D operator*(const Field3D &lhs, Flexible<Field2D> &rhs) {
  return lhs * rhs.get(lhs.getLocation());
};

inline Field3D operator/(Flexible<Field3D> &lhs, const Field2D &rhs) {
  return lhs.get(rhs.getLocation()) / rhs;
};

inline Field3D operator/(const Field3D &lhs, Flexible<Field2D> &rhs) {
  return lhs / rhs.get(lhs.getLocation());
};

inline Field3D operator+(Flexible<Field3D> &lhs, const Field2D &rhs) {
  return lhs.get(rhs.getLocation()) + rhs;
};

inline Field3D operator+(const Field3D &lhs, Flexible<Field2D> &rhs) {
  return lhs + rhs.get(lhs.getLocation());
};

inline Field3D operator-(Flexible<Field3D> &lhs, const Field2D &rhs) {
  return lhs.get(rhs.getLocation()) - rhs;
};

inline Field3D operator-(const Field3D &lhs, Flexible<Field2D> &rhs) {
  return lhs - rhs.get(lhs.getLocation());
};

inline Field3D operator*(Flexible<Field3D> &lhs, BoutReal rhs) {
  return lhs.get(CELL_DEFAULT) * rhs;
};

inline Field3D operator/(Flexible<Field3D> &lhs, BoutReal rhs) {
  return lhs.get(CELL_DEFAULT) / rhs;
};

inline Field3D operator+(Flexible<Field3D> &lhs, BoutReal rhs) {
  return lhs.get(CELL_DEFAULT) + rhs;
};

inline Field3D operator-(Flexible<Field3D> &lhs, BoutReal rhs) {
  return lhs.get(CELL_DEFAULT) - rhs;
};

inline Field3D operator*(Flexible<Field2D> &lhs, const Field3D &rhs) {
  return lhs.get(rhs.getLocation()) * rhs;
};

inline Field3D operator*(const Field2D &lhs, Flexible<Field3D> &rhs) {
  return lhs * rhs.get(lhs.getLocation());
};

inline Field3D operator/(Flexible<Field2D> &lhs, const Field3D &rhs) {
  return lhs.get(rhs.getLocation()) / rhs;
};

inline Field3D operator/(const Field2D &lhs, Flexible<Field3D> &rhs) {
  return lhs / rhs.get(lhs.getLocation());
};

inline Field3D operator+(Flexible<Field2D> &lhs, const Field3D &rhs) {
  return lhs.get(rhs.getLocation()) + rhs;
};

inline Field3D operator+(const Field2D &lhs, Flexible<Field3D> &rhs) {
  return lhs + rhs.get(lhs.getLocation());
};

inline Field3D operator-(Flexible<Field2D> &lhs, const Field3D &rhs) {
  return lhs.get(rhs.getLocation()) - rhs;
};

inline Field3D operator-(const Field2D &lhs, Flexible<Field3D> &rhs) {
  return lhs - rhs.get(lhs.getLocation());
};

inline Field2D operator*(Flexible<Field2D> &lhs, BoutReal rhs) {
  return lhs.get(CELL_DEFAULT) * rhs;
};

inline Field2D operator/(Flexible<Field2D> &lhs, BoutReal rhs) {
  return lhs.get(CELL_DEFAULT) / rhs;
};

inline Field2D operator+(Flexible<Field2D> &lhs, BoutReal rhs) {
  return lhs.get(CELL_DEFAULT) + rhs;
};

inline Field2D operator-(Flexible<Field2D> &lhs, BoutReal rhs) {
  return lhs.get(CELL_DEFAULT) - rhs;
};

inline Field3D operator*(BoutReal lhs, Flexible<Field3D> &rhs) {
  return lhs * rhs.get(CELL_DEFAULT);
};

inline Field3D operator/(BoutReal lhs, Flexible<Field3D> &rhs) {
  return lhs / rhs.get(CELL_DEFAULT);
};

inline Field3D operator+(BoutReal lhs, Flexible<Field3D> &rhs) {
  return lhs + rhs.get(CELL_DEFAULT);
};

inline Field3D operator-(BoutReal lhs, Flexible<Field3D> &rhs) {
  return lhs - rhs.get(CELL_DEFAULT);
};

inline Field2D operator*(BoutReal lhs, Flexible<Field2D> &rhs) {
  return lhs * rhs.get(CELL_DEFAULT);
};

inline Field2D operator/(BoutReal lhs, Flexible<Field2D> &rhs) {
  return lhs / rhs.get(CELL_DEFAULT);
};

inline Field2D operator+(BoutReal lhs, Flexible<Field2D> &rhs) {
  return lhs + rhs.get(CELL_DEFAULT);
};

inline Field2D operator-(BoutReal lhs, Flexible<Field2D> &rhs) {
  return lhs - rhs.get(CELL_DEFAULT);
};

#endif
