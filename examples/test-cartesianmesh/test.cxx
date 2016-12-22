#include <boutmain.hxx>
#include <derivs.hxx>
#include <field_factory.hxx>

#include <assert.h>

Field3D n;

void compare(Field3D a, Field3D b);

int physics_init(bool restart) {
  SOLVE_FOR(n); 
  compare(DDX(n),
	  FieldFactory::get()->create3D("diff_x:function", Options::getRoot(),
					mesh, CELL_CENTRE, 0));
  compare(DDX(n,CELL_XLOW),
	  FieldFactory::get()->create3D("diff_x:function", Options::getRoot(),
					mesh, CELL_XLOW, 0));
  compare(DDY(n),
	  FieldFactory::get()->create3D("diff_y:function", Options::getRoot(),
					mesh, CELL_CENTRE, 0));
  compare(DDY(n,CELL_YLOW),
	  FieldFactory::get()->create3D("diff_y:function", Options::getRoot(),
					mesh, CELL_YLOW, 0));
  if (Options::getRoot()->getSection("diff_z")->isSet("function")){
    compare(DDZ(n),
	    FieldFactory::get()->create3D("diff_z:function", Options::getRoot(),
					  mesh, CELL_CENTRE, 0));
    compare(DDZ(n,CELL_ZLOW),
	    FieldFactory::get()->create3D("diff_z:function", Options::getRoot(),
					  mesh, CELL_ZLOW, 0));
    }
  
  return 0;
}


int physics_run(BoutReal time) {
  ddt(n)=0;
  return 0;
}


void compare( Field3D diff, Field3D exp){
  static int counter=0;
  int print;
  OPTION(Options::getRoot(),print,-1);
  if (counter++==print){
    //for (int x=mesh->xstart;x<=mesh->xend;++x)
      for (int x=0;x<mesh->LocalNx;++x)
      for (int y=mesh->ystart;y<=mesh->yend;++y)
	//for (int y=0;y<mesh->LocalNy;++y)
	for (int z=0;z<mesh->LocalNz;++z)
	  output.write("\t%2d %2d %2d   %8g\t%8g\t%8g\n",x,y,z,exp(x,y,z)
		       ,diff(x,y,z),n(x,y,z));
  
  }
  auto error=max(abs(diff-exp)); 
  output.write("\n%g\n",error);
  if (error > 1e-8){
    int x=mesh->xend-1;
    int y=mesh->ystart;
    int z=mesh->LocalNz-1;
    auto err=exp-diff;
    
    for (int x=mesh->xstart;x<=mesh->xend;++x)
      //for (int x=0;x<mesh->LocalNx;++x)
      //for (int y=mesh->ystart;y<=mesh->yend;++y)
      for (int y=0;y<mesh->LocalNy;++y)
	for (int z=0;z<mesh->LocalNz;++z)
	  output.write("\t%2d %2d %2d   %8g\t%8g\t%8g\n",x,y,z,exp(x,y,z)
		       ,diff(x,y,z),n(x,y,z));
    
    throw BoutException("Error is to large: %g",error);
  }
}

