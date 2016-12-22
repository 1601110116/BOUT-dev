#pragma once

#include "../bout/boutmesh.hxx"

#include <stencils.hxx>

#include <list>
#include <vector>
#include <cmath>

using std::list;
using std::vector;


class AiolosMesh : public BoutMesh {
 public:
  AiolosMesh(GridDataSource *s, Options *options = NULL);
  ~AiolosMesh();
  
  typedef Field3D (*deriv_func)(const Field3D ); // f
  typedef Field3D (*upwind_func)(const Field3D , const Field3D); // v, f
  
  struct cart_diff_lookup_table {
    Mesh::deriv_func func;     // Single-argument differencing function
    deriv_func norm;
    deriv_func on;
    deriv_func off;
  };

  #include "generated_header.hxx"

  //virtual const Field3D interp_to(const Field3D &var, CELL_LOC loc) const;
  
  virtual const Field3D interp_to(const Field3D &f , CELL_LOC loc) const override {
    if (loc == f.getLocation() || loc == CELL_DEFAULT){
      return f;
    } else {
      return interp_to_do(f,loc);
    }
  }
  virtual const Field2D interp_to(const Field2D &f , CELL_LOC loc) const override {
    return f;
  }
  
  virtual BoutReal GlobalY(int y) const;
  virtual void derivs_init(Options * option);

  // to check in debugger we have the right mesh
#ifdef CHECK
  bool isAiolos=true;
#endif
private:
  const Field3D interp_to_do(const Field3D & f, CELL_LOC loc) const;
};
